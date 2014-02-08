#!/usr/bin/perl
# -*- perl -*-

#
#   This program is free software: you can redistribute it and/or
#   modify it under the terms of the GNU General Public License as
#   published by the Free Software Foundation, version 3 of the
#   License.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see
#   <http://www.gnu.org/licenses/>.
#
#

use strict;
use textus::OpenFile;

# Each config file desribes one or more modules
# and uses perl like curly brace/square brace deliminators.
# key value pairs can use either = or :.  We start off in a hash,
# so foo=bar is valid on line 1.

package textus::ConfigFile;

sub initialize {
    my $self = shift;
}

sub new {
    my $class = shift;
    my $self = {};
    bless $self, $class;
    $self->initialize();
    return $self;
}

sub addobject {
    my ($stack, $current, $key, $obj) = @_;
    my $hr = { 'Object' => $current, 'Key' => $key};
    push (@{$stack}, $hr);
    if (ref($current) eq "HASH") {
	$current->{${$key}} = $obj;
	$current=$current->{${$key}};
	${$key }= '';
	return $current;
    } elsif (ref($current) eq "ARRAY") {
	push (@{$current}, $obj);
	${$key}='';
	return $obj;
    } else {
	# don't know what to do here, or how we got here.  Must be
	# some sort of syntax error.
	die "Syntax Error?";
    }
}

sub addValue {
    my($current, $key, $value, $continue) = @_;
    $value =~ s/\s+$//;
    $value = unescape($value);
    $value = "\$value=\"$value\"";
    eval $value;
    
    if (ref($current) eq 'HASH') {
	unless ($continue) {
	    $current->{$key} = $value;
	} else {
	    $current->{$key} = $current->{$key} . $value;
	}
    } elsif (ref ($current) eq 'ARRAY') {
	die "syntax error?" if ($key);
	unless ($continue) {
	    push (@{$current}, $value);
	} else {
	    push (@{$current}, pop(@{$current}).$value);
	}
    }
}

sub unescape {
    my $str = shift;
    $str =~ s/(\\\\)*\K\\e(([^\\]|\\[^e])+)\\e/eval($2)/eg;
    $str =~ s/(\\\\)*\K\\((0b|0x)?[0-9]*)/chr(oct($2))/eg;
    $str =~ s/\\(.)/$1/g;
    return $str;
}

sub parseFile {
    my $self = {};
    my $current = $self;
    my $key = '';
    my @obj_stack;
    my $continue;

    $self->{__FILES__} = [@_];
    while (@_) {
	my $file_info = shift;
	$file_info->{handle} = new textus::OpenFile $file_info->{name} unless defined($file_info->{handle});
	$. = int ($file_info->{line});
	while (readline($file_info->{handle})) {
	    if (/(^|[^\\])(\\\\)*\#/p) {
		$_ = ${^PREMATCH}.${^MATCH};
		chop;
	    }
	    while (not /^\s*$/) {
		s/^\s+//;

		unless($continue) {
		    if (substr($_, 0, 1) eq '{') {
			substr($_, 0, 1) = '';
			$current = addobject(\@obj_stack, $current, \$key, {});
			next;
		    }

		    if (substr($_, 0, 1) eq '[') {
			substr($_, 0, 1) = '';
			$current = addobject(\@obj_stack, $current, \$key, []);
			next;
		    }

		    if (substr($_, 0, 1) eq '}') {
			substr($_, 0, 1) = '';
			if (ref($current) ne 'HASH') {
			    # we have a syntax error.
			    return "unexpected } in ".$file_info->{name}." at line $.";
			}
			my $old = pop (@obj_stack);
			#addValue($old->{Object}, $old->{Key}, $current, undef);
			$current = $old->{Object};
			return "Syntax error unexpected } in $file_info->{name} at line $." unless $current;
			$key='';
			next;
		    }

		    if (substr($_, 0, 1) eq ']') {
			substr($_, 0, 1) = '';
			if (ref($current) ne 'ARRAY') {
			    # we have a syntax error.
			    return "unexpected ] in $file_info->{name} at line $.";
			}
			my $old = pop (@obj_stack);
			#addValue($old->{Object}, $old->{Key}, $current, undef);
			$current = $old->{Object};
			$key='';
			next;
		    }

		    #
		    # special case, key's, or values in arrays starting with $ are considered to be directives such as $include.
		    #
		    if (substr($_, 0, 1) eq '$') {
			if (s/^\$(\w+)\s+(.*)$//) {
			    my $directive=$1;
			    my $arg=$2;
			    if ($directive eq 'include') {
				my @file_names = glob $arg;
				push @file_names, $arg unless @file_names;
				foreach $arg (@file_names) {
				    $file_info->{line} = $.;
				    unshift @_, $file_info;
				    undef $file_info;
				    $file_info = { 'name' => $arg, 'line' => 0 };
				    $. = 0;
				    push @{$self->{__FILES__}}, $file_info;
				    # $arg can specify an arbitrary shell command, but it's no worse than a makefile.
				    $file_info->{handle} = new textus::OpenFile $arg or die "Unable to open include file $arg";
				}
				undef $_;
				next;
			    } else {
				return "unknown directive $directive at line $.";
			    }
			}
		    }

		    # eat a single , by itself.
		    if (substr($_, 0, 1) eq ',') {
			substr($_, 0, 1) = '';
			next;
		    }

		    # Now we know just have the simple values, or name value pairs.
		    # If we are in a hash and key is '' we are looking for key: or key=,
		    # otherwise we are looking for a value. 
		    if (ref($current) eq 'HASH' and $key eq '') {
			if (s/^(([^:=]|(\\\\)*\\[:=])+)\s*[:=]//) {
			    $key = $1;
			    $key=~s/\s+$//;
			    $key = unescape($key);
			    next;
			} else {
			    return "syntax error expected key: or key= at line $.";
			}
		    }
		}

		# Here we are just looking for the first unescaped , or closing ] or }
		my $closing;
		if (ref($current) eq 'HASH') {
		    if ($current != $self) {
			$closing = '\\}';
		    }
		} else {
		    $closing = '\\]';
		}

		if (s/(([^,$closing]|(\\\\)*\\[,$closing])*)(\\\\)*(?<end>[,$closing])//) {
		    addValue($current, $key, $1, $continue);
		    undef $continue;
		    $key='';
		    if ($+{end} ne ',') {
			$_ = substr($closing, -1).$_;
		    }
		    next;
		} else {
		    addValue($current, $key, $_, $continue);
		    $continue=1;
		    $_='';
		    next;
		}
	    }
	}
    }
    return $self;
}

1;

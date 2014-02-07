#!/usr/bin/perl -I ./textus/scripts
# -*- perl -*-
# babysitter similiar to init, but made to run more things, 

use strict;
use POSIX qw( WNOHANG );
use Getopt::Long;

#use textus::Monitored;
#use textus::Arguments;
#use textus::MainLoop;
use textus::ConfigFile;
use textus::OpenFile;
package Child;

use vars qw ($config_file);
my $max_delay;
my $restart_threshold;
my %Children;
my $max_restart_rate;
my $time_to_kill;
my $time_to_die;
$config_file="/data/babysitter/config";
my $last_config_update;

#@ISA=qw( Monitored );

sub new {
    my $type = shift;
    my $self = {};
    bless $self, $type;

    my %args = @_;

    #default values.
    $args{restart_threshold} = $restart_threshold unless ($args{restart_threshold});
    $args{max_delay} = $max_delay unless ($args{max_delay});
    $args{max_restart_rate} = $max_restart_rate unless ($args{max_restart_rate});

    # things you are allowed to set 
    my @settable = qw( restart_threshold max_delay max_restart_rate name command args delay rate);

    @{$self}{@settable} = @args{@settable};

    die "Unnamed child" unless ($self->{name});

    $Children{$self->{name}} = $self;
}

sub getChild {
    my $class = shift;
    my $name = shift;
    return $Children{$name};
}

sub forget {
    my $self = shift;
    delete $Children{$self};
}

sub start {
    my $self = shift;
    if ($self->{nextVersion}) {
	$self = Child->new($self->{nextVersion});
	$self->{rate} = 0;
	$self->{delay} = 0;
	return $self->start();
    }
    if ($self->{delay}) {
	delete $self->{pid};
	delete $self->{pg};
	return $self->{delay}+$self->{last_restart};
    }

    my @command = ($self->{command}, @{$self->{args}});
    my $pid = fork;
    if ($pid == 0) {
	setpgrp;
	exec { $command[0] } @command;
	die "Unable to execute command.";
    }
    my $now = time();
	$self->{last_restart} = $now;
    if ($now - $self->{restart} < $self->{restart_threshold}) {
	$self->{rate}++;
	if ($self->{rate} > $self->{max_restart_rate}) {
	    $self->{delay} = $self->{delay} * 10;
	    $self->{delay} = $self->{max_delay} if ($self->{delay} > $self->{max_delay});
	}
    } else {
	$self->{rate} = 0;
    }
    $self->{pg} = -$pid;
    $self->{pid} = $pid;
    return undef;
}

sub dead {
    my $self = shift;
    kill "KILL", $self->{pg};
    delete $self->{pid};
    delete $self->{pg};
    return $self->start();
}

sub reap {
    my $ch;
    do {
	$ch = waitpid(-1, POSIX::WNOHANG);
	if ($ch > 0) {
	    my $child;
	    foreach $child (values %Children) {
		$child->dead() if ($child->{pid} == $ch);
	    }
	}
    } while ($ch > 0);
}

sub killMe {
    my $self = shift;
    kill 'TERM', $self->{pid};
    $self->{terminated} = time();
}

sub killMyKids {
    my $self = shift;
    kill 'TERM', $self->{pg};
}

sub reallyKillEveryone {
    my $self = shift;
    kill 'KILL', $self->{pg};
}

sub terminate {
    my $next_wake = time() + 60*60;
    my $child;
    foreach $child (values %Children) {
	if (!defined $child->{pid}) {
	    my $delay = $child->{start};
	    $next_wake = $delay if (defined($delay) and $delay < $next_wake);
	}
	if ($child->{terminated}) {
	    if (time() - $child->{terminated} >= $time_to_kill) {
		$child->reallyKillEveryone();
	    } elsif (time() - $child->{terminated} >= $time_to_die) {
		$next_wake = $child->{terminated} + $time_to_kill if ($child->{terminated} + $time_to_kill < $next_wake);
		$child->killMyKids();
	    }
	    $next_wake = $child->{terminated} + $time_to_die if ($child->{terminated} + $time_to_die < $next_wake);
	}
    }
    return $next_wake;
}

sub update {
    my $self = shift;
    my $new = shift;
    my $key;
    foreach $key (keys %$new) {
	if ($self->{$key} ne $new->{$key}) {
	    $self->{nextVersion} = $new;
	    $self->killMe() if ($new->{when} eq 'Now');
	}
    }
}

sub loadFile {
    my @sb = textus::OpenFile->stat ($config_file);
    die "Unable to open config file: ".$config_file." ".$@ unless @sb;
    my $mtime = $sb[9];
    return unless (!defined($last_config_update) or $mtime > $last_config_update);
    $last_config_update = $mtime;
    my @args;
    push @args, { 'name' => $config_file };
    my $config = textus::ConfigFile::parseFile( @args );
    my $children = $config->{children}; # needs to be an array ref.
    my $child;
    foreach $child (@$children) {
	# a child is a { child params };
	if (ref ($child) ne 'HASH') {
	    die "Syntax error in child: $child ";
	}
	my $name = $child->{name} or die "Unnamed child.";
	if ($Children{$name}) {
	    $Children{$name}->update($child);
	} else {
	    my $c = Child->new(%$child);
	    $c->start();
	}
    }
}

package main;

sub main {
    # my $args = Arguments->Process(@_);
    # my @sockets = Monitored->createSockets($args);
    my $result = GetOptions ("config_file=s" => \$Child::config_file);
    my $sleep_time;
    # push Child->createSockets(), @sockets;
    do {
	Child->loadFile();
	$sleep_time = Child->terminate();
	eval {
	    local $SIG{CHLD} = sub {die "sigchld\n"};
	    sleep $sleep_time;
	};
	die "Unexpected signal.\n" unless ($@ eq "sigchld\n"); 
	Child->reap();
    } while (1);
}


main(@ARGV) unless caller();

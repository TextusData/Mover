#!/usr/bin/perl
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


use Getopt::Long;
use Cwd 'abs_path';
use File::Basename qw(dirname);

package main;

sub newcert {
    my $path = shift;
    my $OPENSSL = shift;
    my $cert = shift;

    system ("$OPENSSL req -newkey rsa -nodes -keyout newkey.pem -out newreq.pem -days 366 <<FOO
US
VA
Ashburn
Textus
$path
localhost
test\@localhost


FOO");
    system ("$OPENSSL ca -batch -policy policy_anything -passin pass:test -out newcert.pem -infiles newreq.pem");
    system ("mv newcert.pem $path/certificate.pem");
    system ("mv newkey.pem $path/key.pem");
    system ("rm -f newreq.pem");
    system ("$OPENSSL dhparam -outform PEM -out $path/dh_param.pem -2 -rand /dev/urandom");

}

sub main {
    my $filename = shift;
    my @args=@_;
    my $client = undef;
    my $ca = 1;
    my $server = 1;
    my $directory='.';
    my $path="etc/ssl";
    my $CApath = "$path/CA";
    my %command_line = ('client!' => \$client,
			'directory=s' => \$directory,
			'server!' => \$server,
			'ca|certificate-authority!' => \$ca,
			'capath=s' => \$CApath,
	);

    my $CA = "$filename/../../obj/thirdparty/openssl/apps/CA.pl";
    my $OPENSSL = "$filename/../../bin/thirdparty/openssl/bin/openssl";
    my $C_REHASH = "$filename/../../bin/thirdparty/openssl/bin/c_rehash";

    Getopt::Long::GetOptionsFromArray(\@args, %command_line ) or die "Invalid Options";
    
    chdir($directory);
    system ("rm -rf $path/certificate.pem $path/dh_param-pem $path/key.pm") if ($server);
    system ("rm -rf client/$path/CA client/$path/certificate.pem ".
	    "client/$path/dh_param-pem client/$path/key.pm") if ($client);
    system ("rm -f demoCA");
    system ("ln -s $CApath demoCA");

    if ($ca) {
	system ("rm -rf $CApath");
	system ("mkdir -p  $CApath");
	mkdir "$CApath/certs";
	mkdir "$CApath/crl";
	mkdir "$CApath/newcerts";
	mkdir "$CApath/private";
	open O, ">$CApath/index.txt";
	close O;
	open O, ">$CApath/crinumber";
	print O "01\n";
	close O;
	system ("$OPENSSL req -new -passout pass:test -keyout $CApath/private/cakey.pem -passout pass:test -out $CApath/careq.pem <<FOO
US
VA
Ashburn
Textus
$path
localhost
test\@localhost


FOO");
	system("$OPENSSL ca -create_serial -out $CApath/cacert.pem -days 366 -batch -key test ".
		"-keyfile $CApath/private/cakey.pem -selfsign -extensions v3_ca -infiles $CApath/careq.pem");
	system("$C_REHASH $CApath");
    }

    if ($server) {
	newcert($path, $OPENSSL, "$CApath/private/cakey.pem");
    }

    if ($client) {
	system ("mkdir -p client/$path");
	system ("cp -r $CApath client/$path");
	$path = "client/$path";
	newcert($path, $OPENSSL, "$CApath/private/cakey.pem");
    }

    system ("rm  demoCA");
}

main(dirname(abs_path($0)), @ARGV) unless (caller);

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
use textus::Test;
use babysitter;


package babysitterTest;

our @ISA=qw( textus::Test );

sub new {
  my $class = shift;
  my $self = textus::Test->new($0);
  bless $self, $class;
  $self->initialize();
  return $self;
}

sub initialize {
  my $self = shift;
}

sub runTests {
  my $self = shift;
  $Child::config_file = 'config1';
  Child->loadFile();
  my $child = Child->getChild('test1');
  $self->assert($child, "$child");
}

package main;

my $test = babysitterTest->new();
$test->runTests();

#!/usr/bin/perl
# -*- perl -*-
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

#!/usr/bin/perl
# -*- perl -*-

use strict;
use IO::File;
use File::stat qw(stat);

package textus::OpenFile;

my $root='';
my $application_dir='./';

sub setApplicationDir {
  my $class = shift;
  $application_dir = shift;
  $application_dir = $application_dir .'/' unless ($application_dir =~ m:/$:);
  return $application_dir;
}

sub setRootDir {
  my $class = shift;
  $root = shift;
  $root = $root .'/' unless ($root =~ m:/$:);
  return $root;
}

sub processName {
  my $name = shift;
  if ($name !~ m:^/:) {
    $name=$application_dir.$name;
  }

  $name = $root.$name;
  return $name;
}

sub stat {
  my $class = shift;
  my $name = shift;
  $name = processName($name);
  return stat($name);
}

sub open {
  my $class = shift;
  my $parm = shift;
  my $info;
  if (ref $parm eq '') {
    $info = { 'name' => $parm };
  } else {
    $info = $parm;
  }

  $info->{access} = 'r' unless ($info->{access});

  # we only open absoulte paths.
  $info->{name} = processName($info->{name});

  my $self = IO::File->new($info->{name}, $info->{access});
  return $self;
}

sub new {
  return textus::OpenFile::open(@_);
};

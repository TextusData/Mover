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

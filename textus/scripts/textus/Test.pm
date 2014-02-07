#!/usr/bin/perl
#-*- perl -*-

use textus::OpenFile;

package textus::Test;

sub new {
    my $class = shift;
    my $name = shift;
    my $self = {};
    bless $self, $class;
    $name =~ s:/textus/:/test_data/textus/:;
    $self->{name} = $name;
    textus::OpenFile->setRootDir($name);
    return $self;
}

sub assert {
    my $self = shift;
    my $foo = $_[0];
    die join (":", ($!, @_)) unless "$foo";
}

1;

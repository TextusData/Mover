#!/usr/bin/perl
# -*- perl -*-
# MainLoop

package LoopFH;

sub new {
    my $type = shift;
    my $self = {}
    bless $self, $type;
    $self->{watchRead} = 1;
    undef $self->{watchWrite};
    $self->{watchError} = 1;
}


sub watchRead {
    my $self = shift;
    return $self->{watchRead};
}

sub watchWrite {
    my $self = shift;
    return $self->{watchWrite};
}

sub watchError {
    my $self = shift;
    return $self->{watchError};
}

sub hasError {
    my $self = shift;
    $self->close();
}

sub processData {
    my $self = shift;
    my $data = shift;
    # do nothing.
}

sub canRead {
    my $self = shift;
    my $buff;
    my $read = sysread( $self->{handle}, $buff, 4096);
    if ($read < 0 and $! != EAGAIN) {
	$self->hasError();
	return $read;
    }
    return $self->processData($buff);
}

sub canWrite {
    my $self = shift;
    if (length($self->{queue}) == 0) {
	delete $self->{watchWrite};
	return;
    }
    my $wrote = syswrite ($self->{handle}, $self->{queue}, length($self->{queue}));
    if (length($self->{queue}) == $wrote) {
	delete $self->{queue};
	delete $self->{watchWrite};
    } else {
	$self->{queue} = substr($self->{queue}, $wrote);
    }
}

sub write {
    my $self = shift;
    $self->{queue} = $self->{queue}.join("", @_);
    $self->{watchWrite} = 1;
}

package MainLoop;

sub loop {
    my $rout;
    my $rin;
    my $wout;
    my $win;
    my $eout;
    my $ein;
    my $timeout;

    my %args = @_;
    my $rin;
    my $win;
    my $ein;
    my $rout;
    my $wout;
    my $eout;

    for (@{$args{handles}}) {
	my $fh = $_;
	vec($rin, $fh->{handle}, 1) = 1 if ($fh->watchRead());
	vec($win, $fh->{handle}, 1) = 1 if ($fh->watchWrite());
	vec($ein, $fh->{handle}, 1) = 1 if ($fh->watchError());
    }

    my $timeout = $args{timeout};

    my $nfound;
    my $timeleft;

    eval {
	local $SIG{CHLD} = sub {die "sigchld\n"}
	($nfound,$timeleft) = select($rout=$rin, $wout=$win, $eout=$ein, $timeout);
    };

    if ($@) {
	die unless $@ eq "sigchld\n";
	return 1;
    }

    for (@{$args{handles}}) {
	my $fh = $_;
	$fh->hasError() if (vec($eout, $fh->{handle}, 1) == 1);
	$fh->canWrite() if (vec($wout, $fh->{handle}, 1) == 1);
	$fh->canRead() if (vec($rout, $fh->{handle}, 1) == 1);
    }
    
    return 0;
}

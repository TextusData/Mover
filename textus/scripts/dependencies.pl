#!/usr/bin/perl -I ./textus/scripts
# -*- perl -*- 
# Copyright (C) 2009-2010, 2013 Ross Biro 
#
# Read module description files (.mod) and produce appropriate make
# files.  Each module description file desribes one or more modules
# and uses perl like curly brace/square brace deliminators.
# key value pairs can use either = or :.  We start off in a hash,
# so foo=bar is valid on line 1.

use Getopt::Long;
use IO::File;
use File::Basename;
use File::Spec;
use English;
use Cwd;
use textus::ConfigFile;

use strict;

package Module;

our (@sourcetypes);
our (%externalModuleCache);

@sourcetypes = qw(cc_sources proto_sources c_sources);


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

sub fullname {
    my $self = shift;
    return $self->{fullname};
}

# figure out the target of the current Module.
sub target {
    my $self = shift;

    $self->{base} = $self->{__FILES__}->[0]->{name} unless $self->{base};
    my $base=$self->{base};

    my ($name,$path,$suffix) = File::Basename::fileparse($base,  qr/\.[^.]*/);

    $path =~ s:/$::;
    
    $self->{base_path} = $path unless ($self->{base_path});
    $self->{name} = $self->{target} if ($self->{__PARENT__});
    $self->{name} = $name unless ($self->{name});

    $self->{name} =~ s:/:_:g;

    unless ($self->{fullname}) {
	$self->{fullname} = $path."_".$name;
	if ($self->{__PARENT__}) {
	    my ($name,$path,$suffix) = File::Basename::fileparse($self->{name},  qr/\.[^.]*/);
	    $self->{fullname} = $self->{fullname}."_".$name;
	}
	$self->{fullname} =~ s:/:_:g;
    }

    $self->{binprefix} = '$(BINROOT)' unless ($self->{binprefix});
    $self->{objprefix} = '$(OBJROOT)' unless ($self->{objprefix});
    $self->{makeprefix} = '$(MAKEROOT)' unless ($self->{makeprefix});

    $self->{binpath} = $self->{binprefix}.'/'.$self->{base_path} unless($self->{binpath});
    $self->{objpath} = $self->{objprefix}.'/'.$self->{base_path} unless($self->{objpath});
    $self->{makepath} = "$self->{makeprefix}/$self->{base_path}/$self->{name}.inc" unless ($self->{makepath});
    $self->{root_makepath} = "$self->{makeprefix}/$self->{base_path}/$self->{name}.make" unless ($self->{root_makepath});
    $self->{dependency_target} = "$self->{makeprefix}/$self->{base_path}/$self->{name}.deps" unless ($self->{dependency_target});

    my $target = $self->{target};
    $target = $self->{name} unless $target;
    unless ($target =~ m:/:) {
	($name, $path, $suffix) = File::Basename::fileparse($target, qr/\.[^.]*/);
	if ($suffix eq '.o' or $suffix eq '.a') {
	    $target=$self->{objpath}.'/'.$target;
	} else {
	    $target=$self->{binpath}.'/'.$target;
	}
    }

    $self->{target} = $target;

    unless ($self->{__DONE_LINKS__}) {
    my $link_hash;
    my $root = $self->{objpath};
    my $base = $self->{base_path};
    $base = Cwd::abs_path($base);
    
    $root =~ s/\$\(OBJROOT\)/obj/g;
    for $link_hash (@{$self->{links}}) {
	my $from;
	for $from (keys %$link_hash) {
	    my $to = $link_hash->{$from};
	    print "ln -snf $base/$to $root/$from";
	    system ("mkdir -p \"$root\"");
	    system ("ln -snf $base/$to $root/$from");
	}
    }
    $self->{__DONE_LINKS__} = 1;
    }

    return $target;
    
}

sub makefile {
    my $self = shift;
    $self->target() unless ($self->{makepath});
    return $self->{makepath};
}

sub rootMakefile {
    my $self = shift;
    $self->target() unless ($self->{root_makepath});
    return $self->{root_makepath};
}

sub isSrcFile {
    my $self = shift;
    my $file = shift;
    $file = $self->find_source_file($file);
    foreach my $sourcetype (@sourcetypes) {
	next unless (defined ($self->{$sourcetype}));
	my $src;
	for $src (@{$self->{$sourcetype}}) {
	    if ($file eq $self->find_source_file($src)) {
		$file;
	    }
	}
    }
    return undef;
}

sub dependencyTarget {
    my $self = shift;
    my $t = shift;
    $self->target() unless ($self->{dependency_target});
    return $self->{dependency_target};
}

sub find_mod_file {
    my $self = shift;
    my $modfile = shift;

    return $modfile if (-f $modfile);

    if (-d $modfile) {
	my @dirs = File::Spec->splitdir($modfile);
	my $dirname;

	$dirname = pop @dirs while (! $dirname);

	my $dirname = ucfirst($dirname);
	$modfile = File::Spec->catfile($modfile, $dirname);
    }

    return "$modfile.mod" if (-e "$modfile.mod");

    return "$modfile.mod2" if (-e "$modfile.mod2");

    my $cand;

    $cand = $self->{base_path}.'/'.$modfile;

    return $cand if (-e $cand);

    $cand = $cand.".mod";

    return $cand if (-e $cand);

    $cand = $cand . "2";

    return $cand if (-e $cand);

    $cand = 'obj/'.$modfile;

    return $cand if (-e $cand);

    $cand = $cand.".mod";

    return $cand if (-e $cand);

    $cand = $cand . "2";

    return $cand if (-e $cand);

    $cand = $self->{obj_path}.'/'.$modfile;

    return $cand if (-e $cand);

    $cand = $cand.".mod";

    return $cand if (-e $cand);

    $cand = $cand . "2";

    return $cand if (-e $cand);

    $cand = "\$cond=\"$modfile\"";
    eval $cand;

    return $cand if (-e $cand);

    $cand = $cand.".mod";

    return $cand if (-e $cand);

    $cand = $cand . "2";

    return $cand if (-e $cand);

    return $modfile;
}

sub find_source_file {
    my $self = shift;
    my $srcfile = shift;

    return $srcfile if (-e $srcfile);

    my $cand;
    $cand = $self->{base_path}.'/'.$srcfile;

    return $cand if (-e $cand);

    $cand = $self->{objprefix}.'/'.$srcfile;

    return $cand if (-e $cand);

    return $srcfile;

}

sub compute_obj_file {
    my $self = shift;
    my $srcfile = shift;

    my ($name,$path,$suffix) = File::Basename::fileparse($srcfile,  qr/\.[^.]*/);

    if ($suffix eq '.cc') {
	return [ $self->{objprefix}."/$path$name.o", $self->{objprefix}."/$path$name.d" ];
    } 

    if ($suffix eq '.c') {
	return [ $self->{objprefix}."/$path$name.o", $self->{objprefix}."/$path$name.d" ];
    } 

    if ($suffix eq '.proto') {
	return { deps => [ $self->{objprefix}."/$path$name.pb.o", $self->{objprefix}."/$path$name.pb.d", [
			       $self->{objprefix}."/$path$name.pb.h", $self->{objprefix}."/$path$name.pb.cc"]],
		     extra_source => '$(CCPROTOCOMPILER)',
	};
    }

    return [$srcfile];
}

sub addToCollection {
    my $self = shift;
    my $obj = shift;
    my $collection = shift;

    my ($name,$path,$suffix) = File::Basename::fileparse($obj,  qr/\.[^.]*/);

    $collection->{$suffix}=[] unless (defined ($collection->{$suffix}));
    
    push @{$collection->{$suffix}}, $obj;
}

sub getCachedModule {
    my $self = shift;
    my $dep = shift;

    my $modname = $self->getModName($dep);

    if ($externalModuleCache{$modname}) {
	return $externalModuleCache{$modname};
    }

    my $dependency = $self->find_mod_file($modname);
    my $mod = textus::ConfigFile::parseFile({ 'name' => $dependency});
    bless $mod, 'Module';

    $externalModuleCache{$modname}  = $mod;
    return $mod;
}

sub getExternalMakefile {
    my $self = shift;
    my $dep = shift;
    my $mod = $self->getCachedModule($dep);
    return $mod->makefile();
}

sub getModName {
    my $self = shift;
    my $dep = shift;

    my $file = $self->find_source_file($dep);

    return $dep unless (-e $file);
    
    my ($volume, $path, $filename ) = File::Spec->splitpath($file);
    
    my $mod = $self->find_mod_file($path);
    
    return $mod if (-f $mod);

    return $dep;
    
}

sub getExternalDependencyTarget {
    my $self = shift;
    my $dep = shift;

    my $modname = $self->getModName($dep);
    my $mod = $self->getCachedModule($modname);
    return $mod->dependencyTarget($dep);
}

sub getExternalTarget {
    my $self = shift;
    my $dep = shift;

    my $f = $self->find_source_file($dep);
    my $modname = $self->getModName($dep);
    my $mod = $self->getCachedModule($modname);

    if (-f $f and $mod->isSrcFile($f)) {
	return $f;
    }

    return $mod->target();
}

sub buildRule {
    my ($self, $input) = @_;
    my $target = $self->target();
    my $rule = "$target: ";

    my ($name,$path,$suffix) = File::Basename::fileparse($target,  qr/\.[^.]*/);
    if ($suffix eq '.o') {
	$rule = $rule."\n\techo new files: \$?";
	$rule = $rule."\n\t"."mkdir -p \$(dir \$@)$(LD) \$(LDFLAGS) \$(LD_RFLAG) -o $target \$(filter %.o, \$^) ";
    } elsif ($suffix eq '.a') {
	$rule = $rule."\n\techo new files: \$?";
	$rule = $rule."\n\t"."mkdir -p \$(dir \$@)\n\t if [ \"\$(filter %.o, \$^)\" != \"\" ]; then \$(AR) \$(ARFLAGS) $target \$(filter %.o, \$^); fi ";
    } else {
	my $fullname = $self->fullname();
	$rule =$rule."\$(DEPLIBS_$fullname)";
	$rule = $rule."\n\techo new files: \$?";
	$rule = $rule."\n\t"."mkdir -p \$(dir \$@)\n\t\$(CCC) \$(CFLAGS) \$(CCLDFLAGS) -o $target \$(filter %.o, \$^) \$(BEGIN_LIBS) \$(filter %.a, \$^) \$(DEPLIBS_$fullname) \$(END_LIBS) \$(LIBS)";
    }

    $rule=$rule."\n\n";
    return $rule;
}

sub topLevel {
    my $self = shift;
    while ($self->{__PARENT__}) {
	$self = $self->{__PARENT__};
    }
    return $self;
}

sub ancestor {
    my $self = shift;
    my $mod = shift;
    while ($self) {
	if ($self == $mod or $self->target() eq $mod->target() or $self->{__FILES__} eq $mod->{__FILES__}) {
	    return 1;
	}
	$self = $self->{__PARENT__};
    }
    return undef;
}

sub depOrder {
    my $self = shift;
    my $a = shift;
    my $b = shift;

    my $moda = $self->getCachedModule($a);
    my $modb = $self->getCachedModule($b);

    my @parents = @{$moda->{deps}} if ($moda->{deps});
    my %seen;
    while (@parents) {
	my $modname = shift @parents;
	next if ($seen{$modname});
	$seen{$modname} = 1;
	my $modc = $self->getCachedModule($modname);
	if ($modc == $modb) {
	    # b has to come before a
	    return -1;
	}
	push @parents, @{$modc->{deps}} if ($modc->{deps});
    }

    @parents = @{$modb->{deps}} if ($modb->{deps});
    undef %seen;
    while (@parents) {
	my $modname = shift @parents;
	next if ($seen{$modname});
	$seen{$modname}=1;
	my $modc = $self->getCachedModule($modname);
	if ($modc == $moda) {
	    # a has to come before b
	    return 1;
	}
	push @parents, @{$modc->{deps}} if ($modc->{deps});;
    }

    return $a <=> $b;
    
}

sub dependencies {
    my $self = shift;
    my $out = shift;
    my $collection = shift;
    my $target = $self->target();

    print $out $self->dependencyTarget()." : \n\n";

    if ($self->{deps}) {

	print $out "#\n";
	print $out "# External Dependencies.\n";
	print $out "#\n";

	print $out "\n";

	## Now we have to read through all the dependencies and make sure that their makefiles get generated.
	## dependencies are other mod files which we need to read.
	my $dependency;
	my @dep_array;
	my %dep_hash;
	@dep_array = @{$self->{deps}};
	my $parent = $self->{__PARENT__};
	while ($parent) {
	    unshift @dep_array, @{$parent->{deps}} if ($parent->{deps});
	    $parent=$parent->{__PARENT__};
	}

	my $fullname = $self->fullname();

	print $out "DEPLIBS_$fullname=";

	while (@dep_array) {
	    $dependency = shift @dep_array;
	    #$dependency = eval "print '$dependency';";
	    next if ($dep_hash{$dependency});
	    my $mod = $self->getCachedModule($dependency);
	    $dep_hash{$dependency}=1 unless ($self->ancestor($mod));
#	    if ($mod and $mod->{deps}) {
#		print ("adding dependencies for $dependency: ");
#		print join(", ", @{$mod->{deps}});
#		print "\n";
#	    }
	    push @dep_array, @{$mod->{deps}} if ($mod and $mod->{deps});
	}
	
	for $dependency (sort { $self->depOrder($a, $b) } keys %dep_hash) {
	    my $external_target = $self->getExternalTarget($dependency);
	    print $out "$external_target ";
	}

	print $out "\n\n";

	# print $out $self->dependencyTarget()." : ";
	# for $dependency (sort { $self->depOrder($a, $b) } keys %dep_hash) {
	#     my $dependency_target = $self->getExternalDependencyTarget($dependency);
	#     print $out "$dependency_target ";
	# }
	# 
	# print $out "\n\n";

	for $dependency (keys %dep_hash) {
	    next if ($self->topLevel()->{__EXTERNAL_DEPENDENCIES__}->{$dependency});
	    $self->topLevel()->{__EXTERNAL_DEPENDENCIES__}->{$dependency} = 1;
	    my $external_target = $self->getExternalTarget($dependency);
	    my $external_makefile = $self->getExternalMakefile($dependency);
	    $dependency = $self->find_mod_file($self->getModName($dependency));
	    print $out "$external_makefile: $dependency $0\n\n";
	    print $out "$external_target dependencies: $external_makefile\n";
	    print $out "-include $external_makefile\n";
	}

	print $out "\n";
	print $out "#\n";
	print $out "# End of external dependencies.\n";
	print $out "#\n";
	print $out "\n";
    }
    
    print $out "#\n";
    print $out "# Internal Dependencies.\n";
    print $out "#\n";

    print $out "$target : ";
    print $out $self->{__PARENT__}->target() if ($self->{__PARENT__});
    my $sourcetype;
    foreach $sourcetype (@sourcetypes) {
	next unless (defined ($self->{$sourcetype}));
	my $src;
	for $src (@{$self->{$sourcetype}}) {
	    $src = $self->find_source_file($src);
	    my $obj;
	    my $objects = $self->compute_obj_file($src);
	    if (ref($objects) eq 'HASH') {
		$objects = $objects->{deps};
	    }
	    foreach $obj (@$objects) {
		# at this point, we skip the intermediate files because we only care about the end result.
		print $out "\\\n\t$obj " if (not ref($obj));
	    }
	}
    }
    print $out "\n";
    print $out "\n";

    foreach $sourcetype (@sourcetypes) {
	next unless (defined ($self->{$sourcetype}));
	my $src;
	foreach $src (@{$self->{$sourcetype}}) {
	    my $colon;
	    $src = $self->find_source_file($src);
	    my $objs = $self->compute_obj_file($src);
	    if (ref($objs) eq 'HASH') {
		$src=$src." ".$objs->{extra_source};
		$objs = $objs->{deps};
	    }
	    next unless (@$objs);
	    my @stack = ($objs);
	    while (@stack) {
		$objs = shift @stack;
		while (@$objs) {
		    my $obj;
		    $obj = shift @$objs;
		    if (not ref($obj)) {
			$self->addToCollection($obj, $collection);
			print $out "$obj ";
		    } else {
			print $out ": " unless ($colon);
			my $intermediate_src;
			for $intermediate_src (@$obj) {
			    unless (ref ($intermediate_src)) {
				$self->addToCollection($intermediate_src, $collection);
				print $out "$intermediate_src ";
			    }
			}
			print $out "\n\n";
			$colon = 0;
			unshift @stack, $objs;
			$objs = $obj;
		    }
		}
		unless ($colon) {
		    print $out ": ";
		    # only need source for the inner most ones.
		    print $out "$src\n";
		    print $out "\n";
		    $colon = 1;
		}
	    }
	}

    }
    # Finally print out the rule for adding everything to the target.
    my $buildRule = $self->buildRule($collection);
    print $out $buildRule;
    print $out "\n\n";
}

sub expandedMakefile {
    my $self = shift;
    my $makefile = $self->makefile();
    $makefile =~ s/\$\(MAKEROOT\)/make/g;
    return $makefile;
}

sub expandedRootMakefile {
    my $self = shift;
    my $makefile = $self->rootMakefile();
    $makefile =~ s/\$\(MAKEROOT\)/make/g;
    return $makefile;
}

# 
# the syntax was describe above.
# The semantics are each file describes one or more modules
# a single module will be at the top level.  Multiple modules will
# be in an array, i.e. the file will look like [ {...}, {...}, ... ]
# Each module contains enough information to create the makefile 
# to bulid that module.
# The known parameters are
#
# name = name of module defaults to name of dependency file - the .mod suffix.
#
# deps = [ list of modules or files this one depends on ]
#
# env={} make environment variables such as LDFLAGS etc.
#
# targets=path to target with out bin prefix. no /'s is interpreted as relative to current directory, otherwise relative to src root.
#         defaults to name.
#
# cc_sources=[ list of cc source files ]
#
# c_sources=[list of c source files]
#
# proto_sources=[ list of proto source files ]
#
# unittests = [ { recursively the same as above. } + runtime= for how
# long they should be allowed to run. ]
#
# others = [ { recursively the same as above. } ]
#
# links =[{ map of links } ],  currently deprecated
sub writeMakefile {
    my $self = shift;

    my $target = $self->target();
    my $filename = $self->expandedMakefile();
    my $root_filename = $self->expandedRootMakefile();

    my %collection;

    my ($name, $path, $suffix) = File::Basename::fileparse($filename);
    my $fullname = $self->fullname();

    system ("mkdir -p $path");

    my $root_out = new IO::File ">$root_filename" or die "Unable to open $filename: $!";
    print $root_out "# Autogenerated Makefile -*- makefile -*-\n";
    print $root_out "# Copyright (C) 2009-2010 Ross Biro.\n";
    print $root_out "# Do not edit. Do not Distribute.\n";
    print $root_out "#\n";
    print $root_out "-include make_parts/begin/*.inc\n";
    print $root_out "\n";
    print $root_out "-include $filename\n";
    print $root_out "\n";
    print $root_out "-include make_parts/middle/*.inc\n";
    print $root_out "\n";
    print $root_out "\n";
    print $root_out "-include make_parts/end/*.inc\n";
    print $root_out "\n";
    print $root_out "runtests: runtests_$fullname\n";
    print $root_out "\n";
    print $root_out "valgrind: valgrind_$fullname\n";
    print $root_out "\n";
    $root_out->close();

    my $out = new IO::File ">$filename" or die "Unable to open $filename: $!";

    $filename = $self->makefile();

    print $out "# Autogenerated Makefile -*- makefile -*-\n";
    print $out "# Copyright (C) 2009-2013 Ross Biro.\n";
    print $out "# Do not edit. Do not Distribute.\n";
    my $protname = $filename;
    $protname =~ s:/:_:g;
    print $out "ifndef $protname\n";
    print $out "$protname=1\n";
    print $out "#\n";
    print $out "-include make_parts/begin/*.inc\n";
    print $out "\n";
    print $out "#\n";
    print $out "# Environment variables from .mod file.\n";
    print $out "#\n";

    my $key;
    if (ref($self->{env}) eq 'HASH') {
	for $key (keys(%{$self->{env}})) {
	    print $out "$key=$self->{env}->{$key}\n";
	}
    }

    print $out "#\n";
    print $out "# End of environment variables from .mod file.\n";
    print $out "#\n";
    print $out "\n";

    print $out "#\n";
    print $out "# generic target generated from .mod file.\n";
    print $out "#\n";


    print $out "all: dependencies ";

    if ($self->{others}) {
	print $out "others_$fullname ";
    }
    
    my $target = $self->target();
    

    print $out "$target\n\n";
    print $out ".PHONY: clean thirdparty dependencies \n\n";

    print $out "dependencies deps : ".$self->dependencyTarget()."\n\n";
    print $out $self->dependencyTarget().":\n";
    print $out "\n\n";

    print $out "clean::\n";
    print $out "\t rm -rf $self->{binpath}\n";
    print $out "\t rm -rf $self->{objpath}\n";
    print $out "\t rm -rf $self->{makepath}\n";
    print $out "\n";
    print $out "$target: $filename\n";
    print $out "\n";
    print $out "$filename: $self->{base}\n";
    print $out "\n";
    print $out "\$(CCPROTOCOMPILER): thirdparty\n";
    print $out "\n";

    print $out "#\n";
    print $out "# End of generic target generated from .mod file.\n";
    print $out "#\n";
    print $out "\n";

    print $out "#\n";
    print $out "# Middle includes.\n";
    print $out "#\n";

#    print $out "\n";
#    print $out "-include make_parts/middle/*.inc\n";
#    print $out "\n";

    print $out "#\n";
    print $out "# End of Middle includes.\n";
    print $out "#\n";
    print $out "\n";

    $self->dependencies($out, \%collection);

    print $out "#\n";
    print $out "# End of internal dependencies.\n";
    print $out "#\n";
    print $out "\n";

    print $out "#\n";
    print $out "# Tests.\n";
    print $out "#\n";

    my $fullname = $self->fullname();
    my $test;

    foreach $test (@{$self->{unittests}}) {
	if (ref($test) eq 'HASH') {
	    bless $test, 'Module';
	    $test->{__FILES__} = $self->{__FILES__};
	    $test->{__PARENT__} = $self;
	} else {
	    die "Bad unittest specification";
	}
	if ($test->{timeallowed}) {
	    print $out "timeallowed_".$test->target()."=".$test->{timeallowed}."\n";
	}
    }

    my $other;
    foreach $other (@{$self->{others}}) {
	if (ref($other) eq 'HASH') {
	    bless $other, 'Module';
	    $other->{__FILES__} = $self->{__FILES__};
	} else {
	    die "Bad other specification";
	}
    }

    print $out "runtests_$fullname : dependencies ";

    $self->{unittests} = [] unless (defined ($self->{unittests}));
    foreach $test (@{$self->{unittests}}) {
	print $out "\\\n\t".$test->target();
    }
    print $out "\n\t\$(call run_all_tests,\$^)";
    print $out "\n\n";

    print $out "valgrind_$fullname : dependencies ";

    foreach $test (@{$self->{unittests}}) {
	print $out "\\\n\t".$test->target();
    }
    print $out "\n\t\$(call valgrind_all_tests,\$^)";
    print $out "\n\n";

    foreach $test (@{$self->{unittests}}) {
	$test->dependencies($out, \%collection);
	print $out "\n";
	print $out "\n";
   }

    print $out "#\n";
    print $out "# End of tests.\n";
    print $out "#\n";

    print $out "#\n";
    print $out "# Beginning of other targets.\n";
    print $out "#\n";

    print $out "others_$fullname : dependencies ";
    foreach $other (@{$self->{others}}) {
	print $out "\\\n\t".$other->target();
    }

    print $out "\n";
    print $out "\n";

    foreach $other (@{$self->{others}}) {
	$other->dependencies($out, \%collection);
	print $out "\n";
	print $out "\n";
    }

    print $out "#\n";
    print $out "# End of other targets.\n";
    print $out "#\n";

    my $depfile;

    print $out $self->dependencyTarget()." : ";
    if ($collection{'.h'}) {
	foreach $depfile (@{$collection{'.h'}})
	{
	    print $out "\\\n\t$depfile";
	}
    }

    foreach $depfile (@{$collection{'.d'}})
    {
	print $out "\\\n\t$depfile";
    }

    print $out "\n\n";

    foreach $depfile (@{$collection{'.d'}})
    {
	print $out "-include $depfile\n";
    }

    print $out "\n";

    print $out "#\n";
    print $out "# End includes.\n";
    print $out "#\n";

#    print $out "\n";
#    print $out "-include make_parts/end/*.inc\n";
#    print $out "\n";

    print $out "#\n";
    print $out "# End of end includes.\n";
    print $out "#\n";
    print $out "\n";

    print $out "#\n";
    print $out "# End of Makefile.\n";
    print $out "#\n";
    print $out "\n";

    print $out "endif  #$protname\n";

    close $out;
}

sub main {
    my @files = @_;
    my $file_name;
    
    # Getopt::Long::GetOptionsFromArray(\@_, 'outputfile=s,' => \$outputfile) or die "Invalid Options";

    if (@files) {
	my $file;
	my @args;
	for $file (@files) {
	    push @args, { 'name' => $file };
	}
	my $root = textus::ConfigFile::parseFile(@args);
	bless $root, 'Module';
	$externalModuleCache{$args[0]->{name}} = $root;
	if (ref($root) ne 'Module') {
	    print STDERR $root."\n";
	    return -1;
	}
	$root->writeMakefile();
    }
}

package main;

Module::main(@ARGV) unless (caller);

# Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
# All rights reserved.
# This component and the accompanying materials are made available
# under the terms of "Eclipse Public License v1.0"
# which accompanies this distribution, and is available
# at the URL "http://www.eclipse.org/legal/epl-v10.html".
#
# Initial Contributors:
# Nokia Corporation - initial contribution.
#
# Contributors:
#
# Description:
# Generate a link-dependency graph
# Given a baseline list of components, look through the BLDMAKE
# generated files to find the individual DLL and EXE makefiles.
# Scan those makefile to find out which .LIB files are generated,
# and which .LIB files are required, thereby deducing the 
# component dependency graph.
# 
#

my @components;
my %component_releases;
my %components_by_lib;
my %libs_needed;
my %component_deps;
my $errors = 0;
my @platforms = ("WINS", "ARMI", "MAWD");
my $makefile_count=0;

while (<>)
	{
	s/\s*#.*$//;
	if ($_ =~ /^$/)
		{
		next;
		}

	if ($_ =~ /<option (\w+)(.*)>/)
		{
		# placeholder
		next;	
		}

	push @components, lc $_;
	}

scan_bldmakes();

if ($makefile_count==0)
	{
	error("No makefiles scanned!??");
	}

if (%libs_needed==0)
	{
	error("No libraries needed!??");
	}

foreach $lib (sort keys %libs_needed)
	{
	if ($components_by_lib{$lib} eq "")
		{
		error("library $lib is not produced by any component!");
		$components_by_lib{$lib} = "($lib)";
		}
	}

if ($errors > 0)
	{
	print "\n";
	}

foreach $component (sort keys %component_deps)
	{
	my %dependencies;

	foreach $lib (split /\s+/, $component_deps{$component})
		{
		next if ($lib eq "");
		my $dependent = $components_by_lib{$lib};
		$dependencies{$dependent} = 1;
		}

	print "$component $component_releases{$component}:";
	foreach $dependent (sort keys %dependencies)
		{
		next if ($dependent eq $component);
		print " $dependent";
		}
	print "\n";
	}

sub scan_bldmakes
	{
	foreach $line (@components)
		{
		next if ($line =~ /<special (\w+)(.*)>/);
		my ($name, $groupdir, $subdir, $release) = split /\s+/,$line;
		$component_releases{$name} = $release;
		my $bldmake = "\\epoc32\\build\\$groupdir";
		if (! -d $bldmake)
			{
			error("bldmake failed for $name $release");
			next;
			}
		foreach $platform (@platforms)
			{
			scan_bldmake_makefile($name,"$bldmake\\$platform.make");
			}
		}
	}

exit ($errors > 0);

sub error
	{
	my ($text) = @_;
	print "# ERROR: $text\n";
	$errors+=1;
	}

# In \epoc32\build\<place>\wins.make
#
# SAVESPACEAPPARC :
#	nmake -nologo $(VERBOSE) -f "\EPOC32\BUILD\APPARC\GROUP\WINS\APPARC.WINS" $(CFG) CLEANBUILD$(CFG)
#
# repeated N times

sub scan_bldmake_makefile
	{
	my ($component, $makefile) = @_;
	if (! -e $makefile) 
		{
		return;
		}
	open FILE, "<$makefile" or error("Can't open $makefile") and return;
	while ($line = <FILE>)
		{
		if ($line =~ /^SAVESPACE/)
			{
			$line = <FILE>;
			if ($line =~ /-f "(\S+)"\s/i)
				{
				scan_mmp_makefile($component,$1);
				}
			}
		}
	close FILE;
	}

# In \EPOC32\MAKE\LEXICON\WINS\INSO.WINS
#
# # MMPFile \LEXICON\GROUP\INSO.MMP
# # Target INSO.LIB
# # TargetType LIB
# # GeneralTargetType LIB
#

# In \EPOC32\BUILD\APPARC\GROUP\WINS\APPARC.WINS
#
# LIBRARY : "$(EPOCLIB)UDEB\APPARC.LIB"
#
# LIBS= \
#	"$(EPOCLINKDEB)\EUSER.LIB" \
#	"$(EPOCLINKDEB)\EFSRV.LIB" \
#	"$(EPOCLINKDEB)\GDI.LIB" \
#	"$(EPOCLINKDEB)\ESTOR.LIB"
#
sub scan_mmp_makefile
	{
	my ($component, $makefile) = @_;
	open SUBFILE, "<$makefile" or error("Can't open mmp $makefile for $component") and return;
	$makefile_count++;
	while ($line = <SUBFILE>)
		{
		if ($line =~ /^LIBRARY : .*\\(\S+)"/ || 
			$line =~ /^# Target (\S+\.LIB)/)
			{
			my $built_lib = $1;
			my $previous = $components_by_lib{$built_lib};
			if ($previous && $previous ne $component)
				{
				error("$built_lib is generated by $component and $previous");
				next;
				}
			$components_by_lib{$built_lib} = $component;
			next
			}
		if ($line =~ /^LIBS= \\/)
			{
			while ($line = <SUBFILE>)
				{
				if ($line =~ /\\(\S+)"/)
					{
					$component_deps{$component} .= " $1";
					$libs_needed{$1} = 1;
					}
				if ($line !~ /\\$/)
					{
					last;
					}
				}
			}
		}
	close SUBFILE;
	}



#!/usr/bin/perl -w
# converts a text file into a text file containing a std::string declaration
# with the input file as data.

use strict;
use File::Basename;

my $filename = shift;
my $stringname = $filename;
$stringname =~ s/\./_/g;
my $base = basename($stringname);

open( FILE, "<$filename" ) or die "Can't open $filename: $!";

print "//Generated file - Edit $filename!\n";
print "static const std::string $base = \"";

foreach my $line (<FILE>)
{
    chomp( $line );
    print "$line\\n ";
}

print "\";\n";



#!/usr/bin/perl -w

use strict;

my $filename = shift;
my $stringname = $filename;
$stringname =~ s/\./_/g;

open( FILE, "<$filename" ) or die "Can't open $filename: $!";

print "static const std::string $stringname = \"";

foreach my $line (<FILE>)
{
    chomp( $line );
    print "$line\\n ";
}

print "\";\n";



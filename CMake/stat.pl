#!/usr/bin/perl -w
# To use:
#   make | & CMake/stat.pl > tt.csv
#   open -a numbers tt.csv

use strict;

my $line;
my $lastPercent = 0;

while( <STDIN> )
{
    if( /(\d+)...Built target (\w+)/ )
    {
        my $abs = $1-$lastPercent;
        print "$2, $abs\n";
        $lastPercent = $1;
    }
}

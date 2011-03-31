#!/usr/bin/perl -w
# Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch> 
#  needs nodes.txt in the local directory, one hostname per line
#  provide local IP as first argument

use strict;
use Cwd;
use Env;
use File::Basename;

my $localnode = $ARGV[0];
$localnode or die "Usage: $0 <localIP> [<numPackets>]";

my $num = $ARGV[1];
$num or $num = 10;

open FILE, "< nodes.txt" or die "Can't open nodes.txt: $!";
my @nodes = <FILE>;
close( FILE );

my $netperf;
my $uname = `uname`;
chomp( $uname );

my @netperfDirs = ( ".", dirname( $0 ), "build/$uname/bin" );
foreach my $dir (@netperfDirs)
{
    if( -e $dir . "/netperf" )
    {
        $netperf = $dir . "/netperf";
    }
}

$netperf or die "netperf not found";

if( !($netperf =~ /^\//) )
{
    $netperf = getcwd() . "/$netperf";
}

my @pids;
foreach my $node (@nodes)
{
    chomp( $node );
    push( @pids, launchNetperf( $node ));
}

sleep( 1 ); # give clients some time to start
#print "$netperf -n $num -c RSP#102400#239.255.42.43#$localnode#4242#default#\n";
system( "$netperf -n $num -c RSP#102400#239.255.42.43#$localnode#4242#default# 2>&1" );

foreach my $pid (@pids)
{
    waitpid( $pid, 0 );
}

sub launchNetperf
{
    my $node = $_[0];
    my $pid = fork();

    if( $pid == 0 )
    {
        my $cmdLine = "ssh $node env LD_LIBRARY_PATH=" .
            $ENV{"LD_LIBRARY_PATH"} . " DYLD_LIBRARY_PATH=" .
            $ENV{"DYLD_LIBRARY_PATH"} .
            " $netperf -s RSP\\\\#102400\\\\#239.255.42.43\\\\#$node\\\\#4242\\\\#default\\\\# 2>&1";
        print "$cmdLine\n";
        my @result = `$cmdLine`;
        open FILE, "> $node.log" or die "Can't open $node.log: $!";
        foreach my $line (@result)
	{
	    print FILE "$node: $line";
	}
        close( FILE );
        exit(0);
    }
    else
    {
        return $pid;
    }
}


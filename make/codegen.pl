#!/usr/bin/perl -w

# Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
# All rights reserved.

use strict;
use File::Basename;

sub generate( $; $ );

my $HEADER;
my $CODE;

foreach my $file (@ARGV)
{
    open( FILE, "<$file" ) or die "Can't open $file: $!";
    
    my $basename = basename( $file, ".h" );
    open( HEADER, ">$basename" . "Packets.h" ) 
        or die "Can't open $basename" . "Packets.h for writing";
    open( CODE, ">$basename" . "Dist.cpp" ) 
        or die "Can't open $basename" . "Dist.cpp for writing";
    
    print HEADER "#include \"packets.h\"\n";
    print HEADER "namespace eqNet\n";
    print HEADER "{\n";

    print CODE   "#include \"$basename.h\"\n";
    print CODE   "#include \"$basename" . "Packets.h\"\n";

    # Can't think of an easy way to determine necessary dependencies, so
    # just hardcode them here.
    print CODE   "#include \"user.h\"\n";

    print CODE   "using namespace eqNet;\n";

    my $parseFunction = 0;
    my $function;
    my $class;

    foreach my $line (<FILE>)
    {
        if( $parseFunction )
        {
            $function .= $line;
            chomp( $function );

            if( $line =~ /;/ )
            {
                $function =~ s/;//;
                generate( $class, $function );
                $function = "";
                $parseFunction = 0;
            }
        }
        
        if( $line =~ /^\s*\/\/\s*__eq_generate_distributed__/ )
        {
            $parseFunction = 1;
        }
        
        if( $line =~ /^\s*class\s*(\w+)\s+/ )
        {
            $class = $1;
            $class =~ s/\s*$//;
        }
    }

    print HEADER "}\n";

    close CODE;
    close HEADER;
    close FILE;
}

sub generate( $; $ )
{
    my $class = shift;
    my $function = shift;

    if( !($function =~ /([\w\*\&]+)\s+(\w+)\s*\((.*)\)/ ))
    {
        print STDERR "Can't parse function '$function'\n";
        exit(-1);
    }

    my $functionRet  = $1;
    my $functionName = $2;
    my @functionArgs = split( /,/, $3 );

    my $capFunctionName = $functionName;
    $capFunctionName =~ s/^(\w)/uc($1)/e; # capitalize 1st letter

    my $packetType = "$class" . $capFunctionName . "Packet";

    my $command = $capFunctionName;
    $command =~ s/([A-Z])/_$1/g;
    $command = "CMD_$class$command";
    $command = uc($command);

    print HEADER "\n";
    print HEADER "    struct  $packetType : public $class" . "Packet\n";
    print HEADER "    {\n";
    print HEADER "        $packetType()\n";
    print HEADER "        {\n";
    print HEADER "            command = $command;\n";
    print HEADER "            size    = sizeof($packetType);\n";
    print HEADER "        }\n";

    print CODE "\n";
    print CODE "$functionRet $class" . "::$functionName(@functionArgs)\n";
    print CODE "{\n";
    print CODE "    $packetType packet;\n";

    my $idClass = $class;
    $idClass =~ s/^(\w)/lc($1)/e; # lowercase first letter
    $idClass = 0 if $idClass =~ /node/;

    # fill up entity identifiers
    while( $idClass )
    {
        if( $idClass =~ /$class/i )
        {
            print CODE "    packet.$idClass" . "ID = getID();\n";
        }
        else
        {
            print CODE "    packet.$idClass" . "ID = _$idClass->getID();\n";
        }

        if( $idClass eq "user" )
        {
            $idClass = "session";
        }
        elsif( $idClass eq "session" )
        {
            $idClass = 0;
        }
    }

    # fill up arguments
    foreach my $arg (@functionArgs)
    {
        if( !($arg =~ /([\w\*\&]+)\s+(\w+)\s*$/) )
        {
            print STDERR
                "Can't parse argument '$arg' of function '$function'\n";
            exit(-1);
        }

        my $type = $1;
        my $name = $2;
        $type =~ s/\&$//;

        if( $type =~ /\*$/ ) # pointer to object -> use ID
        {
            print HEADER "        uint $name" . "ID;\n";
            print CODE   "    packet.$name" . "ID = $name->getID();\n";
        }
        else
        {
            print HEADER "        $type $name;\n";
            print CODE   "    packet.$name = $name;\n";
        }
    }

    # has return value - allocate request ID
    if( !($functionRet =~ /void/) )
    {
        print HEADER "        uint requestID;\n";
        print CODE 
            "    packet.requestID = _requestHandler.registerRequest(NULL);\n";
    }

    print CODE "    send( packet );\n";

    # handle return value
    if( !($functionRet =~ /void/) )
    {
        print CODE "    return ($functionRet)_requestHandler.waitRequest( packet.requestID );\n";
    }

    print CODE "}\n";
    print HEADER "    };\n";
}

#!/usr/bin/perl -w

# Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
# All rights reserved.

use strict;
use File::Basename;

sub generateAsync( $; $; $ );

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
    
    print "HEADER #include \"packets.h\"\n";

    print "CODE   #include \"$basename.h\"\n";
    print "CODE   #include \"$basename" . "Packets.h\"\n";

    my $parseFunction = 0;
    my $namespace;
    my $class;
    my $function;

    foreach my $line (<FILE>)
    {
        if( $parseFunction )
        {
            $function .= $line;
            chomp( $function );

            if( $line =~ /;/ )
            {
                $function =~ s/;//;
                generateAsync( $namespace, $class, $function );
                $function = "";
                $parseFunction = 0;
            }
        }
        
        if( $line =~ /^\s*\/\/\s*__eq_generate_async__/ )
        {
            $parseFunction = 1;
        }
        elsif( $line =~ /^\s*namespace\s+(\w+)\s*/ )
        {
            $namespace = $1;
            chomp( $namespace );
        }
        elsif( $line =~ /^\s*class\s+(\w+)\s*/ )
        {
            $class = $1;
            chomp( $class );
        }
    }

    close CODE;
    close HEADER;
    close FILE;
}

# generates code for an asynchronous function, with two sub-methods
# send<Func> and sync<Func>.
sub generateAsync( $; $; $ )
{
    my $namespace = shift;
    my $class     = shift;
    my $function  = shift;

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
   
    my $packetNamespace = $namespace;
    $packetNamespace =~ s/^eqs$/eq/; # eqs packages are defined in client ns

    print "HEADER \n";
    print "HEADER namespace $packetNamespace\n";
    print "HEADER {\n";
    print "HEADER     struct  $packetType : public $class" . "Packet\n";
    print "HEADER     {\n";
    print "HEADER         $packetType()\n";
    print "HEADER         {\n";
    print "HEADER             command = $command;\n";
    print "HEADER             size    = sizeof( $packetType );\n";
    print "HEADER         }\n";
    print "HEADER         uint32_t requestID;\n";
    
    # asynchronous send function
    print "CODE   \n";
    print "CODE   void $namespace" . "::$class" . 
        "::send$capFunctionName( @functionArgs )\n";
    print "CODE   {\n";
    print "CODE       ASSERT( _pendingRequestID == INVALID_ID );\n";
    print "CODE       $packetType packet;\n";

    my $idClass = $class;
    $idClass =~ s/^(\w)/lc($1)/e; # lowercase first letter

    # fill up entity identifiers
    while( $idClass )
    {
        if( $idClass =~ /$class/i )
        {
            print "CODE       packet.$idClass" . "ID = getID();\n";
        }
        else
        {
            print "CODE       packet.$idClass" . "ID = _$idClass->getID();\n";
        }

        if( $idClass eq "node" )
        {
            $idClass = "config";
        }
        elsif( $idClass eq "config" )
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
            print "HEADER         uint32_t $name" . "ID;\n";
            print "CODE       packet.$name" . "ID = $name->getID();\n";
        }
        else
        {
            print "HEADER         $type $name;\n";
            print "CODE       packet.$name = $name;\n";
        }
    }
    print "HEADER     };\n";
    print "HEADER }\n";


    # allocate request ID
    print "CODE       _pendingRequestID = _requestHandler.registerRequest();\n";
    print "CODE       packet.requestID  = _pendingRequestID;\n";
    print "CODE       send( packet );\n";
    print "CODE   }\n";

    # synchronization function w/ return value
    print "CODE   $functionRet $namespace" . "::$class" . 
        "::sync$capFunctionName()\n";
    print "CODE   {\n";
    print "CODE       ASSERT( _pendingRequestID != INVALID_ID );\n";
    if( $functionRet =~ /void/ )
    {
        print "CODE       _requestHandler.waitRequest( _pendingRequestID );\n";
        print "CODE       _pendingRequestID = INVALID_ID;\n";
    }
    else
    {
        print "CODE       const uint32_t requestID = _pendingRequestID;\n";
        print "CODE       _pendingRequestID    = INVALID_ID;\n";
        print "CODE       return ($functionRet)_requestHandler.waitRequest( requestID );\n";
    }
    print "CODE   }\n";

    # synchronous function
    print "CODE   $functionRet $namespace" . "::$class" . 
        "::$functionName(  @functionArgs )\n";
    print "CODE   {\n";
    print "CODE       send$capFunctionName( ";
    
    my $needKomma = 0;
    foreach my $arg (@functionArgs)
    {
        $arg =~ /[\w\*\&]+\s+(\w+)\s*$/;
        if( $needKomma )
        {
            print "CODE   , $1";
        }
        else
        {
            print "CODE    $1";
            $needKomma = 1;
        }
    }
    print "CODE   )\n";
    if( $functionRet =~ /void/ )
    {
        print "CODE       sync$capFunctionName();\n";
    } 
    else
    {
        print "CODE       return sync$capFunctionName();\n";
    }

    print "CODE   }\n";
}

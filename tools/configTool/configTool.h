
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com>
   All rights reserved. */

#ifndef EQ_CONFIGTOOL_H
#define EQ_CONFIGTOOL_H

#include <string>

class ConfigTool
{
public:
    ConfigTool();
    bool parseArguments( int argc, char** argv );
    void writeConfig() const;

private:
    enum Mode
    {
        MODE_2D = 0,
        MODE_DB,
        MODE_DB_DS,
        MODE_DB_DS_AC,
        MODE_ALL
    }
        _mode;

    unsigned _nPipes;
    unsigned _nChannels;
    bool     _useDestination;
    bool     _fullScreen;

    std::string _nodesFile;

    void _writeResources()  const;
    void _writeCompound()   const;
    void _write2D()         const;
    void _writeDB()         const;
    void _writeDS()         const;
    void _writeDSAC()       const;
};

#endif // EQ_CONFIGTOOL_H

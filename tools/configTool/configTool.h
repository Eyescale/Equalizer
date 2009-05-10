
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

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
        MODE_DB_STREAM,
        MODE_DB_DS_AC,
        MODE_DPLEX,
        MODE_WALL,
        MODE_ALL
    }
        _mode;

    unsigned _nPipes;
    unsigned _nChannels;
    bool     _useDestination;
    bool     _fullScreen;
    unsigned _columns, _rows;

    std::string _nodesFile;
    std::string _descrFile;

    unsigned _resX, _resY;


    void _writeResources()  const;
    void _writeCompound()   const;
    void _write2D()         const;
    void _writeDB()         const;
    void _writeDBStream()   const;
    void _writeDS()         const;
    void _writeDSAC()       const;
    void _writeDPlex()      const;
    void _writeWall()       const;
    void _writeFromDescription() const;

};

#endif // EQ_CONFIGTOOL_H

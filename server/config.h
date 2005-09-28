
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_CONFIG_H
#define EQS_CONFIG_H

#include <iostream>
#include <vector>

namespace eqs
{
    class Compound;
    class Window;

    /**
     * The config.
     */
    class Config
    {
    public:
        /** 
         * Constructs a new Config.
         */
        Config();

        /** 
         * Adds a new window to this config.
         * 
         * @param window the window.
         */
        void addWindow( Window* window ){ _windows.push_back( window ); }

        /** 
         * Removes a window from this config.
         * 
         * @param window the window
         * @return <code>true</code> if the window was removed, 
         *         <code>false</code> otherwise.
         */
        bool removeWindow( Window* window );

        /** 
         * Returns the number of windows on this config.
         * 
         * @return the number of windows on this config. 
         */
        uint nWindows() const { return _windows.size(); }

        /** 
         * Gets a window.
         * 
         * @param index the window's index. 
         * @return the window.
         */
        Window* getWindow( const uint index ) const
            { return _windows[index]; }

        /** 
         * Adds a new compound to this config.
         * 
         * @param compound the compound.
         */
        void addCompound( Compound* compound )
            { _compounds.push_back( compound ); }

        /** 
         * Removes a compound from this config.
         * 
         * @param compound the compound
         * @return <code>true</code> if the compound was removed,
         *         <code>false</code> otherwise.
         */
        bool removeCompound( Compound* compound );

        /** 
         * Returns the number of compounds on this config.
         * 
         * @return the number of compounds on this config. 
         */
        uint nCompounds() const { return _compounds.size(); }

        /** 
         * Gets a compound.
         * 
         * @param index the compound's index. 
         * @return the compound.
         */
        Compound* getCompound( const uint index ) const
            { return _compounds[index]; }

    protected:
        /** 
         * Constructs a new deep copy of another config.
         * 
         * @param from the original config.
         */
        Config(const Config& from);

    private:
        /** The list of compounds. */
        std::vector<Compound*> _compounds;

        /** The list of windows. */
        std::vector<Window*>   _windows;
    };

    inline std::ostream& operator << ( std::ostream& os, const Config* config )
    {
        if( !config )
        {
            os << "NULL config";
            return os;
        }

        const uint nWindows = config->nWindows();
        const uint nCompounds = config->nCompounds();
        os << "config " << (void*)config << " " << nWindows << " windows "
           << nCompounds << " compounds";

        for( uint i=0; i<nWindows; i++ )
            os << std::endl << "    " << config->getWindow(i);

        for( uint i=0; i<nCompounds; i++ )
            os << std::endl << "    " << config->getCompound(i);

        return os;
    }
};
#endif // EQS_CONFIG_H

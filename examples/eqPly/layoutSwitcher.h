
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQPLY_LAYOUTSWITCHER_H
#define EQPLY_LAYOUTSWITCHER_H

namespace eqPly
{
/** Helper to find the next view in a config. */
class LayoutSwitcher : public eq::ConfigVisitor
{
public:
    LayoutSwitcher( const uint32_t currentViewID ) 
            : _id( currentViewID ), _layout( 0 ) {}
    virtual ~LayoutSwitcher(){}

    const eq::Layout* getResult() const { return _layout; }

protected:
    virtual eq::VisitorResult visitPre( eq::Config* config )
        {
            if( _id == EQ_ID_INVALID )
                return eq::TRAVERSE_TERMINATE;
            return eq::TRAVERSE_CONTINUE;
        }

    virtual eq::VisitorResult visit( eq::View* view )
        {
            if( _layout || _id != view->getID( ))
                return eq::TRAVERSE_CONTINUE;
                
            _layout = view->getLayout();
            eq::Config* config = view->getConfig();
            config->accept( *this ); // reinvoke, find first canvas with layout
            return eq::TRAVERSE_TERMINATE;
        }

    virtual eq::VisitorResult visitPre( eq::Canvas* canvas )
        {
            if( !_layout || _layout != canvas->getLayout( ))
                return eq::TRAVERSE_PRUNE; // no need to visit segments

            // found first canvas with layout, switch to next layout
            const eq::Config*       config  = canvas->getConfig();
            const eq::LayoutVector& layouts = config->getLayouts();
            EQASSERT( !layouts.empty( ));

            eq::LayoutVector::const_iterator i;
            for( i = layouts.begin(); i != layouts.end(); ++i )
            {
                if( *i != _layout )
                    continue;
                            
                ++i;
                break;
            }
            if( i == layouts.end( ))
                i = layouts.begin(); // wrap around
            
            _layout = *i;
            canvas->useLayout( _layout );
            EQINFO << "Using layout " << _layout->getName() << " on " 
                   << canvas->getID() << std::endl;

            return eq::TRAVERSE_TERMINATE; 
        }

private:
    const uint32_t _id;
    eq::Layout*    _layout;
};

}
#endif // EQPLY_LAYOUTSWITCHER_H

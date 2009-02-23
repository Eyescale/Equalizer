
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQPLY_NEXTVIEWFINDER_H
#define EQPLY_NEXTVIEWFINDER_H

namespace eqPly
{
/** Helper to find the next view in a config. */
class NextViewFinder : public eq::ConfigVisitor
{
public:
    NextViewFinder( const uint32_t currentViewID ) 
            : _id( currentViewID ), _layout( 0 ), _result( 0 )
            , _stopNext( false ) {}
    virtual ~NextViewFinder(){}

    const eq::View* getResult() const { return _result; }

protected:
    virtual eq::VisitorResult visitPre( eq::Canvas* canvas )
        {
            _layout = canvas->getLayout();

            if( _layout && _layout->accept( *this ) == eq::TRAVERSE_TERMINATE )
                return eq::TRAVERSE_TERMINATE;

            _layout = 0;
            return eq::TRAVERSE_PRUNE;
        }

    virtual eq::VisitorResult visitPre( eq::Layout* layout )
        {
            if( _layout != layout )
                return eq::TRAVERSE_PRUNE; // only consider used layouts
            return eq::TRAVERSE_CONTINUE; 
        }

    virtual eq::VisitorResult visit( eq::View* view )
        {
            if( _stopNext || _id == EQ_ID_INVALID )
            {
                _result = view;
                return eq::TRAVERSE_TERMINATE;
            }
            
            if( view->getID() == _id )
                _stopNext = true; 
            return eq::TRAVERSE_CONTINUE; 
        }

private:
    const uint32_t _id;
    eq::Layout*    _layout;
    eq::View*      _result;
    bool           _stopNext;
};

}
#endif // EQPLY_NEXTVIEWFINDER_H

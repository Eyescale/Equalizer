
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
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


/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
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

#ifndef EQ_PLY_MODELASSIGNER_H
#define EQ_PLY_MODELASSIGNER_H

namespace eqPly
{
/** Helper to assign models to views. */
class ModelAssigner : public eq::ConfigVisitor
{
public:
    ModelAssigner( const ModelDists& models ) 
            : _models( models ), _current( models.begin( )), _layout( 0 ) {}

    virtual eq::VisitorResult visit( eq::View* view )
        {
            const ModelDist* model = *_current;
            static_cast< View* >( view )->setModelID( model->getID( ));

            ++_current;
            if( _current == _models.end( ))
                _current = _models.begin(); // wrap around

            return eq::TRAVERSE_CONTINUE; 
        }

private:
    const ModelDists&          _models;
    ModelDists::const_iterator _current;
    
    eq::Layout* _layout;
};

}
#endif // EQ_PLY_MODELASSIGNER_H

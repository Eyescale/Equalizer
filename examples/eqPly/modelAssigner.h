
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

#ifndef EQPLY_MODELASSIGNER_H
#define EQPLY_MODELASSIGNER_H

namespace eqPly
{
/** Helper to assign models to views. */
class ModelAssigner : public eq::ConfigVisitor
{
public:
    ModelAssigner( const ModelDistVector& models ) 
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
    const ModelDistVector&          _models;
    ModelDistVector::const_iterator _current;
    
    eq::Layout* _layout;
};

}
#endif // EQPLY_MODELASSIGNER_H

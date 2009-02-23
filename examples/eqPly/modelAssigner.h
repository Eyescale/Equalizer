
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

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

    virtual eq::VisitorResult visitPre( eq::Canvas* canvas )
        {
            _layout = canvas->getLayout();
            if( _layout )
                _layout->accept( *this );

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

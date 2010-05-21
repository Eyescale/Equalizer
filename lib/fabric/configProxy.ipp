
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch>
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

namespace eq
{
namespace fabric
{

template< class S, class C, class O, class L, class CV, class N, class V >
class ConfigProxy : public Object //!< Used for data distribution 
{
public:
    ConfigProxy( Config< S, C, O, L, CV, N, V >& config );
    virtual ~ConfigProxy() {}

    void create( O** observer ) 
        {
            *observer = _config.getServer()->getNodeFactory()->createObserver(
                static_cast< C* >( &_config ));
        }
    void release( O* observer ) 
        { _config.getServer()->getNodeFactory()->releaseObserver( observer ); }

    void create( L** layout )
        {
            *layout = _config.getServer()->getNodeFactory()->createLayout(
                static_cast< C* >( &_config ));
        }
    void release( L* layout )
        { _config.getServer()->getNodeFactory()->releaseLayout( layout ); }

    void create( CV** canvas )
        {
            *canvas = _config.getServer()->getNodeFactory()->createCanvas(
                static_cast< C* >( &_config ));
        }
    void release( CV* canvas )
        { _config.getServer()->getNodeFactory()->releaseCanvas( canvas ); }

    void create( N** node )
        {
            *node = _config.getServer()->getNodeFactory()->createNode(
                static_cast< C* >( &_config ));
        }
    void release( N* node )
        { _config.getServer()->getNodeFactory()->releaseNode( node ); }

protected:
    virtual void serialize( net::DataOStream& os, const uint64_t dirtyBits );
    virtual void deserialize( net::DataIStream& is, const uint64_t dirtyBits );

private:
    Config< S, C, O, L, CV, N, V >& _config;
    template< class, class, class, class, class, class, class >
    friend class Config;
};

template< class S, class C, class O, class L, class CV, class N, class V >
ConfigProxy< S, C, O, L, CV, N, V >::ConfigProxy( Config< S, C, O, L, CV, N, V >& config )
        : _config( config )
{}

template< class S, class C, class O, class L, class CV, class N, class V >
void ConfigProxy< S, C, O, L, CV, N, V >::serialize( net::DataOStream& os,
                                               const uint64_t dirtyBits )
{
    Object::serialize( os, dirtyBits );

    if( dirtyBits & Config< S, C, O, L, CV, N, V >::DIRTY_MEMBER )
        os << _config._appNodeID;
    if( dirtyBits & Config< S, C, O, L, CV, N, V >::DIRTY_ATTRIBUTES )
        os.write( _config._fAttributes, C::FATTR_ALL * sizeof( float ));
    if( dirtyBits & Config< S, C, O, L, CV, N, V >::DIRTY_NODES )
        os.serializeChildren( this, _config._nodes );
    if( dirtyBits & Config< S, C, O, L, CV, N, V >::DIRTY_OBSERVERS )
        os.serializeChildren( this, _config._observers );
    if( dirtyBits & Config< S, C, O, L, CV, N, V >::DIRTY_LAYOUTS )
        os.serializeChildren( this, _config._layouts );
    if( dirtyBits & Config< S, C, O, L, CV, N, V >::DIRTY_CANVASES )
        os.serializeChildren( this, _config._canvases );
    if( dirtyBits & Config< S, C, O, L, CV, N, V >::DIRTY_LATENCY )
        os << _config._data.latency;
}

template< class S, class C, class O, class L, class CV, class N, class V >
void ConfigProxy< S, C, O, L, CV, N, V >::deserialize( net::DataIStream& is, 
                                                      const uint64_t dirtyBits )
{
    Object::deserialize( is, dirtyBits );

    if( dirtyBits & Config< S, C, O, L, CV, N, V >::DIRTY_MEMBER )
        is >> _config._appNodeID;
    if( dirtyBits & Config< S, C, O, L, CV, N, V >::DIRTY_ATTRIBUTES )
        is.read( _config._fAttributes, C::FATTR_ALL * sizeof( float ));

    if( dirtyBits & Config< S, C, O, L, CV, N, V >::DIRTY_NODES )
    {
        if( _config.mapNodeObjects( ))
        {
            typename C::Nodes result;
            is.deserializeChildren( this, _config._nodes, result );
            _config._nodes.swap( result );
            EQASSERT( _config._nodes.size() == result.size( ));
        }
        else // consume unused ObjectVersions
        {
            net::ObjectVersions childIDs;
            is >> childIDs;
        }
    }

    if( _config.mapViewObjects( )) // depends on _config._appNodeID !
    {
        if( dirtyBits & Config< S, C, O, L, CV, N, V >::DIRTY_OBSERVERS )
        {
            typename C::Observers result;
            is.deserializeChildren( this, _config._observers, result );
            _config._observers.swap( result );
            EQASSERT( _config._observers.size() == result.size( ));
        }
        if( dirtyBits & Config< S, C, O, L, CV, N, V >::DIRTY_LAYOUTS )
        {
            typename C::Layouts result;
            is.deserializeChildren( this, _config._layouts, result );
            _config._layouts.swap( result );
            EQASSERT( _config._layouts.size() == result.size( ));
        }
        if( dirtyBits & Config< S, C, O, L, CV, N, V >::DIRTY_CANVASES )
        {
            typename C::Canvases result;
            is.deserializeChildren( this, _config._canvases, result );
            _config._canvases.swap( result );
            EQASSERT( _config._canvases.size() == result.size( ));
        }
    }
    else // consume unused ObjectVersions
    {
        net::ObjectVersions childIDs;
        if( dirtyBits & Config< S, C, O, L, CV, N, V >::DIRTY_OBSERVERS )
            is >> childIDs;
        if( dirtyBits & Config< S, C, O, L, CV, N, V >::DIRTY_LAYOUTS )
            is >> childIDs;
        if( dirtyBits & Config< S, C, O, L, CV, N, V >::DIRTY_CANVASES )
            is >> childIDs;
    }

    if( dirtyBits & Config< S, C, O, L, CV, N, V >::DIRTY_LATENCY )
    {
        uint32_t latency = 0;
        is >> latency;
        if( _config._data.latency != latency )
        {
            _config._data.latency = latency;
            _config.changeLatency( latency );
        }
    }
}

}
}

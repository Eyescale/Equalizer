
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
class ConfigProxy : public Object //!< @internal Used for data distribution 
{
public:
    typedef Config< S, C, O, L, CV, N, V > ConfigType;

    ConfigProxy( ConfigType& config );
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
        {
            _config.getServer()->getNodeFactory()->releaseCanvas( canvas );
        }

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
    virtual void notifyDetach();
    virtual void removeChild( const uint32_t id ) { _config._removeChild( id );}

private:
    ConfigType& _config;
    template< class, class, class, class, class, class, class >
    friend class Config;

    virtual uint32_t commitNB(); //!< @internal
};

template< class S, class C, class O, class L, class CV, class N, class V >
ConfigProxy< S, C, O, L, CV, N, V >::ConfigProxy( ConfigType& config )
        : _config( config )
{}


template< class S, class C, class O, class L, class CV, class N, class V >
uint32_t ConfigProxy< S, C, O, L, CV, N, V >::commitNB()
{
    if( Serializable::isDirty( ConfigType::DIRTY_NODES ))
        commitChildren< N >( _config._nodes );
    if( Serializable::isDirty( ConfigType::DIRTY_OBSERVERS ))
        commitChildren< O, ConfigNewObserverPacket, C >(
            _config._observers, static_cast< C* >( &_config ));
    // Always traverse layouts: view proxy objects may be dirty
    commitChildren< L, ConfigNewLayoutPacket, C >( _config._layouts,
                                                   static_cast<C*>( &_config ));
    if( Serializable::isDirty( ConfigType::DIRTY_CANVASES ))
        commitChildren< CV, ConfigNewCanvasPacket, C >(
            _config._canvases, static_cast< C* >( &_config ));
    return Object::commitNB();
}

template< class S, class C, class O, class L, class CV, class N, class V >
void ConfigProxy< S, C, O, L, CV, N, V >::serialize( net::DataOStream& os,
                                                     const uint64_t dirtyBits )
{
    Object::serialize( os, dirtyBits );

    if( dirtyBits & ConfigType::DIRTY_MEMBER )
        os << _config._appNodeID;
    if( dirtyBits & ConfigType::DIRTY_ATTRIBUTES )
    {
        os.write( _config._fAttributes, C::FATTR_ALL * sizeof( float ));
        os.write( _config._iAttributes, C::IATTR_ALL * sizeof( int32_t ));
    }
    if( isMaster( ))
    {
        if( dirtyBits & ConfigType::DIRTY_NODES )
            os.serializeChildren( _config._nodes );
        if( dirtyBits & ConfigType::DIRTY_OBSERVERS )
            os.serializeChildren( _config._observers );
        if( dirtyBits & ConfigType::DIRTY_LAYOUTS )
            os.serializeChildren( _config._layouts );
        if( dirtyBits & ConfigType::DIRTY_CANVASES )
            os.serializeChildren( _config._canvases );
    }
    if( dirtyBits & ConfigType::DIRTY_LATENCY )
        os << _config._data.latency;
}

template< class S, class C, class O, class L, class CV, class N, class V >
void ConfigProxy< S, C, O, L, CV, N, V >::deserialize( net::DataIStream& is, 
                                                      const uint64_t dirtyBits )
{
    Object::deserialize( is, dirtyBits );

    if( dirtyBits & ConfigType::DIRTY_MEMBER )
        is >> _config._appNodeID;
    if( dirtyBits & ConfigType::DIRTY_ATTRIBUTES )
    {
        is.read( _config._fAttributes, C::FATTR_ALL * sizeof( float ));
        is.read( _config._iAttributes, C::IATTR_ALL * sizeof( int32_t ));
    }
    if( isMaster( ))
    {
        if( dirtyBits & ConfigType::DIRTY_NODES )
            syncChildren( _config._nodes );
        if( dirtyBits & ConfigType::DIRTY_OBSERVERS )
            syncChildren( _config._observers );
        if( dirtyBits & ConfigType::DIRTY_LAYOUTS )
            syncChildren( _config._layouts );
        if( dirtyBits & ConfigType::DIRTY_CANVASES )
            syncChildren( _config._canvases );
    }
    else
    {
        if( dirtyBits & ConfigType::DIRTY_NODES )
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
            if( dirtyBits & ConfigType::DIRTY_OBSERVERS )
            {
                typename C::Observers result;
                is.deserializeChildren( this, _config._observers, result );
                _config._observers.swap( result );
                EQASSERT( _config._observers.size() == result.size( ));
            }
            if( dirtyBits & ConfigType::DIRTY_LAYOUTS )
            {
                typename C::Layouts result;
                is.deserializeChildren( this, _config._layouts, result );
                _config._layouts.swap( result );
                EQASSERT( _config._layouts.size() == result.size( ));
            }
            if( dirtyBits & ConfigType::DIRTY_CANVASES )
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
            if( dirtyBits & ConfigType::DIRTY_OBSERVERS )
                is >> childIDs;
            if( dirtyBits & ConfigType::DIRTY_LAYOUTS )
                is >> childIDs;
            if( dirtyBits & ConfigType::DIRTY_CANVASES )
                is >> childIDs;
        }
    }

    if( dirtyBits & ConfigType::DIRTY_LATENCY )
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

template< class S, class C, class O, class L, class CV, class N, class V >
void ConfigProxy< S, C, O, L, CV, N, V >::notifyDetach()
{
    if( isMaster( ))
        return;

    typename S::NodeFactory* nodeFactory =_config.getServer()->getNodeFactory();

    while( !_config._nodes.empty( ))
    {
        EQASSERT( _config.mapNodeObjects( ));
        N* node = _config._nodes.back();
        _config.unmapObject( node );
        _config._removeNode( node );
        nodeFactory->releaseNode( node );
    }

    while( !_config._canvases.empty( ))
    {
        EQASSERT( _config.mapViewObjects( ));
        CV* canvas = _config._canvases.back();
        _config.unmapObject( canvas );
        _config._removeCanvas( canvas );
        nodeFactory->releaseCanvas( canvas );
    }

    while( !_config._layouts.empty( ))
    {
        EQASSERT( _config.mapViewObjects( ));
        L* layout = _config._layouts.back();
        _config.unmapObject( layout );
        _config._removeLayout( layout );
        nodeFactory->releaseLayout( layout );
    }

    while( !_config._observers.empty( ))
    {
        EQASSERT( _config.mapViewObjects( ));
        O* observer = _config._observers.back();
        _config.unmapObject( observer );
        _config._removeObserver( observer );
        nodeFactory->releaseObserver( observer );
    }
}

}
}

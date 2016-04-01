
/* Copyright (c) 2013-2016, Stefan Eilemann <eile@equalizergraphics.com>
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
namespace detail
{
class InitVisitor : public ConfigVisitor
{
public:
    InitVisitor( const Strings& activeLayouts, const float modelUnit )
        : _layouts( activeLayouts ), _modelUnit( modelUnit ), _update( false )
    {}

    virtual VisitorResult visit( eq::Observer* observer )
    {
        if( observer->configInit( ))
            return TRAVERSE_CONTINUE;
        LBWARN << *observer << " initialization failed" << std::endl;
        return TRAVERSE_TERMINATE;
    }

    virtual VisitorResult visit( eq::View* view )
    {
        if( view->setModelUnit( _modelUnit ))
            _update = true;
        if( view->configInit( ))
            return TRAVERSE_CONTINUE;
        LBWARN << *view << " initialization failed" << std::endl;
        return TRAVERSE_TERMINATE;
    }

    virtual VisitorResult visitPre( eq::Canvas* canvas )
    {
        const Layouts& layouts = canvas->getLayouts();

        for( StringsCIter i = _layouts.begin(); i != _layouts.end(); ++i )
        {
            const std::string& name = *i;
            for( LayoutsCIter j = layouts.begin(); j != layouts.end(); ++j )
            {
                const eq::Layout* layout = *j;
                if( layout && layout->getName() == name &&
                    canvas->useLayout( j - layouts.begin( )))
                {
                    _update = true;
                }
            }
        }
        return TRAVERSE_CONTINUE;
    }

    bool needsUpdate() const { return _update; }

private:
    const Strings& _layouts;
    const float _modelUnit;
    bool _update;
};
}
}

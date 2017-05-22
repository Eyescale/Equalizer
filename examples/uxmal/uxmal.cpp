
/* Copyright (c) 2007-2017, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <uxmal/aabb.h>
#include <uxmal/frustum.h>

#include <seq/seq.h>

#include <eq/gl.h>
#include <lunchbox/file.h>
#include <zeroeq/subscriber.h>

#include <set>
#include <stdlib.h>

namespace uxmal
{
namespace
{
eq::Vector3f _xfm(const float x, const float y, const float z,
                  const float m[16])
{
    return eq::Vector3f(eq::Matrix4f(m, m + 16) * eq::Vector4f(x, y, z, 1.f));
}
static const std::vector<eq::Vector3f> _colors{{1, 1, 1}, {1, 0, 0}, {0, 1, 0},
                                               {0, 0, 1}, {1, 1, 0}, {1, 0, 1},
                                               {0, 1, 1}};
}

struct Box
{
    explicit Box(const eq::uint128_t& i, const Frustum& f)
        : id(i)
        , corners{_xfm(f.getLeft(), f.getBottom(), -f.getNear(), f.getView()),
                  _xfm(f.getRight(), f.getBottom(), -f.getNear(), f.getView()),
                  _xfm(f.getRight(), f.getTop(), -f.getNear(), f.getView()),
                  _xfm(f.getLeft(), f.getTop(), -f.getNear(), f.getView()),
                  _xfm(f.getLeft() * f.getFar() / f.getNear(),
                       f.getBottom() * f.getFar() / f.getNear(), -f.getFar(),
                       f.getView()),
                  _xfm(f.getRight() * f.getFar() / f.getNear(),
                       f.getBottom() * f.getFar() / f.getNear(), -f.getFar(),
                       f.getView()),
                  _xfm(f.getRight() * f.getFar() / f.getNear(),
                       f.getTop() * f.getFar() / f.getNear(), -f.getFar(),
                       f.getView()),
                  _xfm(f.getLeft() * f.getFar() / f.getNear(),
                       f.getTop() * f.getFar() / f.getNear(), -f.getFar(),
                       f.getView())}
    {
    }

    explicit Box(const eq::uint128_t& i, const AABB& b)
        : id(i)
        , corners{{b.getMin()[0], b.getMin()[1], b.getMin()[2]},
                  {b.getMin()[0], b.getMax()[1], b.getMin()[2]},
                  {b.getMax()[0], b.getMax()[1], b.getMin()[2]},
                  {b.getMax()[0], b.getMin()[1], b.getMin()[2]},
                  {b.getMin()[0], b.getMin()[1], b.getMax()[2]},
                  {b.getMin()[0], b.getMax()[1], b.getMax()[2]},
                  {b.getMax()[0], b.getMax()[1], b.getMax()[2]},
                  {b.getMax()[0], b.getMin()[1], b.getMax()[2]}}
    {
    }

    const eq::uint128_t id;
    const eq::Vector3f corners[8];
};

class Renderer : public seq::Renderer
{
    template <class O>
    void _handle(const void* data, const size_t size)
    {
        std::unique_ptr<const O> object = O::create(data, size);

        if (object->getFrame() > _dataFrame)
        {
            _dataFrame = object->getFrame();
            _boxes.clear();
            _ids.clear();
        }

        if (_dataFrame > object->getFrame() + 10) // restarted app, reset
            _dataFrame = object->getFrame();

        if (_dataFrame != object->getFrame())
            return;

        auto i = _ids.find(object->getId());
        if (i == _ids.end())
            i = _ids.insert(object->getId()).first;
        _boxes.emplace_back(Box{object->getId(), *object});
    }

public:
    Renderer(seq::Application& application)
        : seq::Renderer(application)
    {
        _subscriber.subscribe(Frustum::ZEROBUF_TYPE_IDENTIFIER(),
                              [this](const void* data, const size_t size) {
                                  _handle<Frustum>(data, size);
                              });
        _subscriber.subscribe(AABB::ZEROBUF_TYPE_IDENTIFIER(),
                              [this](const void* data, const size_t size) {
                                  _handle<AABB>(data, size);
                              });
    }
    virtual ~Renderer() {}
protected:
    bool init(co::Object*) final
    {
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        return true;
    }

    void draw(co::Object*) final
    {
        while (_subscriber.receive(0))
            /*poll ZeroEQ events for new data*/;

        applyRenderContext(); // set up OpenGL State
        applyModelMatrix();
        glDisable(GL_LIGHTING);

        static const size_t loop[] = {0, 1, 2, 3, 0, 4, 5, 6,
                                      7, 4, 7, 3, 2, 6, 5, 1};
        for (const auto& box : _boxes)
        {
            auto index =
                std::distance(_ids.begin(), _ids.find(box.id)) % _colors.size();
            const auto& color = _colors[index];
            glColor4f(color[0], color[1], color[2], 0.5f);
            glBegin(GL_LINE_LOOP);
            for (const size_t i : loop)
                glVertex3f(box.corners[i].x(), box.corners[i].y(),
                           box.corners[i].z());
            glEnd();
        }
        requestRedraw();
    }

private:
    zeroeq::Subscriber _subscriber;
    uint32_t _dataFrame = 0;

    std::vector<Box> _boxes;
    std::set<eq::uint128_t> _ids;
};

class Application : public seq::Application
{
    virtual ~Application() {}
public:
    virtual seq::Renderer* createRenderer() { return new Renderer(*this); }
    bool init(int argc, char** argv, co::Object* initData) final
    {
        for (int i = 1; i < argc; ++i)
        {
            if (std::string(argv[i]) == "--help")
            {
                std::cout << lunchbox::getFilename(argv[0])
                          << ": view and debug LOD selection and culling"
                          << std::endl
                          << getHelp() << std::endl;
                ::exit(EXIT_SUCCESS);
            }
        }
        return seq::Application::init(argc, argv, initData);
    }
};
typedef lunchbox::RefPtr<Application> ApplicationPtr;
}

int main(const int argc, char** argv)
{
    uxmal::ApplicationPtr app = new uxmal::Application;

    if (app->init(argc, argv, 0) && app->run(0) && app->exit())
        return EXIT_SUCCESS;

    return EXIT_FAILURE;
}


/* Copyright (c) 2006-2017, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
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

#include "config.h"
#include "eqPly.h"
#include "modelAssigner.h"
#include "view.h"

#include <admin/addWindow.h>
#include <admin/removeWindow.h>

using eq::KeyModifier;

namespace eqPly
{
Config::Config(eq::ServerPtr parent)
    : eq::Config(parent)
    , _spinX(5)
    , _spinY(5)
    , _advance(0)
    , _currentCanvas(0)
    , _redraw(true)
    , _numFramesAA(0)
{
}

Config::~Config()
{
    for (ModelsCIter i = _models.begin(); i != _models.end(); ++i)
        delete *i;
    _models.clear();

    for (ModelDistsCIter i = _modelDist.begin(); i != _modelDist.end(); ++i)
        delete *i;
    _modelDist.clear();
}

bool Config::init()
{
    if (!_animation.isValid())
        _animation.loadAnimation(_initData.getPathFilename());

    // init distributed objects
    if (!_initData.useColor())
        _frameData.setColorMode(COLOR_WHITE);

    _frameData.setRenderMode(_initData.getRenderMode());
    registerObject(&_frameData);
    _frameData.setAutoObsolete(getLatency());

    _initData.setFrameDataID(_frameData.getID());
    registerObject(&_initData);

    // init config
    if (!eq::Config::init(_initData.getID()))
    {
        _deregisterData();
        return false;
    }

    _loadModels();
    _registerModels();

    if (_initData.centerCamera())
        _frameData.setCameraPosition(eq::Vector3f::zero());

    const eq::Canvases& canvases = getCanvases();
    if (canvases.empty())
        _currentCanvas = 0;
    else
        _currentCanvas = canvases.front();

    _switchLayout(0);
    return true;
}

bool Config::exit()
{
    const bool ret = eq::Config::exit(); // cppcheck-suppress unreachableCode
    _deregisterData();
    _closeAdminServer();

    // retain model & distributors for possible other config runs, dtor deletes
    return ret;
}

namespace
{
static bool _isPlyfile(const std::string& filename)
{
    const size_t size = filename.length();
    if (size < 5)
        return false;

    if (filename[size - 4] != '.' || filename[size - 3] != 'p' ||
        filename[size - 2] != 'l' || filename[size - 1] != 'y')
    {
        return false;
    }
    return true;
}
}

void Config::_loadModels()
{
    if (!_models.empty()) // only load on the first config run
        return;

    eq::Strings filenames = _initData.getFilenames();
    while (!filenames.empty())
    {
        const std::string filename = filenames.back();
        filenames.pop_back();

        if (_isPlyfile(filename))
        {
            Model* model = new Model;

            if (_initData.useInvertedFaces())
                model->useInvertedFaces();
            if (!_initData.rescaleModels())
                model->disableRescaling();

            if (!model->readFromFile(filename.c_str()))
            {
                LBWARN << "Can't load model: " << filename << std::endl;
                delete model;
            }
            else
                _models.push_back(model);
        }
        else
        {
            const std::string basename = lunchbox::getFilename(filename);
            if (basename == "." || basename == "..")
                continue;

            // recursively search directories
            const eq::Strings subFiles =
                lunchbox::searchDirectory(filename, ".*");
            for (eq::StringsCIter i = subFiles.begin(); i != subFiles.end();
                 ++i)
                filenames.push_back(filename + '/' + *i);
        }
    }
}

void Config::_registerModels()
{
    for (Model* model : _models)
    {
        ModelDist* modelDist = new ModelDist(*model, getClient());
        _modelDist.push_back(modelDist);
        _frameData.setModelID(modelDist->getID());
    }

    if (!_modelDist.empty())
    {
        ModelAssigner assigner(_modelDist);
        accept(assigner);
    }
}

void Config::_deregisterData()
{
    for (auto modelDist : _modelDist)
        delete modelDist;
    _modelDist.clear();

    deregisterObject(&_initData);
    deregisterObject(&_frameData);

    _initData.setFrameDataID(eq::uint128_t());
    _frameData.setModelID(eq::uint128_t());
}

bool Config::loadInitData(const eq::uint128_t& id)
{
    LBASSERT(!_initData.isAttached());
    return getClient()->syncObject(&_initData, id, getApplicationNode());
}

const Model* Config::getModel(const eq::uint128_t& modelID)
{
    if (modelID == 0)
        return 0;

    lunchbox::ScopedWrite _mutex(_modelLock);

    const size_t nModels = _models.size();
    LBASSERT(_modelDist.size() == nModels);

    for (size_t i = 0; i < nModels; ++i)
    {
        const ModelDist* dist = _modelDist[i];
        if (dist->getID() == modelID)
            return _models[i];
    }

    _models.push_back(new Model);
    _modelDist.push_back(new ModelDist(*_models.back(), getApplicationNode(),
                                       getClient(), modelID));
    return _models.back();
}

uint32_t Config::startFrame()
{
    _updateData();
    const eq::uint128_t& version = _frameData.commit();

    _redraw = false;
    const auto frameNumber = eq::Config::startFrame(version);

    View* view = _getCurrentView();
    if (view)
        view->disableScreenshot();

    return frameNumber;
}

void Config::_updateData()
{
    // update camera
    if (_animation.isValid())
    {
        const eq::Vector3f& modelRotation = _animation.getModelRotation();
        const CameraAnimation::Step& curStep = _animation.getNextStep();

        _frameData.setModelRotation(modelRotation);
        _frameData.setRotation(curStep.rotation);
        _frameData.setCameraPosition(curStep.position);
    }
    else if (_initData.useCameraAnimation())
    {
        if (_frameData.usePilotMode())
            _frameData.spinCamera(-0.001f * _spinX, -0.001f * _spinY);
        else
            _frameData.spinModel(-0.001f * _spinX, -0.001f * _spinY, 0.f);

        _frameData.moveCamera(0.0f, 0.0f, 0.001f * _advance);
    }

    // idle mode
    if (isIdleAA())
    {
        LBASSERT(_numFramesAA > 0);
        _frameData.setIdle(true);
    }
    else
        _frameData.setIdle(false);

    _numFramesAA = 0;
}

bool Config::isIdleAA()
{
    return (!_needNewFrame() && _numFramesAA > 0);
}

bool Config::needRedraw()
{
    return (_needNewFrame() || _numFramesAA > 0);
}

uint32_t Config::getAnimationFrame()
{
    return _animation.getCurrentFrame();
}

bool Config::_needNewFrame()
{
    return (_spinX != 0 || _spinY != 0 || _advance != 0 || _redraw);
}

bool Config::handleEvent(const eq::EventType type, const eq::KeyEvent& event)
{
    if (type != eq::EVENT_KEY_PRESS)
        return eq::Config::handleEvent(type, event);

    switch (event.key)
    {
    case 'z':
        _adjustEyeBase(-0.1f);
        return true;
    case 'Z':
        _adjustEyeBase(0.1f);
        return true;
    case 'y':
        _adjustModelScale(0.1f);
        return true;
    case 'Y':
        _adjustModelScale(10.0f);
        return true;
    case 't':
        _adjustTileSize(-1);
        return true;
    case 'T':
        _adjustTileSize(1);
        return true;
    case 'u':
        _frameData.toggleCompression();
        return true;

    case 'n':
    case 'N':
        _frameData.togglePilotMode();
        return true;
    case ' ':
        stopFrames();
        _spinX = 0;
        _spinY = 0;
        _advance = 0;
        _frameData.reset();
        _setHeadMatrix(eq::Matrix4f());
        return true;

    case 'i':
        _useIdleAA = !_useIdleAA;
        return true;

    case 'k':
    {
        lunchbox::RNG rng;
        if (rng.get<bool>())
            _frameData.toggleOrtho();
        if (rng.get<bool>())
            _frameData.toggleStatistics();
        if (rng.get<bool>())
            _switchCanvas();
        if (rng.get<bool>())
            _switchView();
        if (rng.get<bool>())
            _switchLayout(1);
        if (rng.get<bool>())
            _switchModel();
        if (rng.get<bool>())
            eqAdmin::addWindow(_getAdminServer(), rng.get<bool>());
        if (rng.get<bool>())
        {
            eqAdmin::removeWindow(_getAdminServer());
            _currentCanvas = 0;
        }
        if (rng.get<bool>())
            _switchViewMode();
        return true;
    }

    case 'o':
    case 'O':
        _frameData.toggleOrtho();
        return true;

    case 's':
    case 'S':
        _frameData.toggleStatistics();
        return true;

    case 'f':
        _freezeLoadBalancing(true);
        return true;

    case 'F':
        _freezeLoadBalancing(false);
        return true;

    case eq::KC_F1:
    case 'h':
    case 'H':
        _frameData.toggleHelp();
        return true;

    case eq::KC_F10:
        _switchLayoutSize();
        return true;

    case eq::KC_F11:
        _screenshot();
        return true;

    case 'd':
    case 'D':
        _frameData.toggleColorMode();
        return true;

    case 'q':
        _frameData.adjustQuality(-.1f);
        return true;

    case 'Q':
        _frameData.adjustQuality(.1f);
        return true;

    case 'c':
    case 'C':
        _switchCanvas();
        return true;

    case 'v':
    case 'V':
        _switchView();
        return true;

    case 'm':
    case 'M':
        _switchModel();
        return true;

    case 'l':
        _switchLayout(1);
        return true;
    case 'L':
        _switchLayout(-1);
        return true;

    case 'w':
    case 'W':
        _frameData.toggleWireframe();
        return true;

    case 'r':
    case 'R':
    {
        std::ostringstream os;
        os << "Switched to " << _frameData.toggleRenderMode();
        _setMessage(os.str());
        return true;
    }
    case 'g':
    case 'G':
        _switchViewMode();
        return true;
    case 'a':
        eqAdmin::addWindow(_getAdminServer(), false /* active stereo */);
        return true;
    case 'p':
        eqAdmin::addWindow(_getAdminServer(), true /* passive stereo */);
        return true;
    case 'x':
        eqAdmin::removeWindow(_getAdminServer());
        _currentCanvas = 0;
        LBASSERT(update());
        return false;

    // Head Tracking Emulation
    case eq::KC_UP:
    {
        eq::Matrix4f headMatrix = _getHeadMatrix();
        headMatrix.array[13] += 0.1f;
        _setHeadMatrix(headMatrix);
        return true;
    }
    case eq::KC_DOWN:
    {
        eq::Matrix4f headMatrix = _getHeadMatrix();
        headMatrix.array[13] -= 0.1f;
        _setHeadMatrix(headMatrix);
        return true;
    }
    case eq::KC_RIGHT:
    {
        eq::Matrix4f headMatrix = _getHeadMatrix();
        headMatrix.array[12] += 0.1f;
        _setHeadMatrix(headMatrix);
        return true;
    }
    case eq::KC_LEFT:
    {
        eq::Matrix4f headMatrix = _getHeadMatrix();
        headMatrix.array[12] -= 0.1f;
        _setHeadMatrix(headMatrix);
        return true;
    }
    case eq::KC_PAGE_DOWN:
    {
        eq::Matrix4f headMatrix = _getHeadMatrix();
        headMatrix.array[14] += 0.1f;
        _setHeadMatrix(headMatrix);
        return true;
    }
    case eq::KC_PAGE_UP:
    {
        eq::Matrix4f headMatrix = _getHeadMatrix();
        headMatrix.array[14] -= 0.1f;
        _setHeadMatrix(headMatrix);
        return true;
    }
    case '.':
    {
        eq::Matrix4f headMatrix = _getHeadMatrix();
        headMatrix.pre_rotate_x(.1f);
        _setHeadMatrix(headMatrix);
        return true;
    }
    case ',':
    {
        eq::Matrix4f headMatrix = _getHeadMatrix();
        headMatrix.pre_rotate_x(-.1f);
        _setHeadMatrix(headMatrix);
        return true;
    }
    case ';':
    {
        eq::Matrix4f headMatrix = _getHeadMatrix();
        headMatrix.pre_rotate_y(.1f);
        _setHeadMatrix(headMatrix);
        return true;
    }
    case '\'':
    {
        eq::Matrix4f headMatrix = _getHeadMatrix();
        headMatrix.pre_rotate_y(-.1f);
        _setHeadMatrix(headMatrix);
        return true;
    }
    case '[':
    {
        eq::Matrix4f headMatrix = _getHeadMatrix();
        headMatrix.pre_rotate_z(-.1f);
        _setHeadMatrix(headMatrix);
        return true;
    }
    case ']':
    {
        eq::Matrix4f headMatrix = _getHeadMatrix();
        headMatrix.pre_rotate_z(.1f);
        _setHeadMatrix(headMatrix);
        return true;
    }

    case '+':
        _changeFocusDistance(.1f);
        return true;

    case '-':
        _changeFocusDistance(-.1f);
        return true;

    case '1':
        _setFocusMode(eq::FOCUSMODE_FIXED);
        return true;

    case '2':
        _setFocusMode(eq::FOCUSMODE_RELATIVE_TO_ORIGIN);
        return true;

    case '3':
        _setFocusMode(eq::FOCUSMODE_RELATIVE_TO_OBSERVER);
        return true;

    case '4':
        _adjustResistance(1);
        return true;

    case '5':
        _adjustResistance(-1);
        return true;

    case 'j':
        stopFrames();
        return true;

    case 'e':
        _toggleEqualizer();
        return true;

    default:
        return eq::Config::handleEvent(type, event);
    }
}

bool Config::handleEvent(const eq::EventType type,
                         const eq::PointerEvent& event)
{
    switch (type)
    {
    case eq::EVENT_CHANNEL_POINTER_BUTTON_PRESS:
    {
        const eq::uint128_t& viewID = event.context.view.identifier;
        _frameData.setCurrentViewID(viewID);
        if (viewID == 0)
        {
            _currentCanvas = 0;
            return false;
        }

        const View* view = _getCurrentView();
        const eq::Layout* layout = view->getLayout();
        const eq::Canvases& canvases = getCanvases();
        for (eq::Canvas* canvas : canvases)
        {
            const eq::Layout* canvasLayout = canvas->getActiveLayout();

            if (canvasLayout == layout)
            {
                _currentCanvas = canvas;
                return true;
            }
        }
        return true;
    }

    case eq::EVENT_CHANNEL_POINTER_BUTTON_RELEASE:
        if (event.buttons == eq::PTR_BUTTON_NONE)
        {
            if (event.button == eq::PTR_BUTTON1)
            {
                _spinX = event.dy;
                _spinY = event.dx;
                return true;
            }
            if (event.button == eq::PTR_BUTTON2)
            {
                _advance = -event.dy;
                return true;
            }
        }
        break;

    case eq::EVENT_CHANNEL_POINTER_MOTION:
        switch (event.buttons)
        {
        case eq::PTR_BUTTON1:
            _spinX = 0;
            _spinY = 0;

            if ((event.modifiers & KeyModifier::shift) == KeyModifier::shift)
                _frameData.moveCamera(0.f, 0.f, -.005f * event.dy);
            else if (_frameData.usePilotMode())
                _frameData.spinCamera(-0.005f * event.dy, -0.005f * event.dx);
            else
                _frameData.spinModel(-0.005f * event.dy, -0.005f * event.dx,
                                     0.f);
            return true;

        case eq::PTR_BUTTON2:
            _advance = -event.dy;
            _frameData.moveCamera(0.f, 0.f, .005f * _advance);
            return true;

        case eq::PTR_BUTTON3:
            _frameData.moveCamera(.0005f * event.dx, -.0005f * event.dy, 0.f);
            return true;
        }
        break;

    case eq::EVENT_CHANNEL_POINTER_WHEEL:
        _frameData.moveCamera(-0.05f * event.xAxis, 0.f, 0.05f * event.yAxis);
        return true;

    default:
        break;
    }

    return eq::Config::handleEvent(type, event);
}

bool Config::handleEvent(const eq::AxisEvent& event)
{
    _spinX = 0;
    _spinY = 0;
    _advance = 0;
    _frameData.spinModel(0.0001f * event.xRotation, 0.0001f * event.yRotation,
                         0.0001f * event.zRotation);
    _frameData.moveCamera(0.0001f * event.xAxis, 0.0001f * event.yAxis,
                          0.0001f * event.zAxis);
    return true;
}

bool Config::handleEvent(const eq::ButtonEvent& event)
{
    if (event.button == eq::PTR_BUTTON1)
    {
        _frameData.toggleColorMode();
        return true;
    }
    return eq::Config::handleEvent(event);
}

bool Config::handleEvent(const eq::EventType type, const eq::Event& event)
{
    switch (type)
    {
    case eq::EVENT_WINDOW_EXPOSE:
    case eq::EVENT_WINDOW_RESIZE:
    case eq::EVENT_WINDOW_CLOSE:
    case eq::EVENT_VIEW_RESIZE:
        return true;

    default:
        return eq::Config::handleEvent(type, event);
    }
}

bool Config::handleEvent(eq::EventICommand command)
{
    switch (command.getEventType())
    {
    case IDLE_AA_LEFT:
        if (_useIdleAA)
        {
            const int32_t steps = command.read<int32_t>();
            _numFramesAA = LB_MAX(_numFramesAA, steps);
        }
        else
            _numFramesAA = 0;
        return _numFramesAA > 0;

    default:
        break;
    }

    _redraw |= eq::Config::handleEvent(command);
    return _redraw;
}

co::uint128_t Config::sync(const co::uint128_t& version)
{
    if (_admin.isValid() && _admin->isConnected())
        _admin->syncConfig(getID(), version);

    return eq::Config::sync(version);
}

void Config::_switchCanvas()
{
    const eq::Canvases& canvases = getCanvases();
    if (canvases.empty())
        return;

    _frameData.setCurrentViewID(eq::uint128_t());

    if (!_currentCanvas)
    {
        _currentCanvas = canvases.front();
        return;
    }

    eq::CanvasesCIter i = lunchbox::find(canvases, _currentCanvas);
    LBASSERT(i != canvases.end());

    ++i;
    if (i == canvases.end())
        _currentCanvas = canvases.front();
    else
        _currentCanvas = *i;
    _switchView(); // activate first view on canvas
}

void Config::_switchView()
{
    const eq::Canvases& canvases = getCanvases();
    if (!_currentCanvas && !canvases.empty())
        _currentCanvas = canvases.front();

    if (!_currentCanvas)
        return;

    const eq::Layout* layout = _currentCanvas->getActiveLayout();
    if (!layout)
        return;

    const View* view = _getCurrentView();
    const eq::Views& views = layout->getViews();
    LBASSERT(!views.empty());

    if (!view)
    {
        _frameData.setCurrentViewID(views.front()->getID());
        return;
    }

    eq::ViewsCIter i = std::find(views.begin(), views.end(), view);
    if (i != views.end())
        ++i;
    if (i == views.end())
        _frameData.setCurrentViewID(eq::uint128_t());
    else
        _frameData.setCurrentViewID((*i)->getID());
}

void Config::_switchModel()
{
    if (_modelDist.empty()) // no models
        return;

    // current model of current view
    View* view = _getCurrentView();
    const eq::uint128_t& currentID =
        view ? view->getModelID() : _frameData.getModelID();
    // next model
    ModelDistsCIter i;
    for (i = _modelDist.begin(); i != _modelDist.end(); ++i)
    {
        if ((*i)->getID() != currentID)
            continue;

        ++i;
        break;
    }
    if (i == _modelDist.end())
        i = _modelDist.begin(); // wrap around

    // set identifier on view or frame data (default model)
    const eq::uint128_t& modelID = (*i)->getID();
    if (view)
        view->setModelID(modelID);
    else
        _frameData.setModelID(modelID);

    if (view)
    {
        const Model* model = getModel(modelID);
        _setMessage("Using " + lunchbox::getFilename(model->getName()));
    }
}

void Config::_switchViewMode()
{
    View* view = _getCurrentView();
    if (!view)
        return;

    const eq::View::Mode mode = view->getMode();
    if (mode == eq::View::MODE_MONO)
    {
        view->changeMode(eq::View::MODE_STEREO);
        _setMessage("Switched to stereoscopic rendering");
    }
    else
    {
        view->changeMode(eq::View::MODE_MONO);
        _setMessage("Switched to monoscopic rendering");
    }
}

void Config::_freezeLoadBalancing(const bool onOff)
{
    View* view = _getCurrentView();
    if (view)
        view->getEqualizer().setFrozen(onOff);
}

void Config::_adjustEyeBase(const float delta)
{
    const eq::Observers& observers = getObservers();
    for (eq::ObserversCIter i = observers.begin(); i != observers.end(); ++i)
    {
        eq::Observer* observer = *i;
        observer->setEyePosition(eq::EYE_LEFT,
                                 observer->getEyePosition(eq::EYE_LEFT) +
                                     eq::Vector3f(-delta, 0.f, 0.f));
        observer->setEyePosition(eq::EYE_RIGHT,
                                 observer->getEyePosition(eq::EYE_RIGHT) +
                                     eq::Vector3f(delta, 0.f, 0.f));
        std::ostringstream os;
        os << "Set eyes to " << observer->getEyePosition(eq::EYE_LEFT) << ", "
           << observer->getEyePosition(eq::EYE_RIGHT);
        _setMessage(os.str());
    }
}

void Config::_adjustTileSize(const int delta)
{
    View* view = _getCurrentView();
    if (!view)
        return;

    eq::Vector2i tileSize = view->getEqualizer().getTileSize();
    if (tileSize == eq::Vector2i(-1, -1))
        tileSize = eq::Vector2i(64, 64);
    tileSize += delta;
    view->getEqualizer().setTileSize(tileSize);
}

void Config::_adjustResistance(const int delta)
{
    View* view = _getCurrentView();
    if (!view)
        return;

    eq::Vector2i size = view->getEqualizer().getResistance2i();
    size += delta;
    size.x() = LB_MAX(size.x(), 0);
    size.y() = LB_MAX(size.y(), 0);
    std::ostringstream stream;
    stream << "Set load equalizer resistance to " << size;
    _setMessage(stream.str());
    view->getEqualizer().setResistance(size);
}

void Config::_adjustModelScale(const float factor)
{
    View* view = _getCurrentView();
    if (!view)
        return;

    const float current = view->getModelUnit() * factor;
    if (current > std::numeric_limits<float>::epsilon())
        view->setModelUnit(current);

    std::ostringstream stream;
    stream << "Set model unit to " << view->getModelUnit();
    _setMessage(stream.str());
}

void Config::_screenshot()
{
    View* view = _getCurrentView();
    if (!view)
        return;

    view->enableScreenshot(
        eq::Frame::Buffer::color,
        [&](const uint32_t frameNumber, const eq::Image& image) {
            const std::string screenshotPath =
                "./eqPly_" + std::to_string(frameNumber) + ".png";
            image.writeImage(screenshotPath, eq::Frame::Buffer::color);
            std::ostringstream os;
            os << "Screenshot written to " << screenshotPath;
            _setMessage(os.str());
        });
}

void Config::_switchLayout(const int32_t increment)
{
    if (!_currentCanvas)
        return;

    int64_t index = _currentCanvas->getActiveLayoutIndex() + increment;
    const eq::Layouts& layouts = _currentCanvas->getLayouts();
    LBASSERT(!layouts.empty());

    index = (index % layouts.size());
    _currentCanvas->useLayout(uint32_t(index));

    const eq::Layout* layout = layouts[index];
    if (layout)
    {
        const std::string& name = layout->getName();
        _setMessage(name);
        const auto view = layout->getViews().front();
        _frameData.setCurrentViewID(view ? view->getID() : eq::uint128_t());
    }
}

void Config::_switchLayoutSize()
{
    if (!_currentCanvas)
        return;

    eq::Layout* layout = _currentCanvas->getActiveLayout();
    if (!layout)
        return;

    const eq::PixelViewport small(0, 0, 800, 600);
    const eq::PixelViewport big(0, 0, 1280, 1024);

    if (layout->getPixelViewport() != small)
        layout->setPixelViewport(small);
    else
        layout->setPixelViewport(big);
}

void Config::_toggleEqualizer()
{
    View* view = _getCurrentView();
    if (view)
        view->toggleEqualizer();
}

// Note: real applications would use one tracking device per observer
void Config::_setHeadMatrix(const eq::Matrix4f& matrix)
{
    const eq::Observers& observers = getObservers();
    for (eq::ObserversCIter i = observers.begin(); i != observers.end(); ++i)
    {
        (*i)->setHeadMatrix(matrix);
    }

    std::ostringstream stream;
    stream << "Observer at " << matrix.getTranslation();
    _setMessage(stream.str());
}

const eq::Matrix4f& Config::_getHeadMatrix() const
{
    const eq::Observers& observers = getObservers();
    static const eq::Matrix4f identity;
    if (observers.empty())
        return identity;

    return observers[0]->getHeadMatrix();
}

void Config::_changeFocusDistance(const float delta)
{
    const eq::Observers& observers = getObservers();
    for (eq::ObserversCIter i = observers.begin(); i != observers.end(); ++i)
    {
        eq::Observer* observer = *i;
        observer->setFocusDistance(observer->getFocusDistance() + delta);
        std::ostringstream stream;
        stream << "Set focus distance to " << observer->getFocusDistance();
        _setMessage(stream.str());
    }
}

void Config::_setFocusMode(const eq::FocusMode mode)
{
    const eq::Observers& observers = getObservers();
    for (eq::ObserversCIter i = observers.begin(); i != observers.end(); ++i)
        (*i)->setFocusMode(mode);

    std::ostringstream stream;
    stream << "Set focus mode to " << mode;
    _setMessage(stream.str());
}

void Config::_setMessage(const std::string& message)
{
    _frameData.setMessage(message);
}

eq::admin::ServerPtr Config::_getAdminServer()
{
    // Debug: _closeAdminServer();
    if (_admin.isValid() && _admin->isConnected())
        return _admin;

    eq::admin::init(0, 0);
    eq::admin::ClientPtr client = new eq::admin::Client;
    if (!client->initLocal(0, 0))
    {
        _setMessage("Can't init admin client");
        eq::admin::exit();
    }

    _admin = new eq::admin::Server;
    if (!client->connectServer(_admin))
    {
        _setMessage("Can't open connection to administrate server");
        client->exitLocal();
        _admin = 0;
        eq::admin::exit();
    }
    return _admin;
}

void Config::_closeAdminServer()
{
    if (!_admin)
        return;

    eq::admin::ClientPtr client = _admin->getClient();
    client->disconnectServer(_admin);
    client->exitLocal();
    LBASSERT(client->getRefCount() == 1);
    LBASSERT(_admin->getRefCount() == 1);

    _admin = 0;
    eq::admin::exit();
}

View* Config::_getCurrentView()
{
    const eq::uint128_t& viewID = _frameData.getCurrentViewID();
    eq::View* view = find<eq::View>(viewID);
    return static_cast<View*>(view);
}

const View* Config::_getCurrentView() const
{
    const eq::uint128_t& viewID = _frameData.getCurrentViewID();
    const eq::View* view = find<eq::View>(viewID);
    return static_cast<const View*>(view);
}
}

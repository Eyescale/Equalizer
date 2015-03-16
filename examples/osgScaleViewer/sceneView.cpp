
/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *                           2010-2014 Stefan Eilemann <eile@eyescale.ch>
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/
#include "sceneView.h"

#include <osgUtil/UpdateVisitor>
#include <osgUtil/GLObjectsVisitor>

#include <osg/Version>
#include <osg/Timer>
#include <osg/GLExtensions>
#include <osg/GLObjects>
#include <osg/Notify>
#include <osg/Texture>
#include <osg/AlphaFunc>
#include <osg/TexEnv>
#include <osg/ColorMatrix>
#include <osg/LightModel>
#include <osg/CollectOccludersVisitor>

#include <osg/GLU>

#include <iterator>

using namespace osg;
using namespace osgUtil;

namespace osgScaleViewer
{

SceneView::SceneView( osg::DisplaySettings* ds )
    : _fusionDistanceValue( 0.f )
{
    _displaySettings = ds;

    _lightingMode=NO_SCENEVIEW_LIGHT;

    _prioritizeTextures = false;

    setCamera(new Camera);
    _camera->setViewport(new Viewport);
    _camera->setClearColor(osg::Vec4(0.2f, 0.2f, 0.4f, 1.0f));

    _initCalled = false;

    _camera->setDrawBuffer(GL_BACK);

    _requiresFlush = true;

    _activeUniforms = DEFAULT_UNIFORMS;

    _previousFrameTime = 0;
    _previousSimulationTime = 0;

    _dynamicObjectCount = 0;
}

SceneView::SceneView(const SceneView& rhs, const osg::CopyOp& copyop)
    : osg::Object(rhs,copyop)
    , osg::CullSettings(rhs)
    , _fusionDistanceValue( 0.f )
{
    _displaySettings = rhs._displaySettings;

    _lightingMode = rhs._lightingMode;

    _prioritizeTextures = rhs._prioritizeTextures;

    _camera = rhs._camera;
    _cameraWithOwnership = rhs._cameraWithOwnership;

    _initCalled = false;

    _requiresFlush = rhs._requiresFlush;

    _activeUniforms = rhs._activeUniforms;

    _previousFrameTime = 0;
    _previousSimulationTime = 0;

    _dynamicObjectCount = 0;
}

SceneView::~SceneView()
{
}


void SceneView::setDefaults(unsigned int options)
{
    osg::CullSettings::setDefaults();

    _camera->getProjectionMatrix().makePerspective(50.0f,1.4f,1.0f,10000.0f);
    _camera->getViewMatrix().makeIdentity();

    if (!_globalStateSet) _globalStateSet = new osg::StateSet;
    else _globalStateSet->clear();

    if ((options & HEADLIGHT) || (options & SKY_LIGHT))
    {
        #if defined(OSG_GL_FIXED_FUNCTION_AVAILABLE)
            _lightingMode=(options&HEADLIGHT) ? HEADLIGHT : SKY_LIGHT;
            _light = new osg::Light;
            _light->setLightNum(0);
            _light->setAmbient(Vec4(0.00f,0.0f,0.00f,1.0f));
            _light->setDiffuse(Vec4(0.8f,0.8f,0.8f,1.0f));
            _light->setSpecular(Vec4(1.0f,1.0f,1.0f,1.0f));


            _globalStateSet->setAssociatedModes(_light.get(),osg::StateAttribute::ON);

            // enable lighting by default.
            _globalStateSet->setMode(GL_LIGHTING, osg::StateAttribute::ON);
        #endif

        #if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE) && defined(OSG_GL_FIXED_FUNCTION_AVAILABLE)
            osg::LightModel* lightmodel = new osg::LightModel;
            lightmodel->setAmbientIntensity(osg::Vec4(0.1f,0.1f,0.1f,1.0f));
            _globalStateSet->setAttributeAndModes(lightmodel, osg::StateAttribute::ON);
        #endif
    }
    else
    {
        _lightingMode = NO_SCENEVIEW_LIGHT;
    }

    _renderInfo.setState(new State);

    _stateGraph = new StateGraph;
    _renderStage = new RenderStage;


    if (options & COMPILE_GLOBJECTS_AT_INIT)
    {
#ifndef __sgi
        GLObjectsVisitor::Mode dlvMode = GLObjectsVisitor::COMPILE_DISPLAY_LISTS |
                                          GLObjectsVisitor::COMPILE_STATE_ATTRIBUTES |
                                          GLObjectsVisitor::CHECK_BLACK_LISTED_MODES;

#else
        // sgi's IR graphics has a problem with lighting and display lists, as it seems to store
        // lighting state with the display list, and the display list visitor doesn't currently apply
        // state before creating display lists. So will disable the init visitor default, this won't
        // affect functionality since the display lists will be created as and when needed.
        GLObjectsVisitor::Mode dlvMode = GLObjectsVisitor::COMPILE_STATE_ATTRIBUTES;
#endif

        GLObjectsVisitor* dlv = new GLObjectsVisitor(dlvMode);
        dlv->setNodeMaskOverride(0xffffffff);
        _initVisitor = dlv;

    }

    _updateVisitor = new UpdateVisitor;

    _cullVisitor = CullVisitor::create();

    _cullVisitor->setStateGraph(_stateGraph.get());
    _cullVisitor->setRenderStage(_renderStage.get());

    _globalStateSet->setGlobalDefaults();

    #if defined(OSG_GL_FIXED_FUNCTION_AVAILABLE)
        // set up an texture environment by default to speed up blending operations.
         osg::TexEnv* texenv = new osg::TexEnv;
         texenv->setMode(osg::TexEnv::MODULATE);
         _globalStateSet->setTextureAttributeAndModes(0,texenv, osg::StateAttribute::ON);
    #endif

    _camera->setClearColor(osg::Vec4(0.2f, 0.2f, 0.4f, 1.0f));
}

void SceneView::setCamera(osg::Camera* camera, bool assumeOwnershipOfCamera)
{
    if (camera)
    {
        _camera = camera;
    }
    else
    {
        osg::notify(osg::NOTICE)<<"Warning: attempt to assign a NULL camera to SceneView not permitted."<<std::endl;
    }

    if (assumeOwnershipOfCamera)
    {
        _cameraWithOwnership = _camera.get();
    }
    else
    {
        _cameraWithOwnership = 0;
    }
}

void SceneView::setSceneData(osg::Node* node)
{
    // take a temporary reference to node to prevent the possibility
    // of it getting deleted when when we do the camera clear of children.
    osg::ref_ptr<osg::Node> temporaryRefernce = node;

    // remove pre existing children
    _camera->removeChildren(0, _camera->getNumChildren());

    // add the new one in.
    _camera->addChild(node);
}

void SceneView::init()
{
    _initCalled = true;

    // force the initialization of the OpenGL extension string
    // to try and work around a Windows NVidia driver bug circa Oct 2006.
    osg::isGLExtensionSupported(_renderInfo.getState()->getContextID(),"");

    if (_camera.valid() && _initVisitor.valid())
    {
        _initVisitor->reset();
        _initVisitor->setFrameStamp(_frameStamp.get());

        GLObjectsVisitor* dlv = dynamic_cast<GLObjectsVisitor*>(_initVisitor.get());
        if (dlv) dlv->setState(_renderInfo.getState());

        if (_frameStamp.valid())
        {
             _initVisitor->setTraversalNumber(_frameStamp->getFrameNumber());
        }

        _camera->accept(*_initVisitor.get());

    }
}

void SceneView::updateUniforms()
{
    if (!_localStateSet)
    {
        _localStateSet = new osg::StateSet;
    }

    if (!_localStateSet) return;

    if ((_activeUniforms & FRAME_NUMBER_UNIFORM) && _frameStamp.valid())
    {
#if OSG_MIN_VERSION_REQUIRED(2,9,11)
        osg::Uniform* uniform = _localStateSet->getOrCreateUniform("osg_FrameNumber",osg::Uniform::UNSIGNED_INT);
#else
        osg::Uniform* uniform = _localStateSet->getOrCreateUniform("osg_FrameNumber",osg::Uniform::INT);
#endif

        uniform->set(_frameStamp->getFrameNumber());
    }

    if ((_activeUniforms & FRAME_TIME_UNIFORM) && _frameStamp.valid())
    {
        osg::Uniform* uniform = _localStateSet->getOrCreateUniform("osg_FrameTime",osg::Uniform::FLOAT);
        uniform->set(static_cast<float>(_frameStamp->getReferenceTime()));
    }

    if ((_activeUniforms & DELTA_FRAME_TIME_UNIFORM) && _frameStamp.valid())
    {
        float delta_frame_time = (_previousFrameTime != 0.0) ? static_cast<float>(_frameStamp->getReferenceTime()-_previousFrameTime) : 0.0f;
        _previousFrameTime = _frameStamp->getReferenceTime();

        osg::Uniform* uniform = _localStateSet->getOrCreateUniform("osg_DeltaFrameTime",osg::Uniform::FLOAT);
        uniform->set(delta_frame_time);
    }

    if ((_activeUniforms & SIMULATION_TIME_UNIFORM) && _frameStamp.valid())
    {
        osg::Uniform* uniform = _localStateSet->getOrCreateUniform("osg_SimulationTime",osg::Uniform::FLOAT);
        uniform->set(static_cast<float>(_frameStamp->getSimulationTime()));
    }

    if ((_activeUniforms & DELTA_SIMULATION_TIME_UNIFORM) && _frameStamp.valid())
    {
        float delta_simulation_time = (_previousSimulationTime != 0.0) ? static_cast<float>(_frameStamp->getSimulationTime()-_previousSimulationTime) : 0.0f;
        _previousSimulationTime = _frameStamp->getSimulationTime();

        osg::Uniform* uniform = _localStateSet->getOrCreateUniform("osg_DeltaSimulationTime",osg::Uniform::FLOAT);
        uniform->set(delta_simulation_time);
    }

    if (_activeUniforms & VIEW_MATRIX_UNIFORM)
    {
        osg::Uniform* uniform = _localStateSet->getOrCreateUniform("osg_ViewMatrix",osg::Uniform::FLOAT_MAT4);
        uniform->set(getViewMatrix());
    }

    if (_activeUniforms & VIEW_MATRIX_INVERSE_UNIFORM)
    {
        osg::Uniform* uniform = _localStateSet->getOrCreateUniform("osg_ViewMatrixInverse",osg::Uniform::FLOAT_MAT4);
        uniform->set(osg::Matrix::inverse(getViewMatrix()));
    }

}

void SceneView::setLightingMode(LightingMode mode)
{
    if (mode==_lightingMode) return;

    if (_lightingMode!=NO_SCENEVIEW_LIGHT)
    {
        // remove GL_LIGHTING mode
        _globalStateSet->removeMode(GL_LIGHTING);

        if (_light.valid())
        {
            _globalStateSet->removeAssociatedModes(_light.get());
        }

    }

    _lightingMode = mode;

    if (_lightingMode!=NO_SCENEVIEW_LIGHT)
    {
        #if defined(OSG_GL_FIXED_FUNCTION_AVAILABLE)
            // add GL_LIGHTING mode
            _globalStateSet->setMode(GL_LIGHTING, osg::StateAttribute::ON);
            if (_light.valid())
            {
                _globalStateSet->setAssociatedModes(_light.get(), osg::StateAttribute::ON);
            }
        #endif
    }
}

void SceneView::inheritCullSettings(const osg::CullSettings& settings, unsigned int inheritanceMask)
{
    if (_camera.valid() && _camera->getView())
    {
        if (inheritanceMask & osg::CullSettings::LIGHTING_MODE)
        {
            LightingMode newLightingMode = _lightingMode;

            switch(_camera->getView()->getLightingMode())
            {
                case(osg::View::NO_LIGHT): newLightingMode = NO_SCENEVIEW_LIGHT; break;
                case(osg::View::HEADLIGHT): newLightingMode = HEADLIGHT; break;
                case(osg::View::SKY_LIGHT): newLightingMode = SKY_LIGHT; break;
            }

            if (newLightingMode != _lightingMode)
            {
                setLightingMode(newLightingMode);
            }
        }

        if (inheritanceMask & osg::CullSettings::LIGHT)
        {
            setLight(_camera->getView()->getLight());
        }
    }

    osg::CullSettings::inheritCullSettings(settings, inheritanceMask);
}


void SceneView::cull()
{
    _dynamicObjectCount = 0;

    if (_camera->getNodeMask()==0) return;

    _renderInfo.setView(_camera->getView());

    // update the active uniforms
    updateUniforms();

    if (!_renderInfo.getState())
    {
        osg::notify(osg::INFO) << "Warning: no valid osgUtil::SceneView::_state attached, creating a default state automatically."<< std::endl;

        // note the constructor for osg::State will set ContextID to 0 which will be fine to single context graphics
        // applications which is ok for most apps, but not multiple context/pipe applications.
        _renderInfo.setState(new osg::State);
    }

    osg::State* state = _renderInfo.getState();

    if (!_localStateSet)
    {
        _localStateSet = new osg::StateSet;
    }

    // we in theory should be able to be able to bypass reset, but we'll call it just incase.
    //_state->reset();

    state->setFrameStamp(_frameStamp.get());
    state->setDisplaySettings(_displaySettings.get());


    if (!_cullVisitor)
    {
        osg::notify(osg::INFO) << "Warning: no valid osgUtil::SceneView:: attached, creating a default CullVisitor automatically."<< std::endl;
        _cullVisitor = CullVisitor::create();
    }
    if (!_stateGraph)
    {
        osg::notify(osg::INFO) << "Warning: no valid osgUtil::SceneView:: attached, creating a global default StateGraph automatically."<< std::endl;
        _stateGraph = new StateGraph;
    }
    if (!_renderStage)
    {
        osg::notify(osg::INFO) << "Warning: no valid osgUtil::SceneView::_renderStage attached, creating a default RenderStage automatically."<< std::endl;
        _renderStage = new RenderStage;
    }

    _cullVisitor->setTraversalMask(_cullMask);
    bool computeNearFar = cullStage(getProjectionMatrix(),getViewMatrix(),_cullVisitor.get(),_stateGraph.get(),_renderStage.get(),getViewport());

    if (computeNearFar)
    {
        CullVisitor::value_type zNear = _cullVisitor->getCalculatedNearPlane();
        CullVisitor::value_type zFar = _cullVisitor->getCalculatedFarPlane();
        _cullVisitor->clampProjectionMatrix(getProjectionMatrix(),zNear,zFar);
    }
}

bool SceneView::cullStage(const osg::Matrixd& projection,const osg::Matrixd& modelview,osgUtil::CullVisitor* cullVisitor, osgUtil::StateGraph* rendergraph, osgUtil::RenderStage* renderStage, osg::Viewport *viewport)
{

    if (!_camera || !viewport) return false;

    osg::ref_ptr<RefMatrix> proj = new osg::RefMatrix(projection);
    osg::ref_ptr<RefMatrix> mv = new osg::RefMatrix(modelview);

    // collect any occluder in the view frustum.
    if (_camera->containsOccluderNodes())
    {
        //std::cout << "Scene graph contains occluder nodes, searching for them"<<std::endl;


        if (!_collectOccludersVisitor) _collectOccludersVisitor = new osg::CollectOccludersVisitor;

        _collectOccludersVisitor->inheritCullSettings(*this);

        _collectOccludersVisitor->reset();

        _collectOccludersVisitor->setFrameStamp(_frameStamp.get());

        // use the frame number for the traversal number.
        if (_frameStamp.valid())
        {
             _collectOccludersVisitor->setTraversalNumber(_frameStamp->getFrameNumber());
        }

        _collectOccludersVisitor->pushViewport(viewport);
        _collectOccludersVisitor->pushProjectionMatrix(proj.get());
        _collectOccludersVisitor->pushModelViewMatrix(mv.get(),osg::Transform::ABSOLUTE_RF);

        // traverse the scene graph to search for occluder in there new positions.
        _collectOccludersVisitor->traverse(*_camera);

        _collectOccludersVisitor->popModelViewMatrix();
        _collectOccludersVisitor->popProjectionMatrix();
        _collectOccludersVisitor->popViewport();

        // sort the occluder from largest occluder volume to smallest.
        _collectOccludersVisitor->removeOccludedOccluders();


        osg::notify(osg::DEBUG_INFO) << "finished searching for occluder - found "<<_collectOccludersVisitor->getCollectedOccluderSet().size()<<std::endl;

        cullVisitor->getOccluderList().clear();
        std::copy(_collectOccludersVisitor->getCollectedOccluderSet().begin(),_collectOccludersVisitor->getCollectedOccluderSet().end(), std::back_insert_iterator<CullStack::OccluderList>(cullVisitor->getOccluderList()));
    }



    cullVisitor->reset();

    cullVisitor->setFrameStamp(_frameStamp.get());

    // use the frame number for the traversal number.
    if (_frameStamp.valid())
    {
         cullVisitor->setTraversalNumber(_frameStamp->getFrameNumber());
    }

    cullVisitor->inheritCullSettings(*this);

    cullVisitor->setStateGraph(rendergraph);
    cullVisitor->setRenderStage(renderStage);

    cullVisitor->setRenderInfo( _renderInfo );

    renderStage->reset();

    // comment out reset of rendergraph since clean is more efficient.
    //  rendergraph->reset();

    // use clean of the rendergraph rather than reset, as it is able to
    // reuse the structure on the rendergraph in the next frame. This
    // achieves a certain amount of frame cohereancy of memory allocation.
    rendergraph->clean();

    renderStage->setViewport(viewport);
    renderStage->setClearColor(_camera->getClearColor());
    renderStage->setClearDepth(_camera->getClearDepth());
    renderStage->setClearAccum(_camera->getClearAccum());
    renderStage->setClearStencil(_camera->getClearStencil());
    renderStage->setClearMask(_camera->getClearMask());

#if 1
    renderStage->setCamera(_camera.get());
#endif

    #if defined(OSG_GL_FIXED_FUNCTION_AVAILABLE)
        switch(_lightingMode)
        {
        case(HEADLIGHT):
            if (_light.valid()) renderStage->addPositionedAttribute(NULL,_light.get());
            else osg::notify(osg::WARN)<<"Warning: no osg::Light attached to ogUtil::SceneView to provide head light.*/"<<std::endl;
            break;
        case(SKY_LIGHT):
            if (_light.valid()) renderStage->addPositionedAttribute(mv.get(),_light.get());
            else osg::notify(osg::WARN)<<"Warning: no osg::Light attached to ogUtil::SceneView to provide sky light.*/"<<std::endl;
            break;
        default:
            break;
        }
    #endif

    if (_globalStateSet.valid()) cullVisitor->pushStateSet(_globalStateSet.get());
    if (_secondaryStateSet.valid()) cullVisitor->pushStateSet(_secondaryStateSet.get());
    if (_localStateSet.valid()) cullVisitor->pushStateSet(_localStateSet.get());


    cullVisitor->pushViewport(viewport);
    cullVisitor->pushProjectionMatrix(proj.get());
    cullVisitor->pushModelViewMatrix(mv.get(),osg::Transform::ABSOLUTE_RF);

    // traverse the scene graph to generate the rendergraph.
    // If the camera has a cullCallback execute the callback which has the
    // requirement that it must traverse the camera's children.
    {
        osg::NodeCallback* callback = dynamic_cast< osg::NodeCallback* >(
            _camera->getCullCallback( ));
       if (callback) (*callback)(_camera.get(), cullVisitor);
       else cullVisitor->traverse(*_camera);
    }


    cullVisitor->popModelViewMatrix();
    cullVisitor->popProjectionMatrix();
    cullVisitor->popViewport();

    if (_localStateSet.valid()) cullVisitor->popStateSet();
    if (_secondaryStateSet.valid()) cullVisitor->popStateSet();
    if (_globalStateSet.valid()) cullVisitor->popStateSet();


    renderStage->sort();

    // prune out any empty StateGraph children.
    // note, this would be not required if the rendergraph had been
    // reset at the start of each frame (see top of this method) but
    // a clean has been used instead to try to minimize the amount of
    // allocation and deleteing of the StateGraph nodes.
    rendergraph->prune();

    // set the number of dynamic objects in the scene.
    _dynamicObjectCount += renderStage->computeNumberOfDynamicRenderLeaves();


    bool computeNearFar = (cullVisitor->getComputeNearFarMode()!=osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR) && getSceneData()!=0;
    return computeNearFar;
}

void SceneView::releaseAllGLObjects()
{
    if (!_camera) return;

    _camera->releaseGLObjects(_renderInfo.getState());

    // we need to reset State as it keeps handles to Program objects.
    if (_renderInfo.getState()) _renderInfo.getState()->reset();
}


void SceneView::flushAllDeletedGLObjects()
{
    _requiresFlush = false;

    osg::flushAllDeletedGLObjects(getState()->getContextID());
 }

void SceneView::flushDeletedGLObjects(double& availableTime)
{
    osg::State* state = _renderInfo.getState();

    _requiresFlush = false;

    double currentTime = state->getFrameStamp()?state->getFrameStamp()->getReferenceTime():0.0;

    osg::flushDeletedGLObjects(getState()->getContextID(), currentTime, availableTime);
}

void SceneView::draw()
{
    if (_camera->getNodeMask()==0) return;

    osg::State* state = _renderInfo.getState();
    state->initializeExtensionProcs();

    osg::Texture::TextureObjectManager* tom = osg::Texture::getTextureObjectManager(state->getContextID()).get();
    tom->newFrame(state->getFrameStamp());

    osg::GLBufferObjectManager* bom = osg::GLBufferObjectManager::getGLBufferObjectManager(state->getContextID()).get();
    bom->newFrame(state->getFrameStamp());

    if (!_initCalled) init();

    // note, to support multi-pipe systems the deletion of OpenGL display list
    // and texture objects is deferred until the OpenGL context is the correct
    // context for when the object were originally created.  Here we know what
    // context we are in so can flush the appropriate caches.

    if (_requiresFlush)
    {
        double availableTime = 0.005;
        flushDeletedGLObjects(availableTime);
    }

    // assume the the draw which is about to happen could generate GL objects that need flushing in the next frame.
    _requiresFlush = true;

    state->setInitialViewMatrix(new osg::RefMatrix(getViewMatrix()));

    RenderLeaf* previous = NULL;

    _localStateSet->setAttribute(getViewport());

    // bog standard draw.
    _renderStage->drawPreRenderStages(_renderInfo,previous);
    _renderStage->draw(_renderInfo,previous);

    // re apply the defalt OGL state.
    state->popAllStateSets();
    state->apply();

#if 0
    if (_camera->getPostDrawCallback())
    {
        (*(_camera->getPostDrawCallback()))(*_camera);
    }
#endif

    if (state->getCheckForGLErrors()!=osg::State::NEVER_CHECK_GL_ERRORS)
    {
        if (state->checkGLErrors("end of SceneView::draw()"))
        {
            // go into debug mode of OGL error in a fine grained way to help
            // track down OpenGL errors.
            state->setCheckForGLErrors(osg::State::ONCE_PER_ATTRIBUTE);
        }
    }

// #define REPORT_TEXTURE_MANAGER_STATS
#ifdef REPORT_TEXTURE_MANAGER_STATS
    tom->reportStats();
    bom->reportStats();
#endif

    // osg::notify(osg::NOTICE)<<"SceneView  draw() DynamicObjectCount"<<getState()->getDynamicObjectCount()<<std::endl;

}

/** Calculate, via glUnProject, the object coordinates of a window point.
    Note, current implementation requires that SceneView::draw() has been previously called
    for projectWindowIntoObject to produce valid values.  As per OpenGL
    windows coordinates are calculated relative to the bottom left of the window.*/
bool SceneView::projectWindowIntoObject(const osg::Vec3& window,osg::Vec3& object) const
{
    osg::Matrix inverseMVPW;
    inverseMVPW.invert(computeMVPW());

    object = window*inverseMVPW;

    return true;
}


/** Calculate, via glUnProject, the object coordinates of a window x,y
    when projected onto the near and far planes.
    Note, current implementation requires that SceneView::draw() has been previously called
    for projectWindowIntoObject to produce valid values.  As per OpenGL
    windows coordinates are calculated relative to the bottom left of the window.*/
bool SceneView::projectWindowXYIntoObject(int x,int y,osg::Vec3& near_point,osg::Vec3& far_point) const
{
    osg::Matrix inverseMVPW;
    inverseMVPW.invert(computeMVPW());

    near_point = osg::Vec3(x,y,0.0f)*inverseMVPW;
    far_point = osg::Vec3(x,y,1.0f)*inverseMVPW;

    return true;
}


/** Calculate, via glProject, the object coordinates of a window.
    Note, current implementation requires that SceneView::draw() has been previously called
    for projectWindowIntoObject to produce valid values.  As per OpenGL
    windows coordinates are calculated relative to the bottom left of the window.*/
bool SceneView::projectObjectIntoWindow(const osg::Vec3& object,osg::Vec3& window) const
{
    window = object*computeMVPW();
    return true;
}

const osg::Matrix SceneView::computeMVPW() const
{
    osg::Matrix matrix( getViewMatrix() * getProjectionMatrix());

    if (getViewport())
        matrix.postMult(getViewport()->computeWindowMatrix());
    else
        osg::notify(osg::WARN)<<"osg::Matrix SceneView::computeMVPW() - error no viewport attached to SceneView, coords will be computed inccorectly."<<std::endl;

    return matrix;
}

void SceneView::clearArea(int x,int y,int width,int height,const osg::Vec4& color)
{
    osg::ref_ptr<osg::Viewport> viewport = new osg::Viewport;
    viewport->setViewport(x,y,width,height);

    _renderInfo.getState()->applyAttribute(viewport.get());

    glScissor( x, y, width, height );
    glEnable( GL_SCISSOR_TEST );
    glClearColor( color[0], color[1], color[2], color[3]);
    glClear( GL_COLOR_BUFFER_BIT);
    glDisable( GL_SCISSOR_TEST );
}

void SceneView::setProjectionMatrixAsOrtho(double left, double right,
                                           double bottom, double top,
                                           double zNear, double zFar)
{
    setProjectionMatrix(osg::Matrixd::ortho(left, right,
                                           bottom, top,
                                           zNear, zFar));
}

void SceneView::setProjectionMatrixAsOrtho2D(double left, double right,
                                             double bottom, double top)
{
    setProjectionMatrix(osg::Matrixd::ortho2D(left, right,
                                             bottom, top));
}

void SceneView::setProjectionMatrixAsFrustum(double left, double right,
                                             double bottom, double top,
                                             double zNear, double zFar)
{
    setProjectionMatrix(osg::Matrixd::frustum(left, right,
                                             bottom, top,
                                             zNear, zFar));
}

void SceneView::setProjectionMatrixAsPerspective(double fovy,double aspectRatio,
                                                 double zNear, double zFar)
{
    setProjectionMatrix(osg::Matrixd::perspective(fovy,aspectRatio,
                                                 zNear, zFar));
}

bool SceneView::getProjectionMatrixAsOrtho(double& left, double& right,
                                           double& bottom, double& top,
                                           double& zNear, double& zFar) const
{
    return getProjectionMatrix().getOrtho(left, right,
                                       bottom, top,
                                       zNear, zFar);
}

bool SceneView::getProjectionMatrixAsFrustum(double& left, double& right,
                                             double& bottom, double& top,
                                             double& zNear, double& zFar) const
{
    return getProjectionMatrix().getFrustum(left, right,
                                         bottom, top,
                                         zNear, zFar);
}

bool SceneView::getProjectionMatrixAsPerspective(double& fovy,double& aspectRatio,
                                                 double& zNear, double& zFar) const
{
    return getProjectionMatrix().getPerspective(fovy, aspectRatio, zNear, zFar);
}

void SceneView::setViewMatrixAsLookAt(const osg::Vec3& eye,const osg::Vec3& center,const osg::Vec3& up)
{
    setViewMatrix(osg::Matrixd::lookAt(eye,center,up));
}

void SceneView::getViewMatrixAsLookAt(osg::Vec3& eye,osg::Vec3& center,osg::Vec3& up,float lookDistance) const
{
    getViewMatrix().getLookAt(eye,center,up,lookDistance);
}

bool SceneView::getStats(osgUtil::Statistics& stats)
{
    return _renderStage->getStats(stats);
}

void SceneView::collateReferencesToDependentCameras()
{
    if (_renderStage.valid()) _renderStage->collateReferencesToDependentCameras();
}

void SceneView::clearReferencesToDependentCameras()
{
    if (_renderStage.valid()) _renderStage->clearReferencesToDependentCameras();
}
}

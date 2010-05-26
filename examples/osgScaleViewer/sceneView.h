
/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
 *                           2010 Stefan Eilemann <eile@eyescale.ch>
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

#ifndef OSG_SV_SCENEVIEW
#define OSG_SV_SCENEVIEW

#include <osg/Node>
#include <osg/StateSet>
#include <osg/Light>
#include <osg/FrameStamp>
#include <osg/DisplaySettings>
#include <osg/CollectOccludersVisitor>
#include <osg/CullSettings>
#include <osg/Camera>

#include <osgUtil/CullVisitor>

namespace osgScaleViewer {

/**
 * Based on osgUtil::SceneView, but stripped done to only render and not
 * interfere with stereo setup.
 *
 * SceneView is deprecated, and is now just kept for backwards compatibility.
 * It is recommend that you use osgViewer::Viewer/Composite in combination
 * with osgViewer::GraphicsWindowEmbedded for embedded rendering support as
 * this provides a greater range of functionality and consistency of API.
 */
class SceneView : public osg::Object, public osg::CullSettings
{
    public:

        /** Construct a default scene view.*/
        SceneView(osg::DisplaySettings* ds=NULL);

        SceneView(const SceneView& sceneview, const osg::CopyOp& copyop = osg::CopyOp());

        META_Object(osgUtil, SceneView);

        enum Options
        {
            NO_SCENEVIEW_LIGHT = 0x0,
            HEADLIGHT = 0x1,
            SKY_LIGHT = 0x2,
            COMPILE_GLOBJECTS_AT_INIT = 0x4,
            STANDARD_SETTINGS = SKY_LIGHT |
                                COMPILE_GLOBJECTS_AT_INIT
        };

        /* Set defaults. */
        virtual void setDefaults() { setDefaults(STANDARD_SETTINGS); }

        /** Set scene view to use default global state, light, camera
         *  and render visitor.
         */
        virtual void setDefaults(unsigned int options);

        /** Set the camera used to represent the camera view of this SceneView.*/
        void setCamera(osg::Camera* camera, bool assumeOwnershipOfCamera = true);

        /** Get the camera used to represent the camera view of this SceneView.*/
        osg::Camera* getCamera() { return _camera.get(); }

        /** Get the const camera used to represent the camera view of this SceneView.*/
        const osg::Camera* getCamera() const { return _camera.get(); }

        /** Set the data to view. The data will typically be
         *  an osg::Scene but can be any osg::Node type.
         */
        void setSceneData(osg::Node* node);
        
        /** Get the scene data to view. The data will typically be
         *  an osg::Scene but can be any osg::Node type.
         */
        osg::Node* getSceneData(unsigned int childNo=0) { return (_camera->getNumChildren()>childNo) ? _camera->getChild(childNo) : 0; }

        /** Get the const scene data which to view. The data will typically be
         *  an osg::Scene but can be any osg::Node type.
         */
        const osg::Node* getSceneData(unsigned int childNo=0) const { return (_camera->getNumChildren()>childNo) ? _camera->getChild(childNo) : 0; }
        
        /** Get the number of scene data subgraphs added to the SceneView's camera.*/
        unsigned int getNumSceneData() const { return _camera->getNumChildren(); }

        /** Set the viewport of the scene view to use specified osg::Viewport. */
        void setViewport(osg::Viewport* viewport) { _camera->setViewport(viewport); }

        /** Set the viewport of the scene view to specified dimensions. */
        void setViewport(int x,int y,int width,int height) { _camera->setViewport(x,y,width,height); }


        /** Get the viewport. */
        osg::Viewport* getViewport() { return (_camera->getViewport()!=0) ? _camera->getViewport() : 0; }

        /** Get the const viewport. */
        const osg::Viewport* getViewport() const { return (_camera->getViewport()!=0) ? _camera->getViewport() : 0; }
        
        /** Set the DisplaySettings. */
        inline void setDisplaySettings(osg::DisplaySettings* vs) { _displaySettings = vs; }
        
        /** Get the const DisplaySettings */
        inline const osg::DisplaySettings* getDisplaySettings() const { return _displaySettings.get(); }

        /** Get the DisplaySettings */
        inline osg::DisplaySettings* getDisplaySettings() { return _displaySettings.get(); }


        /** Set the color used in glClearColor().
            Defaults to an off blue color.*/
        void setClearColor(const osg::Vec4& color) { _camera->setClearColor(color); }

        /** Get the color used in glClearColor.*/
        const osg::Vec4& getClearColor() const { return _camera->getClearColor(); }
        
        void setGlobalStateSet(osg::StateSet* state) { _globalStateSet = state; }
        osg::StateSet* getGlobalStateSet() { return _globalStateSet.get(); }
        const osg::StateSet* getGlobalStateSet() const { return _globalStateSet.get(); }

        void setSecondaryStateSet(osg::StateSet* state) { _secondaryStateSet = state; }
        osg::StateSet* getSecondaryStateSet() { return _secondaryStateSet.get(); }
        const osg::StateSet* getSecondaryStateSet() const { return _secondaryStateSet.get(); }

        void setLocalStateSet(osg::StateSet* state) { _localStateSet = state; }
        osg::StateSet* getLocalStateSet() { return _localStateSet.get(); }
        const osg::StateSet* getLocalStateSet() const { return _localStateSet.get(); }
        
        enum ActiveUniforms
        {
            FRAME_NUMBER_UNIFORM            = 1,
            FRAME_TIME_UNIFORM              = 2,
            DELTA_FRAME_TIME_UNIFORM        = 4,
            SIMULATION_TIME_UNIFORM         = 8,
            DELTA_SIMULATION_TIME_UNIFORM   = 16,
            VIEW_MATRIX_UNIFORM             = 32,
            VIEW_MATRIX_INVERSE_UNIFORM     = 64,
            DEFAULT_UNIFORMS                = FRAME_NUMBER_UNIFORM |
                                              FRAME_TIME_UNIFORM |
                                              DELTA_FRAME_TIME_UNIFORM |
                                              SIMULATION_TIME_UNIFORM |
                                              DELTA_SIMULATION_TIME_UNIFORM |
                                              VIEW_MATRIX_UNIFORM |
                                              VIEW_MATRIX_INVERSE_UNIFORM,
            ALL_UNIFORMS                    = 0x7FFFFFFF
        };

        /** Set the uniforms that SceneView should set set up on each frame.*/        
        void setActiveUniforms(int activeUniforms) { _activeUniforms = activeUniforms; }

        /** Get the uniforms that SceneView should set set up on each frame.*/
        int getActiveUniforms() const { return _activeUniforms; }

        void updateUniforms();
        

        typedef Options LightingMode;

        void setLightingMode(LightingMode mode);
        LightingMode getLightingMode() const { return _lightingMode; }

        void setLight(osg::Light* light) { _light = light; }
        osg::Light* getLight() { return _light.get(); }
        const osg::Light* getLight() const { return _light.get(); }
        
        void setState(osg::State* state) { _renderInfo.setState(state); }
        osg::State* getState() { return _renderInfo.getState(); }
        const osg::State* getState() const { return _renderInfo.getState(); }
        
        void setView(osg::View* view) { _camera->setView(view); }
        osg::View* getView() { return _camera->getView(); }
        const osg::View* getView() const { return _camera->getView(); }

        void setRenderInfo(osg::RenderInfo& renderInfo) { _renderInfo = renderInfo; }
        osg::RenderInfo& getRenderInfo() { return _renderInfo; }
        const osg::RenderInfo& getRenderInfo() const { return _renderInfo; }
        


        /** Set the projection matrix. Can be thought of as setting the lens of a camera. */
        inline void setProjectionMatrix(const osg::Matrixf& matrix) { _camera->setProjectionMatrix(matrix); }

        /** Set the projection matrix. Can be thought of as setting the lens of a camera. */
        inline void setProjectionMatrix(const osg::Matrixd& matrix) { _camera->setProjectionMatrix(matrix); }

        /** Set to an orthographic projection. See OpenGL glOrtho for documentation further details.*/
        void setProjectionMatrixAsOrtho(double left, double right,
                                        double bottom, double top,
                                        double zNear, double zFar);

        /** Set to a 2D orthographic projection. See OpenGL glOrtho2D documentation for further details.*/
        void setProjectionMatrixAsOrtho2D(double left, double right,
                                          double bottom, double top);

        /** Set to a perspective projection. See OpenGL glFrustum documentation for further details.*/
        void setProjectionMatrixAsFrustum(double left, double right,
                                          double bottom, double top,
                                          double zNear, double zFar);

        /** Create a symmetrical perspective projection, See OpenGL gluPerspective documentation for further details.
          * Aspect ratio is defined as width/height.*/
        void setProjectionMatrixAsPerspective(double fovy,double aspectRatio,
                                              double zNear, double zFar);

        /** Get the projection matrix.*/
        osg::Matrixd& getProjectionMatrix() { return _camera->getProjectionMatrix(); }

        /** Get the const projection matrix.*/
        const osg::Matrixd& getProjectionMatrix() const { return _camera->getProjectionMatrix(); }

        /** Get the orthographic settings of the orthographic projection matrix. 
          * Returns false if matrix is not an orthographic matrix, where parameter values are undefined.*/
        bool getProjectionMatrixAsOrtho(double& left, double& right,
                                        double& bottom, double& top,
                                        double& zNear, double& zFar) const;

        /** Get the frustum setting of a perspective projection matrix.
          * Returns false if matrix is not a perspective matrix, where parameter values are undefined.*/
        bool getProjectionMatrixAsFrustum(double& left, double& right,
                                          double& bottom, double& top,
                                          double& zNear, double& zFar) const;

        /** Get the frustum setting of a symmetric perspective projection matrix.
          * Returns false if matrix is not a perspective matrix, where parameter values are undefined. 
          * Note, if matrix is not a symmetric perspective matrix then the shear will be lost.
          * Asymmetric matrices occur when stereo, power walls, caves and reality center display are used.
          * In these configurations one should use the 'getProjectionMatrixAsFrustum' method instead.*/
        bool getProjectionMatrixAsPerspective(double& fovy,double& aspectRatio,
                                              double& zNear, double& zFar) const;


        /** Set the view matrix. Can be thought of as setting the position of the world relative to the camera in camera coordinates. */
        inline void setViewMatrix(const osg::Matrixf& matrix) { _camera->setViewMatrix(matrix); }
        
        /** Set the view matrix. Can be thought of as setting the position of the world relative to the camera in camera coordinates. */
        inline void setViewMatrix(const osg::Matrixd& matrix) { _camera->setViewMatrix(matrix); }

        /** Set to the position and orientation of view matrix, using the same convention as gluLookAt. */
        void setViewMatrixAsLookAt(const osg::Vec3& eye,const osg::Vec3& center,const osg::Vec3& up);

        /** Get the view matrix. */
        osg::Matrixd& getViewMatrix() { return _camera->getViewMatrix(); }

        /** Get the const view matrix. */
        const osg::Matrixd& getViewMatrix() const { return _camera->getViewMatrix(); }

        /** Get to the position and orientation of a modelview matrix, using the same convention as gluLookAt. */
        void getViewMatrixAsLookAt(osg::Vec3& eye,osg::Vec3& center,osg::Vec3& up,float lookDistance=1.0f) const;



        
        void setInitVisitor(osg::NodeVisitor* av) { _initVisitor = av; }
        osg::NodeVisitor* getInitVisitor() { return _initVisitor.get(); }
        const osg::NodeVisitor* getInitVisitor() const { return _initVisitor.get(); }


        void setUpdateVisitor(osg::NodeVisitor* av) { _updateVisitor = av; }
        osg::NodeVisitor* getUpdateVisitor() { return _updateVisitor.get(); }
        const osg::NodeVisitor* getUpdateVisitor() const { return _updateVisitor.get(); }


        void setCullVisitor(osgUtil::CullVisitor* cv) { _cullVisitor = cv; }
        osgUtil::CullVisitor* getCullVisitor() { return _cullVisitor.get(); }
        const osgUtil::CullVisitor* getCullVisitor() const { return _cullVisitor.get(); }

        void setCollectOccludersVisitor(osg::CollectOccludersVisitor* cov) { _collectOccludersVisitor = cov; }
        osg::CollectOccludersVisitor* getCollectOccludersVisitor() { return _collectOccludersVisitor.get(); }
        const osg::CollectOccludersVisitor* getCollectOccludersVisitor() const { return _collectOccludersVisitor.get(); }


        void setStateGraph(osgUtil::StateGraph* rg) { _stateGraph = rg; }
        osgUtil::StateGraph* getStateGraph() { return _stateGraph.get(); }
        const osgUtil::StateGraph* getStateGraph() const { return _stateGraph.get(); }

        void setRenderStage(osgUtil::RenderStage* rs) { _renderStage = rs; }
        osgUtil::RenderStage* getRenderStage() { return _renderStage.get(); }
        const osgUtil::RenderStage* getRenderStage() const { return _renderStage.get(); }

        /** search through any pre and post RenderStage that reference a Camera, and take a reference to each of these cameras to prevent them being deleted while they are still be used by the drawing thread.*/
        void collateReferencesToDependentCameras();

        /** clear the refence to any any dependent cameras.*/
        void clearReferencesToDependentCameras();


        /** Set the draw buffer value used at the start of each frame draw. */
        void setDrawBufferValue( GLenum drawBufferValue ) { _camera->setDrawBuffer(drawBufferValue); }

        /** Get the draw buffer value used at the start of each frame draw. */
        GLenum getDrawBufferValue() const { return _camera->getDrawBuffer(); }


        /** Set whether the draw method should call renderer->prioritizeTexture.*/
        void setPrioritizeTextures(bool pt) { _prioritizeTextures = pt; }
        
        /** Get whether the draw method should call renderer->prioritizeTexture.*/
        bool getPrioritizeTextures() const { return _prioritizeTextures; }

        /** Calculate the object coordinates of a point in window coordinates.
            Note, current implementation requires that SceneView::draw() has been previously called
            for projectWindowIntoObject to produce valid values.  Consistent with OpenGL
            windows coordinates are calculated relative to the bottom left of the window.
            Returns true on successful projection.
        */
        bool projectWindowIntoObject(const osg::Vec3& window,osg::Vec3& object) const;

        /** Calculate the object coordinates of a window x,y when projected onto the near and far planes.
            Note, current implementation requires that SceneView::draw() has been previously called
            for projectWindowIntoObject to produce valid values.  Consistent with OpenGL
            windows coordinates are calculated relative to the bottom left of the window.
            Returns true on successful projection.
        */
        bool projectWindowXYIntoObject(int x,int y,osg::Vec3& near_point,osg::Vec3& far_point) const;

        /** Calculate the window coordinates of a point in object coordinates.
            Note, current implementation requires that SceneView::draw() has been previously called
            for projectWindowIntoObject to produce valid values.  Consistent with OpenGL
            windows coordinates are calculated relative to the bottom left of the window,
            whereas window API's normally have the top left as the origin,
            so you may need to pass in (mouseX,window_height-mouseY,...).
            Returns true on successful projection.
        */
        bool projectObjectIntoWindow(const osg::Vec3& object,osg::Vec3& window) const;


        /** Set the frame stamp for the current frame.*/
        inline void setFrameStamp(osg::FrameStamp* fs) { _frameStamp = fs; }

        /** Get the frame stamp for the current frame.*/
        inline const osg::FrameStamp* getFrameStamp() const { return _frameStamp.get(); }


        /** Inherit the local cull settings variable from specified CullSettings object, according to the inheritance mask.*/
        virtual void inheritCullSettings(const osg::CullSettings& settings) { inheritCullSettings(settings, _inheritanceMask); }

        /** Inherit the local cull settings variable from specified CullSettings object, according to the inheritance mask.*/
        virtual void inheritCullSettings(const osg::CullSettings& settings, unsigned int inheritanceMask);


        /** Do init traversal of attached scene graph using Init NodeVisitor.
          * The init traversal is called once for each SceneView, and should
          * be used to compile display list, texture objects intialize data
          * not otherwise intialized during scene graph loading. Note, is
          * called automatically by update & cull if it hasn't already been called
          * elsewhere. Also init() should only ever be called within a valid
          * graphics context.*/
        virtual void init();

        /** Do cull traversal of attached scene graph using Cull NodeVisitor.*/
        virtual void cull();

        /** Do draw traversal of draw bins generated by cull traversal.*/
        virtual void draw();
        
        /** Compute the number of dynamic objects that will be held in the rendering backend */
        unsigned int getDynamicObjectCount() const { return _dynamicObjectCount; }
        
        /** Release all OpenGL objects from the scene graph, such as texture objects, display lists etc.
          * These released scene graphs placed in the respective delete GLObjects cache, which
          * then need to be deleted in OpenGL by SceneView::flushAllDeleteGLObjects(). */
        virtual void releaseAllGLObjects();

        /** Flush all deleted OpenGL objects, such as texture objects, display lists etc.*/
        virtual void flushAllDeletedGLObjects();

        /** Flush deleted OpenGL objects, such as texture objects, display lists etc within specified available time.*/
        virtual void flushDeletedGLObjects(double& availableTime);
        
        /** Extract stats for current draw list. */
        bool getStats(osgUtil::Statistics& primStats); 

    protected:

        virtual ~SceneView();

        /** Do cull traversal of attached scene graph using Cull NodeVisitor. Return true if computeNearFar has been done during the cull traversal.*/
        virtual bool cullStage(const osg::Matrixd& projection,const osg::Matrixd& modelview,osgUtil::CullVisitor* cullVisitor, osgUtil::StateGraph* rendergraph, osgUtil::RenderStage* renderStage, osg::Viewport *viewport);
        
        const osg::Matrix computeMVPW() const;

        void clearArea(int x,int y,int width,int height,const osg::Vec4& color);

        osg::ref_ptr<osg::StateSet>                 _localStateSet;
        osg::RenderInfo                             _renderInfo;
        
        bool                                        _initCalled;
        osg::ref_ptr<osg::NodeVisitor>              _initVisitor;
        osg::ref_ptr<osg::NodeVisitor>              _updateVisitor;
        osg::ref_ptr<osgUtil::CullVisitor>          _cullVisitor;
        osg::ref_ptr<osgUtil::StateGraph>           _stateGraph;
        osg::ref_ptr<osgUtil::RenderStage>          _renderStage;

        osg::ref_ptr<osg::CollectOccludersVisitor>  _collectOccludersVisitor;
        
        osg::ref_ptr<osg::FrameStamp>               _frameStamp;
        
        osg::observer_ptr<osg::Camera>              _camera;
        osg::ref_ptr<osg::Camera>                   _cameraWithOwnership;
        
        osg::ref_ptr<osg::StateSet>                 _globalStateSet;
        osg::ref_ptr<osg::Light>                    _light;
        osg::ref_ptr<osg::DisplaySettings>          _displaySettings;
        
        osg::ref_ptr<osg::StateSet>                 _secondaryStateSet;

        float                                       _fusionDistanceValue;

        LightingMode                                _lightingMode;
        
        bool                                        _prioritizeTextures;
        
        bool                                        _requiresFlush;
        
        int                                         _activeUniforms;        
        double                                      _previousFrameTime;
        double                                      _previousSimulationTime;
        
        unsigned int                                _dynamicObjectCount;        
};

}

#endif


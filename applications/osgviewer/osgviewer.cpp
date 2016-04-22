/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2010 Robert Osfield
 *
 * This application is open source and may be redistributed and/or modified
 * freely and without restriction, both in commercial and non commercial applications,
 * as long as this copyright notice is maintained.
 *
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osg/CoordinateSystemNode>

#include <osg/Switch>
#include <osg/Types>
#include <osgText/Text>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>
#include <osgGA/SphericalManipulator>
#include <osgAnimation/AnimationManagerBase>
#include <osgAnimation/Bone>
#include <osgAnimation/BoneMapVisitor>
#include <osgAnimation/BasicAnimationManager>
#include <osgAnimation/Skeleton>
#include <osg/io_utils>
#include <osg/Geometry>
#include <osgGA/Device>
#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osgDB/FileNameUtils>
#include <osg/Multisample>
#include <osg/Billboard>

#include <iostream>
#include "../osgconv/ShaderConstants.h"
#include "../osgconv/NodeVisitorHelper.h"
#include "../osgconv/TriggerParameterNode.h"

#include <osg/ShapeDrawable>

struct TriggerNodeWithInfo;

osg::ref_ptr<osg::Texture2D> g_texture;

std::map<std::string, std::vector<TriggerNodeWithInfo> > g_triggerMap;
bool g_shouldStopTriggerAnimation = false;
osg::ref_ptr<osgAnimation::BasicAnimationManager> g_animationManager;
std::vector<osg::ref_ptr<osgAnimation::Animation> > g_shouldStopAnimations;

static const char gVertexShader[] =
"varying vec4 color;                                                    \n"
"varying vec2 uv;                                               \n"

"const vec3 lightPos      =vec3(0.0, 0.0, 10.0);                        \n"
"const vec4 modelColor   =vec4(0.8, 0.8, 0.8, 1.0);                    \n"
"const vec4 lightAmbient  =vec4(0.1, 0.1, 0.1, 1.0);                    \n"
"const vec4 lightDiffuse  =vec4(0.4, 0.4, 0.4, 1.0);                    \n"
"const vec4 lightSpecular =vec4(0.8, 0.8, 0.8, 1.0);                    \n"
"void DirectionalLight(in vec3 normal,                                  \n"
"                      in vec3 ecPos,                                   \n"
"                      inout vec4 ambient,                              \n"
"                      inout vec4 diffuse,                              \n"
"                      inout vec4 specular)                             \n"
"{                                                                      \n"
"     float nDotVP;                                                     \n"
"     vec3 L = normalize(gl_ModelViewMatrix*vec4(lightPos, 0.0)).xyz;   \n"
"     nDotVP = max(0.0, dot(normal, L));                                \n"
"                                                                       \n"
"     if (nDotVP > 0.0) {                                               \n"
"       vec3 E = normalize(-ecPos);                                     \n"
"       vec3 R = normalize(reflect( L, normal ));                       \n"
"       specular = pow(max(dot(R, E), 0.0), 16.0) * lightSpecular;      \n"
"     }                                                                 \n"
"     ambient  = lightAmbient;                                          \n"
"     diffuse  = lightDiffuse * nDotVP;                                 \n"
"}                                                                      \n"
"void main() {                                                          \n"
"    vec4 ambiCol = vec4(0.0);                                          \n"
"    vec4 diffCol = vec4(0.0);                                          \n"
"    vec4 specCol = vec4(0.0);                                          \n"
"    gl_Position   = gl_ModelViewProjectionMatrix * gl_Vertex;          \n"
"    vec3 normal   = normalize(gl_NormalMatrix * gl_Normal);            \n"
"    vec4 ecPos    = gl_ModelViewMatrix * gl_Vertex;                    \n"
"    DirectionalLight(normal, ecPos.xyz, ambiCol, diffCol, specCol);    \n"
"    color = modelColor * (ambiCol + diffCol + specCol);               \n"
"    uv = gl_MultiTexCoord0.xy;                                        \n"
"}                                                                      \n";

static const char gFragmentShader[] =
"uniform sampler2D textureMap;             \n"

"varying vec4 color;               \n"
"varying vec2 uv;                                               \n"
"void main() {                             \n"
//"  gl_FragColor = color;                   \n"
"  gl_FragColor = texture2D(textureMap, uv);                   \n"
//"  gl_FragColor = vec4(1, 1, 0, 1);                   \n"

"}                                         \n";

static const char gFragmentShader2[] =
"precision mediump float;                  \n"
"uniform sampler2D textureMap;             \n"

"varying mediump vec4 color;               \n"
"varying mediump vec2 uv;                                               \n"
"void main() {                             \n"
//"  gl_FragColor = color;                   \n"
//"  gl_FragColor = texture2D(textureMap, uv);                   \n"
"  gl_FragColor = vec4(0, 1, 1, 1);                   \n"

"}\n";


static const char gFragmentShader0[] =
"precision mediump float;                  \n"
"uniform sampler2D textureMap;             \n"

"varying mediump vec4 color;               \n"
"varying mediump vec2 uv;                                               \n"
"void main() {                             \n"
"  gl_FragColor = texture2D(textureMap, uv);                   \n"
"}\n";


struct TriggerNodeWithInfo
{
  osg::ref_ptr<osg::Object> m_node;
  
  std::string m_triggerType;
  std::string m_triggerAction;
  bool m_triggerAutoTimeout;
  float m_triggerTimeoutSecs;
  double m_startTime;
};

struct TriggerEventInfo: public osg::Referenced
{
  TriggerEventInfo(const std::string& name)
  {
    m_name = name;
  }
  
  std::string m_name;
};

struct SkeletonFinder : public osg::NodeVisitor
{
  osg::ref_ptr<osgAnimation::Skeleton> _skeleton;
  SkeletonFinder() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
  void apply(osg::Node& node)
  {
    if (_skeleton.valid())
      return;
    
    osgAnimation::Skeleton* s = dynamic_cast<osgAnimation::Skeleton*>(&node);
    
    if(s)
    {
      _skeleton = s;
      return;
    }

    traverse(node);
  }
};

// FlipImage
class FlipImageVisitor : public osg::NodeVisitor
{
public:
  FlipImageVisitor():osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
  {
  }
  
  virtual void apply(osg::Node& node)
  {
    traverse(node);
  }

  virtual void apply(osg::Geode& geode)
  {
    osg::StateSet* geoStateSet =  geode.getStateSet();
    
    if(geoStateSet)
    {
      for(int i = 0; i < geoStateSet->getTextureAttributeList().size(); i++)
      {
        osg::ref_ptr<osg::Texture2D> tex = dynamic_cast<osg::Texture2D*> (geoStateSet->getTextureAttribute(i, osg::StateAttribute::TEXTURE));
        if(tex.get() && (tex->getImage()) && (tex->getImage()->getFileName() != ""))
        {
          if(m_imageMaps.find(tex->getImage()->getFileName()) == m_imageMaps.end())
          {
            osg::notify(osg::NOTICE)<<"FlipImage 1: "<<tex->getImage()->getFileName()<<std::endl;

            m_imageMaps[tex->getImage()->getFileName()] = true;
            tex->getImage()->flipVertical();
          }
        }
      }
    }
    
   
    for (int i = 0; i < geode.getNumDrawables(); i++)
    {
      osg::Drawable* dr = geode.getDrawable(i);
      
      osg::StateSet* stateSet =  dr->getStateSet();
      
      if(stateSet)
      {
        for(int j = 0; j < stateSet->getTextureAttributeList().size(); j++)
        {
          osg::ref_ptr<osg::Texture2D> tex = dynamic_cast<osg::Texture2D*> (stateSet->getTextureAttribute(j, osg::StateAttribute::TEXTURE));
          if(tex.get() && (tex->getImage()) && (tex->getImage()->getFileName() != ""))
          {
            if(m_imageMaps.find(tex->getImage()->getFileName()) == m_imageMaps.end())
            {
              osg::notify(osg::NOTICE)<<"FlipImage 2: "<<tex->getImage()->getFileName()<<std::endl;

              m_imageMaps[tex->getImage()->getFileName()] = true;
              tex->getImage()->flipVertical();
            }
          }
        }
      }
    }
  }
  
  virtual void apply(osg::Transform& node)
  {
    apply((osg::Node&)node);
  }
  
private:
  std::map<std::string, bool> m_imageMaps;

};


struct ModelMatrixUniformCallback: public osg::Uniform::Callback
{
public:
  ModelMatrixUniformCallback(osg::Camera* camera) :
  m_camera(camera)
  {}
  
  virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
  {
    osg::Matrixd modelMatrix = osg::computeLocalToWorld(nv->getNodePath());
    uniform->set(modelMatrix);
  }
  
  osg::Camera* m_camera;
};


struct ViewMatrixUniformCallback: public osg::Uniform::Callback
{
public:
  ViewMatrixUniformCallback(osg::Camera* camera) :
  m_camera(camera)
  {}
  
  virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
  {
    osg::Matrixd viewMatrix = m_camera->getViewMatrix();
    uniform->set(viewMatrix);
  }
  
  osg::Camera* m_camera;
};

struct ProjectionMatrixUniformCallback: public osg::Uniform::Callback
{
public:
  ProjectionMatrixUniformCallback(osg::Camera* camera) :
  m_camera(camera)
  {}
  
  virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
  {
    osg::Matrixd projectionMatrix = m_camera->getProjectionMatrix();
    uniform->set(projectionMatrix);
  }
  
  osg::Camera* m_camera;
};

struct InverseViewMatrixUniformCallback: public osg::Uniform::Callback
{
public:
  InverseViewMatrixUniformCallback(osg::Camera* camera) :
  m_camera(camera)
  {}
  
  virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
  {
    osg::Matrixd invViewMatrix = m_camera->getInverseViewMatrix();
    uniform->set(invViewMatrix);
  }
  
  osg::Camera* m_camera;
};

struct NormalMatrixUniformCallback: public osg::Uniform::Callback
{
public:
  NormalMatrixUniformCallback(osg::Camera* camera) :
  m_camera(camera)
  {}
  
  virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
  {
    osg::Matrixd viewMatrix = m_camera->getViewMatrix();
    osg::Matrixd modelMatrix = osg::computeLocalToWorld(nv->getNodePath());
    osg::Matrixd modelViewMatrix = modelMatrix * viewMatrix;
    modelViewMatrix.setTrans(0.0, 0.0, 0.0);
    
    osg::Matrixd matrix;
    matrix.invert(modelViewMatrix);
    osg::Matrix3 normalMatrix(matrix(0,0), matrix(1,0), matrix(2,0),
                              matrix(0,1), matrix(1,1), matrix(2,1),
                              matrix(0,2), matrix(1,2), matrix(2,2));
    uniform->set(normalMatrix);
  }
  
  osg::Camera* m_camera;
};

struct MVPMatrixUniformCallback: public osg::Uniform::Callback
{
public:
  MVPMatrixUniformCallback(osg::Camera* camera) :
  m_camera(camera)
  {}
  
  virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
  {
    osg::Matrixd viewMatrix = m_camera->getViewMatrix();
    osg::Matrixd modelMatrix = osg::computeLocalToWorld(nv->getNodePath());
    osg::Matrixd modelViewProjectionMatrix = modelMatrix * viewMatrix * m_camera->getProjectionMatrix();
    uniform->set(modelViewProjectionMatrix);
  }
  
  osg::Camera* m_camera;
};

void setupShaderUniformUpdateCallback(osg::ref_ptr<osg::Node> loadedModel, osg::Camera* camera)
{
  FindUniformVisitor findUniformVisitor;
  loadedModel->accept(findUniformVisitor);
  std::map<std::string, std::vector<osg::ref_ptr<osg::Uniform> > > uniformMap = findUniformVisitor.getUniformMap();
  
  osg::notify(osg::NOTICE)<<"g_UniformMap size: "<<uniformMap.size()<<std::endl;
  
  if(uniformMap.find(ImageMetrics::ShaderConstants::UNIFORM_P) != uniformMap.end())
  {
    for(size_t i = 0; i < uniformMap[ImageMetrics::ShaderConstants::UNIFORM_P].size(); i++)
    {
      uniformMap[ImageMetrics::ShaderConstants::UNIFORM_P][i]->setUpdateCallback(new ProjectionMatrixUniformCallback(camera));
    }
  }
  
  if(uniformMap.find(ImageMetrics::ShaderConstants::UNIFORM_NORMALMATRIX) != uniformMap.end())
  {
    for(size_t i = 0; i < uniformMap[ImageMetrics::ShaderConstants::UNIFORM_NORMALMATRIX].size(); i++)
    {
      uniformMap[ImageMetrics::ShaderConstants::UNIFORM_NORMALMATRIX][i]->setUpdateCallback(new NormalMatrixUniformCallback(camera));
    }
  }

  
  if(uniformMap.find(ImageMetrics::ShaderConstants::UNIFORM_MVP) != uniformMap.end())
  {
    for(size_t i = 0; i < uniformMap[ImageMetrics::ShaderConstants::UNIFORM_MVP].size(); i++)
    {
      uniformMap[ImageMetrics::ShaderConstants::UNIFORM_MVP][i]->setUpdateCallback(new MVPMatrixUniformCallback(camera));
    }
  }
  
  if(uniformMap.find(ImageMetrics::ShaderConstants::UNIFORM_V) != uniformMap.end())
  {
    for(size_t i = 0; i < uniformMap[ImageMetrics::ShaderConstants::UNIFORM_V].size(); i++)
    {
      uniformMap[ImageMetrics::ShaderConstants::UNIFORM_V][i]->setUpdateCallback(new ViewMatrixUniformCallback(camera));
    }
  }
  
  if(uniformMap.find(ImageMetrics::ShaderConstants::UNIFORM_M) != uniformMap.end())
  {
    for(size_t i = 0; i < uniformMap[ImageMetrics::ShaderConstants::UNIFORM_M].size(); i++)
    {
      uniformMap[ImageMetrics::ShaderConstants::UNIFORM_M][i]->setUpdateCallback(new ModelMatrixUniformCallback(camera));
    }
  }
  
  if(uniformMap.find(ImageMetrics::ShaderConstants::UNIFORM_V_INV) != uniformMap.end())
  {
    for(size_t i = 0; i < uniformMap[ImageMetrics::ShaderConstants::UNIFORM_V_INV].size(); i++)
    {
      uniformMap[ImageMetrics::ShaderConstants::UNIFORM_V_INV][i]->setUpdateCallback(new InverseViewMatrixUniformCallback(camera));
    }
  }
}


class TriggerHandler : public osgGA::GUIEventHandler
{
public:
  TriggerHandler(std::map<std::string, std::vector<TriggerNodeWithInfo> > &nodeMap, osg::ref_ptr<osgAnimation::BasicAnimationManager> animationManager)
  {
    m_nodeMap = nodeMap;
    m_animationManager = animationManager;
  }
  
  static void getImageSequenceLength(osg::StateSet* stateSet, float &length)
  {
    for(unsigned int j = 0; j < stateSet->getTextureAttributeList().size(); ++j)
    {
      osg::Texture2D* texture = dynamic_cast<osg::Texture2D*>(stateSet->getTextureAttribute(j, osg::StateAttribute::TEXTURE));
      
      if(texture)
      {
        osg::ImageSequence* imageSeq = dynamic_cast<osg::ImageSequence*>(texture->getImage());
        if(imageSeq)
        {
          length = std::max(length, (float)(imageSeq->getLength()));
        }
      }
    }
  }

  
  static void resetImageSequence(osg::StateSet* stateSet, bool switchValue)
  {
    for(unsigned int j = 0; j < stateSet->getTextureAttributeList().size(); ++j)
    {
      osg::Texture2D* texture = dynamic_cast<osg::Texture2D*>(stateSet->getTextureAttribute(j, osg::StateAttribute::TEXTURE));
      
      if(texture)
      {
        osg::ImageSequence* imageSeq = dynamic_cast<osg::ImageSequence*>(texture->getImage());
        if(imageSeq)
        {
// FIXME: should wait for animation done?
//          if(switchValue && imageSeq->getStatus() != osg::ImageStream::PLAYING)
          if(switchValue)
          {
            osg::notify(osg::NOTICE)<<"image seq start play: "<<std::endl;
            imageSeq->rewind();
            imageSeq->play();
          }
          else if(!switchValue)
          {
            osg::notify(osg::NOTICE)<<"image seq paused: "<<std::endl;

            imageSeq->rewind();
            imageSeq->pause();
          }
        }
      }
    }
  }

  static void setSwitchValueForChild(osg::Switch* oneSwitch, bool value)
  {
    for(size_t i = 0; i < oneSwitch->getNumChildren(); i++)
    {
      oneSwitch->setValue(i, value);
      
      osg::Switch* childNode = dynamic_cast<osg::Switch*>(oneSwitch->getChild(i));
      if(childNode)
      {
        setSwitchValueForChild(childNode, value);
      }
    }
  }
  
  static void setSwitchValueForParent(osg::Switch* oneSwitch, bool value)
  {
    std::vector<osg::Group*> parents = oneSwitch->getParents();

    for(size_t i = 0; i < parents.size(); i++)
    {
      osg::Switch* parentNode = dynamic_cast<osg::Switch*>(parents[i]);
      if(parentNode)
      {
        for(size_t j = 0; j < parentNode->getNumChildren(); j++)
        {
          parentNode->setValue(j, value);
        }
        setSwitchValueForParent(parentNode, value);
      }
    }
  }
  
  static std::string getReverseAction(std::string& curAction)
  {
    if(curAction == "hide")
    {
      return "show";
    }
    else if(curAction == "show")
    {
      return "hide";
    }
    else if(curAction == "run")
    {
      return "pause";
    }
    else if(curAction == "pause")
    {
      return "run";
    }
    return "";
  }
  
  void setupTimeoutTrigger(TriggerNodeWithInfo& info, const osgGA::GUIEventAdapter& ea)
  {
    //First find if given node is already inside timeout queue.
    for(int i = 0; i < m_nodeMap["timeout"].size(); i++)
    {
      if(m_nodeMap["timeout"][i].m_node == info.m_node)
      {
        m_nodeMap["timeout"].erase(m_nodeMap["timeout"].begin()+i);
        i--;
      }
    }
    
    TriggerNodeWithInfo oneNode;
    oneNode.m_triggerAction = getReverseAction(info.m_triggerAction);
    
    // No valid reverse action found, then just return
    if(oneNode.m_triggerAction == "")
    {
      return;
    }
    oneNode.m_node = info.m_node;
    oneNode.m_triggerTimeoutSecs = info.m_triggerTimeoutSecs;
    oneNode.m_startTime = ea.getTime();
    oneNode.m_triggerType = "timeout";
    
    m_nodeMap["timeout"].push_back(oneNode);
  }
  
  void applyTriggerAction(TriggerNodeWithInfo& info, const osgGA::GUIEventAdapter& ea)
  {
    bool switchValue;

    if(info.m_triggerAction == "run" || info.m_triggerAction == "pause")
    {
      if(info.m_triggerAction == "run")
      {
        switchValue = true;
      }
      else if(info.m_triggerAction == "pause")
      {
        switchValue = false;
      }
      
      osg::Geode* oneGeode = dynamic_cast<osg::Geode*>(info.m_node.get());
      if(oneGeode)
      {
        osg::StateSet* stateSet =  oneGeode->getOrCreateStateSet();
        resetImageSequence(stateSet, switchValue);
        
        for(size_t j = 0; j < oneGeode->getNumDrawables(); j++)
        {
          osg::StateSet* drawableStateSet = dynamic_cast<osg::Geometry *>(oneGeode->getDrawable(j))->getOrCreateStateSet();
          resetImageSequence(drawableStateSet, switchValue);
        }
      }
      
      // If it is 3d animation node
      osgAnimation::Animation* oneAnim = dynamic_cast<osgAnimation::Animation*>(info.m_node.get());
      if(oneAnim)
      {
        if(switchValue)
        {
          m_animationManager->playAnimation(oneAnim);
        }
        else
        {
          m_animationManager->stopAnimation(oneAnim);
          //FIXME: reset animation to start frame
          oneAnim->update(0);
        }
      }
    }
    else if(info.m_triggerAction == "hide" || info.m_triggerAction == "show")
    {
      if(info.m_triggerAction == "show")
      {
        switchValue = true;
      }
      else if(info.m_triggerAction == "hide")
      {
        switchValue = false;
      }
      
      osg::Geode* oneGeode = dynamic_cast<osg::Geode*>(info.m_node.get());
      if(oneGeode)
      {
        if(switchValue)
        {
          oneGeode->setNodeMask(0xffffffff);
        }
        else
        {
          oneGeode->setNodeMask(0x0);
        }
      }
    }

    // There is timeout setting for this trigger:
    if(info.m_triggerType != "timeout" && info.m_triggerTimeoutSecs > 0.001f)
    {
      setupTimeoutTrigger(info, ea);
    }
  }
  
  virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
  {
    switch(ea.getEventType())
    {
      case osgGA::GUIEventAdapter::FRAME:
      {

        double curTime = ea.getTime();
        
        for(int i = 0; i < m_nodeMap["timeout"].size(); i++)
        {
          if(curTime - m_nodeMap["timeout"][i].m_startTime >= m_nodeMap["timeout"][i].m_triggerTimeoutSecs)
          {
            applyTriggerAction(m_nodeMap["timeout"][i], ea);

            m_nodeMap["timeout"].erase(m_nodeMap["timeout"].begin()+i);
            i--;
          }
        }
      }
      return false;
      break;
      
      case osgGA::GUIEventAdapter::USER:
      {
        const TriggerEventInfo* triggerInfo = dynamic_cast<const TriggerEventInfo*>(ea.getUserData());
        if(triggerInfo != NULL)
        {
          std::vector<TriggerNodeWithInfo> triggerNodes = m_nodeMap[triggerInfo->m_name];
          
          for(std::vector<TriggerNodeWithInfo>::iterator it = triggerNodes.begin(); it != triggerNodes.end(); it++)
          {
            applyTriggerAction(*it, ea);
          }
        }
      }
      return false;
      break;

      case(osgGA::GUIEventAdapter::KEYDOWN):
      {
        switch(ea.getKey())
        {
          case '1':
          {
            static int count1 = 0;
            std::string event;
            
            // Simulate mouth open
            if(count1 % 2 == 0)
            {
              event = "mouthopen:start";
            }
            else
            {
              event = "mouthopen:end";
            }
            count1++;
            
            std::cout << " t key pressed: "<<event<< std::endl;
            osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
            if(viewer)
            {
              viewer->getEventQueue()->userEvent(new TriggerEventInfo(event));
            }
            return false;
            break;
          }
          case '2':
          {
            static int count2 = 0;
            std::string event;
            
            // Simulate eye open
            if(count2 % 2 == 0)
            {
              event = "eyesclosed:start";
            }
            else
            {
              event = "eyesclosed:end";
            }
            count2++;
            
            std::cout << " t key pressed: "<<event<< std::endl;
            osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
            if(viewer)
            {
              viewer->getEventQueue()->userEvent(new TriggerEventInfo(event));
            }
            return false;
            break;
          }
          case '3':
          {
            static int count3 = 0;
            std::string event;
            
            // Simulate browsraised
            if(count3 % 2 == 0)
            {
              event = "browsraised:start";
            }
            else
            {
              event = "browsraised:end";
            }
            count3++;
            
            std::cout << " t key pressed: "<<event<< std::endl;
            osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
            if(viewer)
            {
              viewer->getEventQueue()->userEvent(new TriggerEventInfo(event));
            }
            return false;
            break;
          }
          case '4':
          {
            static int count4 = 0;
            std::string event;
            
            // Simulate head up
            if(count4 % 2 == 0)
            {
              event = "headup:start";
            }
            else
            {
              event = "headup:end";
            }
            count4++;
            
            std::cout << " t key pressed: "<<event<< std::endl;
            osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
            if(viewer)
            {
              viewer->getEventQueue()->userEvent(new TriggerEventInfo(event));
            }
            return false;
            break;
          }
          case '5':
          {
            static int count5 = 0;
            std::string event;
            
            // Simulate head down
            if(count5 % 2 == 0)
            {
              event = "headdown:start";
            }
            else
            {
              event = "headdown:end";
            }
            count5++;
            
            std::cout << " t key pressed: "<<event<< std::endl;
            osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
            if(viewer)
            {
              viewer->getEventQueue()->userEvent(new TriggerEventInfo(event));
            }
            return false;
            break;
          }
          case '6':
          {
            static int count6 = 0;
            std::string event;
            
            // Simulate head left
            if(count6 % 2 == 0)
            {
              event = "headleft:start";
            }
            else
            {
              event = "headleft:end";
            }
            count6++;
            
            std::cout << " t key pressed: "<<event<< std::endl;
            osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
            if(viewer)
            {
              viewer->getEventQueue()->userEvent(new TriggerEventInfo(event));
            }
            return false;
            break;
          }
          case '7':
          {
            static int count7 = 0;
            std::string event;
            
            // Simulate head right
            if(count7 % 2 == 0)
            {
              event = "headright:start";
            }
            else
            {
              event = "headright:end";
            }
            count7++;
            
            std::cout << " t key pressed: "<<event<< std::endl;
            osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
            if(viewer)
            {
              viewer->getEventQueue()->userEvent(new TriggerEventInfo(event));
            }
            return false;
            break;
          }
          default:
            return false;
        }
      }
      default:
        return false;
    }
  }
  
//  virtual void accept(osgGA::GUIEventHandlerVisitor& v)
//  {
//    v.visit(*this);
//  }
  
private:
  std::map<std::string, std::vector<TriggerNodeWithInfo> > m_nodeMap;
  osg::ref_ptr<osgAnimation::BasicAnimationManager> m_animationManager;

};

void setupTriggers(osgViewer::Viewer* viewer, osg::ref_ptr<osg::Node> root)
{
  // Get animation manager
  AnimationManagerFinder finder;
  root->accept(finder);
  g_animationManager = finder._am;
  std::vector<osg::ref_ptr<osg::Node> > shouldTurnOffNodes;

  if(g_animationManager.valid())
  {
    // Play default 3D animation
    for (osgAnimation::AnimationList::const_iterator animIter = g_animationManager->getAnimationList().begin();
         animIter != g_animationManager->getAnimationList().end(); ++animIter)
    {
      g_animationManager->playAnimation(*animIter);
    }
    root->setUpdateCallback(g_animationManager.get());
  }
  
  FindNodeVisitor findNodeVisitor("trigger");
  root->accept(findNodeVisitor);
  std::vector<osg::Node*> triggerList = findNodeVisitor.getNodeList();
  
  for(size_t i = 0; i < triggerList.size(); i++)
  {
    osg::Group* triggerParams = dynamic_cast<osg::Group*>(triggerList[i]);
    
    if(triggerParams != NULL)
    {
      for(int n = 0; n < triggerParams->getNumChildren(); n++)
      {
        osg::ref_ptr<osg::TriggerParameterNode> oneParam = dynamic_cast<osg::TriggerParameterNode*>(triggerParams->getChild(n));
        
        // if it is switch control type
        if(oneParam->getControlType() == "switch")
        {
          FindGeodeNodeVisitor findGeodeNodeVisitor(oneParam->getNodeName());
          root->accept(findGeodeNodeVisitor);
          
          std::vector<osg::Node*> nodes = findGeodeNodeVisitor.getNodeList();
          
          bool defaultValue, shouldTrigger; // Default switch value
          
          // For 2D animation, the switch is always on
          if(oneParam->getTriggerAction() == "run" || oneParam->getTriggerAction() == "pause")
          {
            defaultValue = true;
            
            shouldTrigger = oneParam->getTriggerAction() == "run" ? oneParam->getTriggeredDefault() : !(oneParam->getTriggeredDefault());
            // Check if there is image sequence animation under geode node
            for(size_t j = 0; j < nodes.size(); j++)
            {
              osg::StateSet* stateSet =  nodes[j]->getOrCreateStateSet();
              TriggerHandler::resetImageSequence(stateSet, shouldTrigger);
              
              for(size_t k = 0; k < dynamic_cast<osg::Geode*>(nodes[j])->getNumDrawables(); k++)
              {
                osg::StateSet* drawableStateSet = dynamic_cast<osg::Geometry *>(dynamic_cast<osg::Geode*>(nodes[j])->getDrawable(k))->getOrCreateStateSet();
                TriggerHandler::resetImageSequence(drawableStateSet, shouldTrigger);
              }
            }

          }
          else if(oneParam->getTriggerAction() == "hide")
          {
            defaultValue = !(oneParam->getTriggeredDefault());
            shouldTrigger = defaultValue;
          }
          else if(oneParam->getTriggerAction() == "show")
          {
            defaultValue = oneParam->getTriggeredDefault();
            shouldTrigger = defaultValue;
          }
          
          // If it is auto timeout, then we update timeout seconds from image sequence length
          if(oneParam->getTriggerAutoTimeout())
          {
            float imageSequenceLength = 0.0f;
            // Check if there is image sequence animation under geode node
            for(size_t j = 0; j < nodes.size(); j++)
            {
              osg::StateSet* stateSet =  nodes[j]->getOrCreateStateSet();
              TriggerHandler::getImageSequenceLength(stateSet, imageSequenceLength);
              
              for(size_t k = 0; k < dynamic_cast<osg::Geode*>(nodes[j])->getNumDrawables(); k++)
              {
                osg::StateSet* drawableStateSet = dynamic_cast<osg::Geometry *>(dynamic_cast<osg::Geode*>(nodes[j])->getDrawable(k))->getOrCreateStateSet();
                TriggerHandler::getImageSequenceLength(drawableStateSet, imageSequenceLength);
              }
            }

            oneParam->setTriggerTimeoutSecs(imageSequenceLength);
          }
          
          
          for(size_t j = 0; j < nodes.size(); j++)
          {
            nodes[j]->setNodeMask(0xffffffff);
            
            // Base on the value turn on/off node
            if(!defaultValue)
            {
              shouldTurnOffNodes.push_back(nodes[j]);
            }

            // Add this switch node into trigger map
            TriggerNodeWithInfo oneNode;
            oneNode.m_node = nodes[j];
            oneNode.m_triggerType = oneParam->getTriggerType();
            oneNode.m_triggerAction = oneParam->getTriggerAction();
            oneNode.m_triggerAutoTimeout = oneParam->getTriggerAutoTimeout();
            oneNode.m_triggerTimeoutSecs = oneParam->getTriggerTimeoutSecs();
            oneNode.m_startTime = 0;

            g_triggerMap[oneParam->getTriggerType()].push_back(oneNode);
            
            // Trigger Default and should timeout
            if(shouldTrigger && oneNode.m_triggerTimeoutSecs > 0.01f)
            {
              
              TriggerNodeWithInfo timeoutNode;
              timeoutNode.m_triggerAction = TriggerHandler::getReverseAction(oneNode.m_triggerAction);
              
              // No valid reverse action found, then just return
              if(timeoutNode.m_triggerAction == "")
              {
                continue;
              }
              timeoutNode.m_node = oneNode.m_node;
              timeoutNode.m_triggerTimeoutSecs = oneNode.m_triggerTimeoutSecs;
              timeoutNode.m_startTime = 0;
              timeoutNode.m_triggerType = "timeout";
              g_triggerMap[timeoutNode.m_triggerType].push_back(timeoutNode);
            }
          }
        }
        else if(oneParam->getControlType() == "animationmanager")
        {

          for (osgAnimation::AnimationList::const_iterator animIter = g_animationManager->getAnimationList().begin();
               animIter != g_animationManager->getAnimationList().end(); ++animIter)
          {
            if((*animIter)->getName() == oneParam->getNodeName())
            {
              // Add this animation node into trigger map
              TriggerNodeWithInfo oneNode;
              oneNode.m_node = *animIter;
              oneNode.m_triggerType = oneParam->getTriggerType();
              oneNode.m_triggerAction = oneParam->getTriggerAction();
              oneNode.m_triggerAutoTimeout = oneParam->getTriggerAutoTimeout();
              (*animIter)->computeDuration();
              oneNode.m_triggerTimeoutSecs = oneNode.m_triggerAutoTimeout ? (*animIter)->getDuration() : oneParam->getTriggerTimeoutSecs();
              
              bool shouldTrigger = oneParam->getTriggerAction() == "run" ? oneParam->getTriggeredDefault() : !(oneParam->getTriggeredDefault());
              g_animationManager->playAnimation(*animIter);

              g_triggerMap[oneParam->getTriggerType()].push_back(oneNode);
              
              if(!shouldTrigger)
              {
                g_shouldStopAnimations.push_back(*animIter);
              }
              else
              {
                // Trigger Default and should timeout
                if(oneNode.m_triggerTimeoutSecs > 0.01f)
                {
                  TriggerNodeWithInfo timeoutNode;
                  timeoutNode.m_triggerAction = TriggerHandler::getReverseAction(oneNode.m_triggerAction);
                  
                  // No valid reverse action found, then just return
                  if(timeoutNode.m_triggerAction == "")
                  {
                    continue;
                  }
                  timeoutNode.m_node = oneNode.m_node;
                  timeoutNode.m_triggerTimeoutSecs = oneNode.m_triggerTimeoutSecs;
                  timeoutNode.m_startTime = 0;
                  timeoutNode.m_triggerType = "timeout";
                  g_triggerMap[timeoutNode.m_triggerType].push_back(timeoutNode);
                }
              }
            }
          }
          
        }
      }
    }
  }
  
  TriggerHandler* triggerEventHandler = new TriggerHandler(g_triggerMap, g_animationManager);
  viewer->addEventHandler(triggerEventHandler);
  // Turn off nodes if they should be hidden
  for(size_t i = 0; i < shouldTurnOffNodes.size(); i++)
  {
    shouldTurnOffNodes[i]->setNodeMask(0x0);
  }
}

void setupSkeletonAnimation(osgViewer::Viewer* viewer, osg::ref_ptr<osg::Node> root)
{
  // Find skeleton node
  SkeletonFinder skeletonFinder;
  root->accept(skeletonFinder);
  
  if(skeletonFinder._skeleton)
  {
    osgAnimation::BoneMapVisitor boneMapVisitor;
    skeletonFinder._skeleton->accept(boneMapVisitor);
    const osgAnimation::BoneMap& boneMap = boneMapVisitor.getBoneMap();
    
//    osg::notify(osg::NOTICE)<<"BoneMap size: "<<boneMap.size()<<std::endl;
//    for(osgAnimation::BoneMap::const_iterator it = boneMap.begin(); it != boneMap.end(); it++)
//    {
//      osg::notify(osg::NOTICE)<<it->first<<std::endl;
//    }
    
//    //FIXME: test skeleton animation
//    osgAnimation::BoneMap::const_iterator it = boneMap.find("IM_JAW_ROOT");
//    if (it == boneMap.end())
//    {
//      return;
//    }
//   
//    osg::ref_ptr<osgAnimation::Bone> b1 = it->second;
//    osg::Matrix m1;
//    osg::Matrixd r1=osg::Matrixd(osg::Quat(osg::PI*0.5, osg::Vec3(1, 0, 0)));
//    m1 = r1 * b1->getMatrix();
//    b1->setMatrix(m1);
//    
//    osgAnimation::Bone* parent1 = b1->getBoneParent();
//    if (parent1)
//    {
//      osg::notify(osg::NOTICE)<<"set parent node"<<std::endl;
//
//      b1->setInvBindMatrixInSkeletonSpace(b1->getMatrix() * parent1->getMatrixInSkeletonSpace());
//      //b1->setMatrixInSkeletonSpace(b1->getMatrix() * parent1->getMatrixInSkeletonSpace());
//    }
//    else
//    {
//      osg::notify(osg::NOTICE)<<"set current node"<<std::endl;
//
//      b1->setInvBindMatrixInSkeletonSpace(b1->getMatrix());
//      //b1->setMatrixInSkeletonSpace(b1->getMatrix());
//    }
  }
}

void setupBillboards(osgViewer::Viewer* viewer, osg::ref_ptr<osg::Node> root)
{
  FindGeodeNodeVisitor findGeodeNodeVisitor("pPlane4");
  root->accept(findGeodeNodeVisitor);

  osg::Node* bbNode = findGeodeNodeVisitor.getFirst();
  if(bbNode)
  {
    osg::ref_ptr<osg::Geode> node = dynamic_cast<osg::Geode*>(bbNode);

    std::vector<osg::Group*> parents = node->getParents();
    
    osg::ref_ptr<osg::Billboard> billBoard = new osg::Billboard();
    billBoard->setMode(osg::Billboard::AXIAL_ROT);
    billBoard->setAxis(osg::Vec3(0.0f,0.0f,1.0f));
    billBoard->setNormal(osg::Vec3(0.0f,-1.0f,0.0f));
    osg::notify(osg::NOTICE)<<"add billboard"<<std::endl;
    for (int i = 0; i < node->getNumDrawables(); i++)
    {
      osg::Drawable* dr = node->getDrawable(i);
      billBoard->addDrawable(dr);
    }
    
    for(size_t k = 0; k < parents.size(); k++)
    {
      osg::Group* parentNode = parents[k];
      parentNode->removeChild(node);
      parentNode->addChild(billBoard);
    }
  }
}

int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription("This app is the standard Image Metrics extended OpenSceneGraph viewer which loads and visualises 3d models.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("--image <filename>","Load an image and render it on a quad");
    arguments.getApplicationUsage()->addCommandLineOption("--dem <filename>","Load an image/DEM and render it on a HeightField");
    arguments.getApplicationUsage()->addCommandLineOption("--login <url> <username> <password>","Provide authentication information for http file access.");
    arguments.getApplicationUsage()->addCommandLineOption("-p <filename>","Play specified camera path animation file, previously saved with 'z' key.");
    arguments.getApplicationUsage()->addCommandLineOption("--speed <factor>","Speed factor for animation playing (1 == normal speed).");
    arguments.getApplicationUsage()->addCommandLineOption("--device <device-name>","add named device to the viewer");
    arguments.getApplicationUsage()->addCommandLineOption("--width <width>","viewport width.");
    arguments.getApplicationUsage()->addCommandLineOption("--height <height>","viewport height.");

  
    osgViewer::Viewer viewer(arguments);

  
    unsigned int helpType = 0;
    if ((helpType = arguments.readHelpType()))
    {
        arguments.getApplicationUsage()->write(std::cout, helpType);
        return 1;
    }

    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }

    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

//    std::string url, username, password;
//    while(arguments.read("--login",url, username, password))
//    {
//        if (!osgDB::Registry::instance()->getAuthenticationMap())
//        {
//            osgDB::Registry::instance()->setAuthenticationMap(new osgDB::AuthenticationMap);
//            osgDB::Registry::instance()->getAuthenticationMap()->addAuthenticationDetails(
//                url,
//                new osgDB::AuthenticationDetails(username, password)
//            );
//        }
//    }
//
//    std::string device;
//    while(arguments.read("--device", device))
//    {
//        osg::ref_ptr<osgGA::Device> dev = osgDB::readRefFile<osgGA::Device>(device);
//        if (dev.valid())
//        {
//            viewer.addDevice(dev);
//        }
//    }
//
    // set up the camera manipulators.
//    {
//        osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;
//
//        keyswitchManipulator->addMatrixManipulator( '1', "Trackball", new osgGA::TrackballManipulator() );
//        keyswitchManipulator->addMatrixManipulator( '2', "Flight", new osgGA::FlightManipulator() );
//        keyswitchManipulator->addMatrixManipulator( '3', "Drive", new osgGA::DriveManipulator() );
//        keyswitchManipulator->addMatrixManipulator( '4', "Terrain", new osgGA::TerrainManipulator() );
//        keyswitchManipulator->addMatrixManipulator( '5', "Orbit", new osgGA::OrbitManipulator() );
//        keyswitchManipulator->addMatrixManipulator( '6', "FirstPerson", new osgGA::FirstPersonManipulator() );
//        keyswitchManipulator->addMatrixManipulator( '7', "Spherical", new osgGA::SphericalManipulator() );
//
//        std::string pathfile;
//        double animationSpeed = 1.0;
//        while(arguments.read("--speed",animationSpeed) ) {}
//        char keyForAnimationPath = '8';
//        while (arguments.read("-p",pathfile))
//        {
//            osgGA::AnimationPathManipulator* apm = new osgGA::AnimationPathManipulator(pathfile);
//            if (apm || !apm->valid())
//            {
//                apm->setTimeScale(animationSpeed);
//
//                unsigned int num = keyswitchManipulator->getNumMatrixManipulators();
//                keyswitchManipulator->addMatrixManipulator( keyForAnimationPath, "Path", apm );
//                keyswitchManipulator->selectMatrixManipulator(num);
//                ++keyForAnimationPath;
//            }
//        }
//
//        viewer.setCameraManipulator( keyswitchManipulator.get() );
//    }
  
    arguments.getApplicationUsage()->addKeyboardMouseBinding('1', "Toggling Triggers: Mouth Open/Close");
    arguments.getApplicationUsage()->addKeyboardMouseBinding('2', "Toggling Triggers: Eye Open/Close");
    arguments.getApplicationUsage()->addKeyboardMouseBinding('3', "Toggling Triggers: Brows Raised Start/End");
    arguments.getApplicationUsage()->addKeyboardMouseBinding('4', "Toggling Triggers: Head Up Start/End");
    arguments.getApplicationUsage()->addKeyboardMouseBinding('5', "Toggling Triggers: Head Down Start/End");
    arguments.getApplicationUsage()->addKeyboardMouseBinding('6', "Toggling Triggers: Head Left Start/End");
    arguments.getApplicationUsage()->addKeyboardMouseBinding('7', "Toggling Triggers: Head Right Start/End");

  
    viewer.setCameraManipulator( new osgGA::TrackballManipulator() );
  
    // add the state manipulator
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );

    // add the thread model handler
    viewer.addEventHandler(new osgViewer::ThreadingHandler);

    // add the window size toggle handler
    viewer.addEventHandler(new osgViewer::WindowSizeHandler);

    // add the stats handler
    viewer.addEventHandler(new osgViewer::StatsHandler);

    // add the help handler
    viewer.addEventHandler(new osgViewer::HelpHandler(arguments.getApplicationUsage()));

    // add the record camera path handler
    //viewer.addEventHandler(new osgViewer::RecordCameraPathHandler);

    // add the LOD Scale handler
    //viewer.addEventHandler(new osgViewer::LODScaleHandler);

    // add the screen capture handler
    viewer.addEventHandler(new osgViewer::ScreenCaptureHandler);

    int width = 720;
    int height = 1280;
    if (arguments.read("--width", width))
    {
    }
    if (arguments.read("--height", height))
    {
    }
  
  
    // load the data
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles(arguments);
    if (!loadedModel)
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }

 
    // flip image
    //FlipImageVisitor flipImageVisitor;
    //loadedModel->accept(flipImageVisitor);
  
    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
  
  
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->x = 0;
    traits->y = 0;
    traits->width = width/2;
    traits->height = height/2;
    traits->alpha = 8;
    traits->depth = 16;
    traits->windowDecoration = true;
    traits->supportsResize = false;
    traits->windowName = "IM-OSGViewer";
    traits->doubleBuffer = true;
    traits->sharedContext = 0;
    traits->setInheritedWindowPixelFormat = true;
    traits->sampleBuffers = true;  // For multisampling anti-aliasing
    traits->samples = 16;
  
//    osg::Multisample* pms=new osg::Multisample;
//    pms->setSampleCoverage(1,true);
//    loadedModel->getOrCreateStateSet()->setAttributeAndModes(pms,osg::StateAttribute::ON);
//    osg::DisplaySettings::instance()->setNumMultiSamples(8);
//    loadedModel->getOrCreateStateSet()->setMode(GL_MULTISAMPLE_ARB, osg::StateAttribute::ON);
  
    osg::ref_ptr<osg::GraphicsContext> graphicscontext = osg::GraphicsContext::createGraphicsContext(traits);
  
    if (graphicscontext.valid())
    {
      graphicscontext->setClearColor(osg::Vec4f(1.0f, 0.0f, 0.0f, 1.0f));
      graphicscontext->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      viewer.getCamera()->setGraphicsContext(graphicscontext);
    }
 
    // Camera setup
    viewer.getCamera()->setViewport(0, 0, width, height);
    viewer.getCamera()->setClearColor(osg::Vec4(0.361f, 0.361f, 0.361f,1.0f));
  
//    viewer.getCamera()->setProjectionMatrix(osg::Matrixf(2.89497, -0, 0, 0,
//                                                       0, -1.62743, 0, 0,
//                                                       -0.00139082, -0.000781861, -3.35036, -1,
//                                                       0, -0, -1316.72, 0));
//    viewer.getCamera()->setViewMatrix(osg::Matrixf(1, 0, 0, 0,
//                                                 0, 1, 0, 0,
//                                                 0, 0, 1, 0,
//                                                 0, -35, -520, 1));

  
    // match maya's iphone camera setting
    viewer.getCamera()->setProjectionMatrixAsPerspective(38.8f, static_cast<double>(width)/static_cast<double>(height), 0.1f, 10000.0f);
    // Fix cull layering artifacts
    viewer.getCamera()->setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_FAR_USING_PRIMITIVES);
    viewer.getCamera()->setNearFarRatio(1e-6);
  
    //viewer.getCamera()->setViewMatrix(osg::Matrixf::translate(0, 35, 520));
//FIXME: turn on this for device preview
//    viewer.getCamera()->setProjectionMatrix(osg::Matrixd::identity());
//    //viewer.getCamera()->setViewMatrix(osg::Matrix::identity());
//    viewer.getCamera()->setViewMatrixAsLookAt(osg::Vec3(0.0f,0.0f,-1.0f), osg::Vec3(),osg::Vec3(0.0f,-1.0f,0.0f) );
//    viewer.getCamera()->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
//    viewer.getCamera()->setRenderOrder(osg::Camera::PRE_RENDER);
  
  
    // Setup matrix unifrom update callbacks
    setupShaderUniformUpdateCallback(loadedModel, viewer.getCamera());

    FindImageStreamsVisitor findImageStreamsVisitor;
    loadedModel->accept(findImageStreamsVisitor);
    std::vector<osg::ImageStream*> imgStreams = findImageStreamsVisitor.getImageStreamList();
    std::cout<<"ImageSequence: "<<imgStreams.size()<<std::endl;
  
    for(size_t i = 0; i < imgStreams.size(); i++)
    {
      dynamic_cast<osg::ImageSequence*>(imgStreams[i])->play();
    }
  
    // Setup triggers
    setupTriggers(&viewer, loadedModel);
  
    setupSkeletonAnimation(&viewer, loadedModel);

    //setupBillboards(&viewer, loadedModel);
  
    // optimize the scene graph, remove redundant nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize(loadedModel);

  
    viewer.setSceneData(loadedModel);
    viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);

    viewer.realize();
  
    //osgDB::ReaderWriter::WriteResult result = osgDB::Registry::instance()->writeNode(*loadedModel, "test_billboard.osgt",new osgDB::Options("WriteImageHint=UseExternal"));

    while(!viewer.done())
    {
      viewer.advance();
      viewer.eventTraversal();
      viewer.updateTraversal();
      viewer.renderingTraversals();
      
      // Stop all triggered 3D animation
      if(!g_shouldStopAnimations.empty())
      {
        for(size_t i = 0; i < g_shouldStopAnimations.size(); i++)
        {
          g_animationManager->stopAnimation(g_shouldStopAnimations[i]);
        }
        g_shouldStopAnimations.clear();
      }

    }
  
    return 1;
}

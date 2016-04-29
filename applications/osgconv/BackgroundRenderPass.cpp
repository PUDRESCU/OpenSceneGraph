//
//  BackgroundRenderPass.cpp
//  OptasiaOpenSceneGraph
//
//  Created by Jie Xu on 1/19/16.
//  Copyright © 2016 Image Metrics, Inc. All rights reserved.
//

#include <osg/GL>
#include "BackgroundRenderPass.hpp"
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

namespace ImageMetrics
{
  
  static const char gBackgroundVertexShader[] =
  "#ifdef GL_ES\n"
  "#else\n"
  "#define lowp\n"
  "#define mediump\n"
  "#define highp\n"
  "#endif\n"
  "varying mediump vec2 uv;                                               \n"
  "void main() {                                                          \n"
  "    gl_Position   = gl_ModelViewProjectionMatrix * gl_Vertex;          \n"
  "    uv = gl_MultiTexCoord0.xy;                                        \n"
  "}                                                                      \n";
  
  static const char gBackgroundFragmentShader[] =
  "#ifdef GL_ES\n"
  "precision mediump float;                  \n"
  "#else\n"
  "#define lowp\n"
  "#define mediump\n"
  "#define highp\n"
  "#endif\n"
  "uniform sampler2D textureMap;             \n"
  "varying mediump vec2 uv;                  \n"
  "void main() {                             \n"
  "  gl_FragColor = texture2D(textureMap, uv); \n"
  "}                                         \n";

struct BackgroundPreDrawFBOCallback : public osg::Camera::DrawCallback
{
  BackgroundPreDrawFBOCallback() {}
  
  BackgroundPreDrawFBOCallback(const BackgroundPreDrawFBOCallback& copy,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY): osg::Camera::DrawCallback(copy, copyop)
  {}
  
  BackgroundPreDrawFBOCallback( osg::FrameBufferObject* fbo )
  : _fbo(fbo)
  {
  }
  
  META_Object(ImageMetrics, BackgroundPreDrawFBOCallback)
  
  
  virtual void operator () (osg::RenderInfo& renderInfo) const
  {
    if(_fbo.get())
    {
      _fbo->apply(*renderInfo.getState());
    }
  }
  
  osg::observer_ptr<osg::FrameBufferObject> _fbo;
};

REGISTER_OBJECT_WRAPPER( BackgroundPreDrawFBOCallback_Wrapper,
                        new ImageMetrics::BackgroundPreDrawFBOCallback,
                        ImageMetrics::BackgroundPreDrawFBOCallback,
                        "osg::Object ImageMetrics::BackgroundPreDrawFBOCallback" )
{
}
  
  
struct BackgroundPostDrawFBOCallback : public osg::Camera::DrawCallback
{
  BackgroundPostDrawFBOCallback()
  {
  }
  
  
  virtual void operator () (osg::RenderInfo& renderInfo) const
  {
    //glBindFramebuffer(GL_FRAMEBUFFER, 0 );
  }
};
  
BackgroundRenderPass::BackgroundRenderPass(osg::ref_ptr<osg::IMTexture2DOSG> inputTexture,
                                           int width, int height,
                                           osg::ref_ptr<osg::FrameBufferObject> fbo) :
m_width(width),
m_height(height)
{
  m_rootGroup = new osg::Group;
  m_inputTexture = inputTexture;
  m_fbo = fbo;
  setupCamera();
  m_camera->addChild(createGeometry().get());
  
  //m_rootGroup->addChild(m_camera.get());
  
  setShader();
  
  //m_camera->setPreDrawCallback(new BackgroundPreDrawFBOCallback(m_fbo.get()));
}

BackgroundRenderPass::~BackgroundRenderPass()
{
}

osg::ref_ptr<osg::Group> BackgroundRenderPass::createGeometry()
{
  // Create a geode to display the texture
  osg::ref_ptr<osg::Group> top_group = new osg::Group;

  osg::ref_ptr<osg::Geode> geode = new osg::Geode();
  osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry();
  geode->addDrawable(geometry);
  osg::ref_ptr<osg::Vec3Array> vertexArray = new osg::Vec3Array();
  vertexArray->push_back(osg::Vec3(-1, -1, -0.999f));
  vertexArray->push_back(osg::Vec3(1, -1, -0.999f));
  vertexArray->push_back(osg::Vec3(1, 1, -0.999f));
  vertexArray->push_back(osg::Vec3(-1, 1, -0.999f));
  geometry->setVertexArray(vertexArray);
  osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array();
  colorArray->push_back(osg::Vec4(1.f, 1.f, 1.f, 1.f));
  geometry->setColorArray(colorArray);
  geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
  osg::ref_ptr<osg::Vec2Array> texCoordArray = new osg::Vec2Array();
  texCoordArray->push_back(osg::Vec2(0.f, 0.f));
  texCoordArray->push_back(osg::Vec2(1.f, 0.f));
  texCoordArray->push_back(osg::Vec2(1.f, 1.f));
  texCoordArray->push_back(osg::Vec2(0.f, 1.f));
  geometry->setTexCoordArray(0, texCoordArray);
  geometry->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_FAN, 0, 4));
  
  m_stateSet = geode->getOrCreateStateSet();
  top_group->addChild(geode.get());

  return top_group;
  

}

void BackgroundRenderPass::setupCamera()
{
  m_camera = new osg::Camera;
  m_camera->setDataVariance(osg::Object::STATIC);
  m_camera->setAllowEventFocus(false);
  m_camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
  m_camera->setRenderOrder(osg::Camera::PRE_RENDER);
  m_camera->setProjectionMatrix(osg::Matrixd::identity());
  m_camera->setViewport(0, 0, m_width, m_height);
  
  // Set the camera's clear color and masks
  m_camera->setClearColor(osg::Vec4(0, 0, 1, 1));
  m_camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  
//  m_transform = new osg::MatrixTransform;
//  m_transform->getOrCreateStateSet()->setRenderBinDetails(-1, "RenderBin");
//  m_transform->getOrCreateStateSet()->setAttribute(fbo.get());
//  m_transform->getOrCreateStateSet()->setAttribute(new osg::Viewport(0, 0, m_width, m_height));
//  m_transform->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
}

void BackgroundRenderPass::setShader()
{
  osg::ref_ptr<osg::Shader> vshader = new osg::Shader(osg::Shader::VERTEX, gBackgroundVertexShader);
  osg::ref_ptr<osg::Shader> fshader = new osg::Shader(osg::Shader::FRAGMENT, gBackgroundFragmentShader);
  
  osg::ref_ptr<osg::Program> prog = new osg::Program;
  prog->addShader(vshader);
  prog->addShader(fshader);
  m_stateSet->setAttributeAndModes(prog.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
  m_stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
  m_stateSet->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

  m_stateSet->setTextureAttributeAndModes(0, m_inputTexture, osg::StateAttribute::ON);
  osg::ref_ptr<osg::Uniform> baseTextureU = new osg::Uniform("textureMap",0);
  m_stateSet->addUniform(baseTextureU);
  
  m_stateSet->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
}

}
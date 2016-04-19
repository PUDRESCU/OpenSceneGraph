//
//  OverlayRenderPass.cpp
//  OptasiaOpenSceneGraph
//
//  Created by Jie Xu on 1/19/16.
//  Copyright © 2016 Image Metrics, Inc. All rights reserved.
//

#include "OverlayRenderPass.hpp"
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

namespace ImageMetrics
{
  
OverlayRenderPass::OverlayRenderPass(bool isDeviceDeployment, osg::ref_ptr<osg::Texture2D> inputTexture,
                                     int width, int height) :
m_width(width),
m_height(height)
{
  m_isDeviceDeployment = isDeviceDeployment;
  
  m_vertexShaderCode =  "#ifdef GL_ES\n"
  "#else\n"
  "#define lowp\n"
  "#define mediump\n"
  "#define highp\n"
  "#endif\n"
  "varying mediump vec2 uv;                                               \n"
  "void main() {                                                          \n"
  "    gl_Position   = gl_Vertex;                                         \n"
  "    uv = gl_MultiTexCoord0.xy;                                         \n"
  "}                                                                      \n";
  std::string colorChannel = isDeviceDeployment ? ".bgra" : ".rgba";
  
  m_fragmentShaderCode =  "#ifdef GL_ES\n"
  "precision mediump float;                  \n"
  "#else\n"
  "#define lowp\n"
  "#define mediump\n"
  "#define highp\n"
  "#endif\n"
  "uniform sampler2D textureMap;             \n"
  "varying mediump vec2 uv;                  \n"
  "void main() {                             \n"
  "  gl_FragColor = texture2D(textureMap, uv)" + colorChannel + "; \n"
  "}\n";
  
  m_inputTexture = inputTexture;
  setupCamera();
  m_geode = createGeometry();

  m_camera->addChild(m_geode.get());
  setShader();
}

OverlayRenderPass::~OverlayRenderPass()
{
}

osg::ref_ptr<osg::Node> OverlayRenderPass::createGeometry()
{
  // Create a geode to display the texture
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
  if(m_isDeviceDeployment)
  {
    texCoordArray->push_back(osg::Vec2(0.f, 1.f));
    texCoordArray->push_back(osg::Vec2(1.f, 1.f));
    texCoordArray->push_back(osg::Vec2(1.f, 0.f));
    texCoordArray->push_back(osg::Vec2(0.f, 0.f));
  }
  else
  {
    texCoordArray->push_back(osg::Vec2(0.f, 0.f));
    texCoordArray->push_back(osg::Vec2(1.f, 0.f));
    texCoordArray->push_back(osg::Vec2(1.f, 1.f));
    texCoordArray->push_back(osg::Vec2(0.f, 1.f));
  }
  geometry->setTexCoordArray(0, texCoordArray);
  geometry->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_FAN, 0, 4));
  
  m_stateSet = geode->getOrCreateStateSet();

  return geode;
}

void OverlayRenderPass::setupCamera()
{
  m_camera = new osg::Camera;
  m_camera->setName("OverlayCamera");
  m_camera->setDataVariance(osg::Object::STATIC);
  m_camera->setAllowEventFocus(false);
  m_camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
  m_camera->setRenderOrder(osg::Camera::PRE_RENDER);
  m_camera->setProjectionMatrix(osg::Matrixd::identity());
  m_camera->setViewport(0, 0, m_width, m_height);
  m_camera->setClearMask(GL_DEPTH_BUFFER_BIT);
}

void OverlayRenderPass::setShader()
{
  osg::ref_ptr<osg::Shader> vshader = new osg::Shader(osg::Shader::VERTEX, m_vertexShaderCode);
  osg::ref_ptr<osg::Shader> fshader = new osg::Shader(osg::Shader::FRAGMENT, m_fragmentShaderCode);
  
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
#pragma once

#include <string>
#include <vector>
#include <map>

#include <osg/ref_ptr>
#include <osg/Group>
#include <osg/Camera>
#include <osg/MatrixTransform>
#include <osg/Projection>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/TextureRectangle>
#include <osgViewer/Viewer>
#include <osgViewer/GraphicsWindow>
#include <osgDB/Registry>
#include "Light.h"

typedef unsigned int GLuint;
typedef int GLint;

namespace ImageMetrics
{
  class ShaderBase
  {
  public:
    ShaderBase();
    virtual ~ShaderBase();
    
    const std::string &GetName() const { return m_name; }
    void SetName(const std::string &name) { m_name= name; }
    
    osg::ref_ptr<osg::Uniform> GetUniform(const std::string &uniformName);
    GLint GetUniformLocation(const std::string &uniformName);
    bool HasUniformLocation(const std::string &uniformName);
    
    GLint GetAttributeLocation(const std::string &attributeName);
    bool HasAttributeLocation(const std::string &attributeName, GLint &loc);
    std::string GetVertexShaderCode() const {return m_VertexShaderCode;}
    std::string GetFragmentShaderCode() const {return m_FragmentShaderCode;}
    virtual void SetLights(std::vector<Light>& lights) { m_lights= lights; }
    bool IsBGRAColorChannels() {return m_isBGRAColorChannels;}
    void SetIsBGRAColorChannels(bool mode) {m_isBGRAColorChannels = mode;}
    virtual void PrepareToRender(osg::ref_ptr<osg::StateSet> stateSet);

    bool IsBlinnShader();
    bool IsLambertShader();
    bool IsPhongShader();
    bool IsNormalMapSupport();
    bool IsReflectiveSupport();
    bool IsStandardShader();
  public:
    
    std::map<std::string, osg::ref_ptr<osg::Uniform> > m_UniformMap;
  protected:
    std::map<std::string, GLint> m_Attributes;
    std::map<std::string, GLint> m_Uniforms;

    std::string m_VertexShaderCode;
    std::string m_FragmentShaderCode;
    bool m_usesLights;
    int m_nLights;
    std::string m_name;
    std::vector<Light> m_lights;
    bool m_isBGRAColorChannels;
    osg::ref_ptr<osg::Program> m_prog;

  };

}

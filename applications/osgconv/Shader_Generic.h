#pragma once

#include "ShaderBase.h"

typedef unsigned int GLuint;

namespace ImageMetrics
{
  class Shader_Generic : public ShaderBase
  {
  public:
    Shader_Generic();
    Shader_Generic(const std::string& shaderName);

    virtual Shader_Generic* clone() const { return new Shader_Generic(*this); }
    virtual void PrepareToRender(osg::ref_ptr<osg::StateSet> stateSet);

  private:
    bool IsMojoRGBAShader();
    bool IsTextureRGBAShader();

  };
}
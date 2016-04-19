#include "ShaderBase.h"
#include "ShaderConstants.h"

#include <assert.h>
#include <algorithm>


#if !defined(_CROSSBRIDGE)

////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma warning (disable : 4244)  //__int64 to GLint

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace ImageMetrics
{
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  ShaderBase::ShaderBase()
  : m_usesLights(false)
  , m_nLights(-1)
  , m_isBGRAColorChannels(false)
  {
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  ShaderBase::~ShaderBase()
  {
  }

  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  bool ShaderBase::IsBlinnShader()
  {
    return (m_name.find("blinn") != std::string::npos);
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  bool ShaderBase::IsLambertShader()
  {
    return (m_name.find("lambert") != std::string::npos);
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  bool ShaderBase::IsPhongShader()
  {
    return (m_name.find("phong") != std::string::npos);
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  bool ShaderBase::IsNormalMapSupport()
  {
    return (m_name.find("normalmap") != std::string::npos);
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  bool ShaderBase::IsReflectiveSupport()
  {
    return (m_name.find("reflective") != std::string::npos);
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  bool ShaderBase::IsStandardShader()
  {
    return (IsBlinnShader() || IsLambertShader() || IsPhongShader());
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  bool ShaderBase::HasUniformLocation(const std::string &uniformName)
  {
    GLint loc = GetUniformLocation(uniformName);
    return loc != -2;
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  GLint ShaderBase::GetUniformLocation(const std::string &uniformName)
  {
    std::map<std::string, GLint>::iterator itr = m_Uniforms.find(uniformName);
    return (itr == m_Uniforms.end() ? -2 : itr->second);
  }
  
  osg::ref_ptr<osg::Uniform> ShaderBase::GetUniform(const std::string &uniformName)
  {
    std::map<std::string, osg::ref_ptr<osg::Uniform> >::iterator itr = m_UniformMap.find(uniformName);
    return (itr == m_UniformMap.end() ? NULL : itr->second);
  }

  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  bool ShaderBase::HasAttributeLocation(const std::string &attributeName, GLint &loc)
  {  
    loc= GetAttributeLocation(attributeName);
    return loc != -2;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  GLint ShaderBase::GetAttributeLocation(const std::string &attributeName)
  {
    std::map<std::string, GLint>::iterator itr = m_Attributes.find(attributeName);
    return (itr == m_Attributes.end() ? -2 : itr->second);
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void ShaderBase::PrepareToRender(osg::ref_ptr<osg::StateSet> stateSet)
  {
    if(!m_prog)
    {
      osg::ref_ptr<osg::Shader> vshader = new osg::Shader(osg::Shader::VERTEX, m_VertexShaderCode);
      osg::ref_ptr<osg::Shader> fshader = new osg::Shader(osg::Shader::FRAGMENT, m_FragmentShaderCode);
    
      m_prog = new osg::Program;
      m_prog->addShader(vshader);
      m_prog->addShader(fshader);
    }
    
    stateSet->setAttributeAndModes(m_prog.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
//    stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
//    stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    
    if (m_usesLights)
    {
      unsigned int rInd, gInd, bInd;
      
      rInd = 0; gInd = 1; bInd = 2;
      
      if(!IsBGRAColorChannels())
      {
        rInd = 2;
        gInd = 1;
        bInd = 0;
      }
      
      int nLights = m_lights.size();
      
      if(HasUniformLocation(ShaderConstants::UNIFORM_LIGHT_POS_XYZ_WORLDSPACE))
      {
        osg::ref_ptr<osg::Uniform> uniform = new osg::Uniform(osg::Uniform::FLOAT_VEC4, ShaderConstants::UNIFORM_LIGHT_POS_XYZ_WORLDSPACE, nLights);
        for(int i = 0; i < nLights; i++)
        {
          uniform->setElement(i, m_lights[i].GetPos());
        }
       
        m_UniformMap[ShaderConstants::UNIFORM_LIGHT_POS_XYZ_WORLDSPACE] = uniform;
      }
      

      if(HasUniformLocation(ShaderConstants::UNIFORM_LIGHT_RGB_AMBIENT))
      {
        osg::ref_ptr<osg::Uniform> uniform = new osg::Uniform(osg::Uniform::FLOAT_VEC3, ShaderConstants::UNIFORM_LIGHT_RGB_AMBIENT, nLights);
        for(int i = 0; i < nLights; i++)
        {
          if(m_isBGRAColorChannels)
          {
            const osg::Vec3 &a = m_lights[i].GetAmbient();
            uniform->setElement(i, osg::Vec3(a[bInd], a[gInd], a[rInd]));
          }
          else
          {
            uniform->setElement(i, m_lights[i].GetAmbient());
          }
        }
        
        m_UniformMap[ShaderConstants::UNIFORM_LIGHT_RGB_AMBIENT] = uniform;
      }
      
      if (HasUniformLocation(ShaderConstants::UNIFORM_LIGHT_RGB_DIFFUSE))
      {
        osg::ref_ptr<osg::Uniform> uniform = new osg::Uniform(osg::Uniform::FLOAT_VEC3, ShaderConstants::UNIFORM_LIGHT_RGB_DIFFUSE, nLights);
        for(int i = 0; i < nLights; i++)
        {
          if(m_isBGRAColorChannels)
          {
            const osg::Vec3 &a = m_lights[i].GetDiffuse();
            uniform->setElement(i, osg::Vec3(a[bInd], a[gInd], a[rInd]));
          }
          else
          {
            uniform->setElement(i, m_lights[i].GetDiffuse());
          }
        }
        
        m_UniformMap[ShaderConstants::UNIFORM_LIGHT_RGB_DIFFUSE] = uniform;
      }
      
      if (HasUniformLocation(ShaderConstants::UNIFORM_LIGHT_RGB_SPECULAR))
      {
        osg::ref_ptr<osg::Uniform> uniform = new osg::Uniform(osg::Uniform::FLOAT_VEC3, ShaderConstants::UNIFORM_LIGHT_RGB_SPECULAR, nLights);
        for(int i = 0; i < nLights; i++)
        {
          if(m_isBGRAColorChannels)
          {
            const osg::Vec3 &a = m_lights[i].GetSpecular();
            uniform->setElement(i, osg::Vec3(a[bInd], a[gInd], a[rInd]));
          }
          else
          {
            uniform->setElement(i, m_lights[i].GetSpecular());
          }
        }
        
        m_UniformMap[ShaderConstants::UNIFORM_LIGHT_RGB_SPECULAR] = uniform;
      }
      
      if (HasUniformLocation(ShaderConstants::UNIFORM_LIGHT_POWER))
      {
        osg::ref_ptr<osg::Uniform> uniform = new osg::Uniform(osg::Uniform::FLOAT, ShaderConstants::UNIFORM_LIGHT_POWER, nLights);
        for(int i = 0; i < nLights; i++)
        {
          uniform->setElement(i, m_lights[i].GetPower());
        }
        
        m_UniformMap[ShaderConstants::UNIFORM_LIGHT_POWER] = uniform;
      }
    }
  }
}

#endif
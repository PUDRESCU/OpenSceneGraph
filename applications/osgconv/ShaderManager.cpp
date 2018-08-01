#include "ShaderBase.h"
#include "ShaderManager.h"
#include "Shader_Generic.h"
#include "Light.h"
#include "Material.h"


//------------------------------------------------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------------------------------------------------


namespace ImageMetrics
{

  //-------------------------------------------------------------------------------------------------------------------------

  ShaderManager::ShaderManager()
  {
    m_shaders.clear();
  }

  //-------------------------------------------------------------------------------------------------------------------------
  
  ShaderManager::~ShaderManager()
  {
    for (size_t i = 0; i < m_shaders.size(); i++)
    {
      delete m_shaders[i];
    }
  }
  
  //------------------------------------------------------------------------------------------------------------------------

  ShaderManager &ShaderManager::getInstance()
  {
    static ShaderManager instance; // lazy singleton, instantiated on first use
    return instance;
  }

  //-------------------------------------------------------------------------------------------------------------------------

  void ShaderManager::AddShader(ShaderBase* shader)
  {
    m_shaders.push_back(shader);
  }

  //-------------------------------------------------------------------------------------------------------------------------

  void ShaderManager::InitShaders(std::vector<Light>& lights)
  {
    //shaders handle their own initialisation state
    vector<ShaderBase*>::iterator sIt = m_shaders.begin();
    for (; sIt != m_shaders.end(); ++sIt)
    {
      (*sIt)->SetLights(lights);
    }
    m_lights = lights;
  }

  //-------------------------------------------------------------------------------------------------------------------------
  
  std::string ShaderManager::GetShaderType(const Material &material)
  {
    // Automatically create shader source code from material
    std::string lowerStrMaterial;
    std::string name = material.GetName();
    lowerStrMaterial.resize(name.size());
    std::transform(name.begin(), name.end(), lowerStrMaterial.begin(), ::tolower);
    
    std::string materialType = material.GetType();
    
    // Detect shader type:
    std::string shaderType = "phong";
    
    if(materialType == "")
    {
      if(lowerStrMaterial.find("phong") != std::string::npos)
      {
        shaderType = "phong";
      }
      else if(lowerStrMaterial.find("blinn") != std::string::npos)
      {
        shaderType = "blinn";
      }
      else if(lowerStrMaterial.find("lambert") != std::string::npos)
      {
        shaderType = "lambert";
      }
      else if(lowerStrMaterial.find("mojo_rgba") != std::string::npos)
      {
        shaderType = "mojo_rgba";
      }
      else if(lowerStrMaterial.find("texture_rgba") != std::string::npos)
      {
        shaderType = "texture_rgba";
      }
      else if(lowerStrMaterial.find("occlusion_rgba") != std::string::npos)
      {
        shaderType = "occlusion_rgba";
      }
    }
    else
    {
      if(materialType.find("phong") != std::string::npos)
      {
        shaderType = "phong";
      }
      else if(materialType.find("blinn") != std::string::npos)
      {
        shaderType = "blinn";
      }
      else if(materialType.find("lambert") != std::string::npos)
      {
        shaderType = "lambert";
      }
      else if(materialType.find("mojo_rgba") != std::string::npos)
      {
        shaderType = "mojo_rgba";
      }
      else if(materialType.find("texture_rgba") != std::string::npos)
      {
        shaderType = "texture_rgba";
      }
      else if(materialType.find("occlusion_rgba") != std::string::npos)
      {
        shaderType = "occlusion_rgba";
      }
      else
      {
        shaderType = materialType;
      }
    }
    return shaderType;
  }
  
  //-------------------------------------------------------------------------------------------------------------------------
  
  std::string ShaderManager::GetShaderExtensions(std::string& shaderType, const Material &material)
  {
    std::string shaderExtensions;
    
    // non-standard material no extensions support
    if(shaderType != "phong" && shaderType != "blinn" && shaderType != "lambert")
    {
      return shaderExtensions;
    }
    
    // Detect whether normal map should be supported
    bool isNormalMap = false;
    std::string normalMapType = "";
    if(material.GetNormalMapTextureFileName () != "" || !material.getNormalMapImageSequence().empty())
    {
      isNormalMap = true;
      normalMapType = "normalmap";
    }
    
    // Detect whether reflection should be supported
    bool isReflective = false;
    std::string reflectiveType = "";
    if(material.GetReflectivity () > 0)
    {
      isReflective = true;
      reflectiveType = "reflective";
    }
    
    shaderExtensions = (isNormalMap ? "_"+normalMapType : "")+(isReflective ? "_"+reflectiveType: "");

    
    return shaderExtensions;
  }
  
  //-------------------------------------------------------------------------------------------------------------------------

  ShaderBase* ShaderManager::GetShaderByMaterial(const Material &material)
  {
    // Automatically create shader source code from material
    // Detect shader type:
    std::string shaderType = GetShaderType(material);
    
    // Detect shader extensions:
    std::string shaderExtensions = GetShaderExtensions(shaderType, material);
    
    std::string shaderName = shaderType + shaderExtensions;
    osg::notify(osg::NOTICE)<<"GetShaderByMaterial: should create shader: "<<shaderName<< std::endl;

    //Check if given shader is already created, otherwise create new one
    vector<ShaderBase*>::const_iterator sIt = m_shaders.begin();
    for (; sIt != m_shaders.end(); ++sIt)
    {
      std::string type = material.GetType();
      
      if ((*sIt)->GetName() == shaderName)
      {
        osg::notify(osg::NOTICE)<<"shader exists in shader manager"<< std::endl;
        
        return (*sIt);
      }
    }
    
    // Create a new shader
    Shader_Generic* newShader = new Shader_Generic(shaderName);
    if(newShader)
    {
      newShader->SetLights(m_lights);
      newShader->SetIsBGRAColorChannels(m_isBGRAColorChannels);
      m_shaders.push_back(newShader);
    }
    
    return newShader;
  }

  
  //-------------------------------------------------------------------------------------------------------------------------
  
  void ShaderManager::SetIsBGRColorChannels(bool value)
  {
    //shaders handle their own initialisation state
    vector<ShaderBase*>::iterator sIt = m_shaders.begin();
    for (; sIt != m_shaders.end(); ++sIt)
    {
      (*sIt)->SetIsBGRAColorChannels(value);
    }
    m_isBGRAColorChannels = value;
  }
}

#pragma once

#include <vector>


namespace ImageMetrics
{
  class ShaderBase;
  class Light;
  class Material;


  //Singleton class to manage a list of shaders
  class ShaderManager
  {    
  public:

    // This is used for construction, prevents multiple instances being created
    static ShaderManager& getInstance(); 

    //Add shaders
    void AddShader(ShaderBase* shader);

    void InitShaders(std::vector<Light>& lights);

    void SetIsBGRColorChannels(bool value);
    
    ShaderBase* GetShaderByMaterial(const Material &material);

  private:
    // private constructor necessary to allow only 1 instance
    ShaderManager(void);

    // prevent copies
    ShaderManager(ShaderManager const&) {}

    // prevent assignments
    void operator=(ShaderManager const&) {}

    ~ShaderManager();

    std::string GetShaderType(const Material &material);
    
    std::string GetShaderExtensions(std::string& shaderType, const Material &material);

  private:

    // Shaders
    std::vector< ShaderBase* > m_shaders;
    std::vector<Light> m_lights;
    bool m_isBGRAColorChannels;
  };

}

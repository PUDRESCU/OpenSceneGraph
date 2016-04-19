#include "ShaderConstants.h"

namespace ImageMetrics
{
  
  const std::string ShaderConstants::UNIFORM_M               = "M";
  const std::string ShaderConstants::UNIFORM_MVP             = "MVP";
  const std::string ShaderConstants::UNIFORM_V               = "V";
  const std::string ShaderConstants::UNIFORM_V_INV           = "V_inv";
  const std::string ShaderConstants::UNIFORM_V_CUBEMAP       = "V_CubeMap";
  const std::string ShaderConstants::UNIFORM_P               = "P";
  const std::string ShaderConstants::UNIFORM_P_CUBEMAP       = "P_CubeMap";
  const std::string ShaderConstants::UNIFORM_NORMALMATRIX    = "NormalMatrix";

  const std::string ShaderConstants::UNIFORM_MOJO_BACKGROUND_SAMPLER  = "Sampler_Background";
  const std::string ShaderConstants::UNIFORM_TEXTURESAMPLER  = "Sampler_Texture";
  const std::string ShaderConstants::UNIFORM_NORMALMAPSAMPLER  = "Sampler_NormalMap";
  const std::string ShaderConstants::UNIFORM_CUBEMAPSAMPLER  = "Sampler_CubeMap";
  const std::string ShaderConstants::UNIFORM_BUFFERSAMPLER = "Sampler_Buffer";
  const std::string ShaderConstants::UNIFORM_TRANSPARENCYSAMPLER = "Sampler_Transparency";
  const std::string ShaderConstants::UNIFORM_SPECULARSAMPLER = "Sampler_Specular";
  
  const std::string ShaderConstants::UNIFORM_CAMERA_POS_XYZ_WORLDSPACE = "camera_pos_xyz_worldspace";
  
  const std::string ShaderConstants::UNIFORM_LIGHT_POS_XYZ_WORLDSPACE  = "light_pos_xyz_worldspace";
  const std::string ShaderConstants::UNIFORM_LIGHT_RGB_AMBIENT  = "light_rgb_ambient";
  const std::string ShaderConstants::UNIFORM_LIGHT_RGB_DIFFUSE  = "light_rgb_diffuse";
  const std::string ShaderConstants::UNIFORM_LIGHT_RGB_SPECULAR  = "light_rgb_specular";
  const std::string ShaderConstants::UNIFORM_LIGHT_POWER  = "light_power";
  
  const std::string ShaderConstants::UNIFORM_IMSIZE_X = "imagesize_x";
  const std::string ShaderConstants::UNIFORM_IMSIZE_Y = "imagesize_7";
  
  
  const std::string ShaderConstants::ATTRIBUTE_VERTEX_POS_XYZ_MODELSPACE    = "vertex_pos_xyz_modelspace";
  const std::string ShaderConstants::ATTRIBUTE_VERTEX_NORMAL_XYZ_MODELSPACE = "vertex_normal_xyz_modelspace";
  const std::string ShaderConstants::ATTRIBUTE_VERTEX_TANGENT_XYZ_MODELSPACE = "vertex_tangent_xyz_modelspace";
  const std::string ShaderConstants::ATTRIBUTE_VERTEX_BITANGENT_XYZ_MODELSPACE = "vertex_bitangent_xyz_modelspace";
  const std::string ShaderConstants::ATTRIBUTE_VERTEX_UV                    = "vertex_uv";
  const std::string ShaderConstants::ATTRIBUTE_VERTEX_POS_UVSPACE     = "vertex_pos_uvspace";
  
  const std::string ShaderConstants::UNIFORM_MATERIAL_RGB_COLOUR = "material_rgb_colour";
  const std::string ShaderConstants::UNIFORM_MATERIAL_RGB_AMBIENT = "material_rgb_ambient";
  const std::string ShaderConstants::UNIFORM_MATERIAL_RGB_DIFFUSE  = "material_rgb_diffuse";
  const std::string ShaderConstants::UNIFORM_MATERIAL_RGB_SPECULAR  = "material_rgb_specular";
  const std::string ShaderConstants::UNIFORM_MATERIAL_RGB_REFLECTION = "material_rgb_reflection";
  const std::string ShaderConstants::UNIFORM_MATERIAL_RGB_IRIDESCENCE = "material_rgb_iridescence";
  const std::string ShaderConstants::UNIFORM_MATERIAL_OPACITY = "material_opacity";
  const std::string ShaderConstants::UNIFORM_MATERIAL_SPECULAR_POWER  = "material_specular_power";
  const std::string ShaderConstants::UNIFORM_MATERIAL_SPECULAR_FALLOFF  = "material_specular_falloff";
  const std::string ShaderConstants::UNIFORM_MATERIAL_SPECULARGLOW_POWER = "material_specularglow_power";
  const std::string ShaderConstants::UNIFORM_MATERIAL_SPECULARGLOW_FALLOFF = "material_specularglow_fallofff";
  const std::string ShaderConstants::UNIFORM_MATERIAL_REFLECTIVITY = "material_reflectivity";
  const std::string ShaderConstants::UNIFORM_MATERIAL_REFRACTIVEINDEX = "material_refractiveindex";
  
  
  const std::string ShaderConstants::UNIFORM_MOJO_LIGHTINGWEIGHT = "mojo_lightingWeight";
  const std::string ShaderConstants::UNIFORM_MOJO_IMAGEREFERENCEINTENSITY = "mojo_imageReferenceIntensity";
  const std::string ShaderConstants::UNIFORM_MOJO_PAINTREFERENCEINTENSITY = "mojo_paintReferenceIntensity";
  
  const int ShaderConstants::TEXUREUNIT_DIFFUSE = 0;
  const int ShaderConstants::TEXUREUNIT_TRANSPARENCY = 1;
  const int ShaderConstants::TEXUREUNIT_REFLECTION = 2;
  const int ShaderConstants::TEXUREUNIT_EMISSIVE = 3;
  const int ShaderConstants::TEXUREUNIT_AMBIENT = 4;
  const int ShaderConstants::TEXUREUNIT_NORMALMAP = 5;
  const int ShaderConstants::TEXUREUNIT_SPECULAR = 6;
  
  const int ShaderConstants::TEXUREUNIT_BACKGROUND = 7;
  const int ShaderConstants::TEXUREUNIT_CUBEMAP = 8;


  
  
  const int ShaderConstants::ATTRIBUTE_VERTEX_UNIT_TANGENT_XYZ_MODELSPACE = 6;
  const int ShaderConstants::ATTRIBUTE_VERTEX_UNIT_BITANGENT_XYZ_MODELSPACE = 7;
}
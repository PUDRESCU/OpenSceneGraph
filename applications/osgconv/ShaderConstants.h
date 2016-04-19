#pragma once

#include <string>

#if defined(__APPLE__)
#include "TargetConditionals.h"
#endif
namespace ImageMetrics
{
  //-------------------------------------------------------------------------------------------------------------------------
  
  class ShaderConstants
  {
  public:
    static const std::string UNIFORM_M;
    static const std::string UNIFORM_MVP;
    static const std::string UNIFORM_V;
    static const std::string UNIFORM_V_INV;
    static const std::string UNIFORM_V_CUBEMAP;
    static const std::string UNIFORM_P;
    static const std::string UNIFORM_P_CUBEMAP;
    static const std::string UNIFORM_NORMALMATRIX;

    static const std::string UNIFORM_MOJO_BACKGROUND_SAMPLER;
    static const std::string UNIFORM_TEXTURESAMPLER;
    static const std::string UNIFORM_NORMALMAPSAMPLER;
    static const std::string UNIFORM_CUBEMAPSAMPLER;
    static const std::string UNIFORM_BUFFERSAMPLER;
    static const std::string UNIFORM_TRANSPARENCYSAMPLER;
    static const std::string UNIFORM_SPECULARSAMPLER;
    
    static const std::string UNIFORM_CAMERA_POS_XYZ_WORLDSPACE;
    
    static const std::string UNIFORM_LIGHT_POS_XYZ_WORLDSPACE;
    static const std::string UNIFORM_LIGHT_RGB_AMBIENT;
    static const std::string UNIFORM_LIGHT_RGB_DIFFUSE;
    static const std::string UNIFORM_LIGHT_RGB_SPECULAR;
    static const std::string UNIFORM_LIGHT_POWER;
    
    static const std::string UNIFORM_IMSIZE_X;
    static const std::string UNIFORM_IMSIZE_Y;
    
    static const std::string UNIFORM_MOJO_LIGHTINGWEIGHT;
    static const std::string UNIFORM_MOJO_IMAGEREFERENCEINTENSITY;
    static const std::string UNIFORM_MOJO_PAINTREFERENCEINTENSITY;
    
    static const std::string ATTRIBUTE_VERTEX_POS_XYZ_MODELSPACE;
    static const std::string ATTRIBUTE_VERTEX_NORMAL_XYZ_MODELSPACE;
    static const std::string ATTRIBUTE_VERTEX_TANGENT_XYZ_MODELSPACE;
    static const std::string ATTRIBUTE_VERTEX_BITANGENT_XYZ_MODELSPACE;
    static const std::string ATTRIBUTE_VERTEX_UV;
    static const std::string ATTRIBUTE_VERTEX_POS_UVSPACE;
    
    static const std::string UNIFORM_MATERIAL_RGB_COLOUR;
    static const std::string UNIFORM_MATERIAL_RGB_AMBIENT;
    static const std::string UNIFORM_MATERIAL_RGB_DIFFUSE;
    static const std::string UNIFORM_MATERIAL_RGB_SPECULAR;
    static const std::string UNIFORM_MATERIAL_RGB_REFLECTION;
    static const std::string UNIFORM_MATERIAL_RGB_IRIDESCENCE;
    static const std::string UNIFORM_MATERIAL_SPECULAR_POWER;
    static const std::string UNIFORM_MATERIAL_SPECULAR_FALLOFF;
    static const std::string UNIFORM_MATERIAL_SPECULARGLOW_POWER;
    static const std::string UNIFORM_MATERIAL_SPECULARGLOW_FALLOFF;
    static const std::string UNIFORM_MATERIAL_OPACITY;
    static const std::string UNIFORM_MATERIAL_REFLECTIVITY;
    static const std::string UNIFORM_MATERIAL_REFRACTIVEINDEX;
    
    static const int TEXUREUNIT_DIFFUSE;
    static const int TEXUREUNIT_TRANSPARENCY;
    static const int TEXUREUNIT_REFLECTION;
    static const int TEXUREUNIT_EMISSIVE;
    static const int TEXUREUNIT_AMBIENT;
    static const int TEXUREUNIT_NORMALMAP;
    static const int TEXUREUNIT_SPECULAR;
    static const int TEXUREUNIT_BACKGROUND;
    static const int TEXUREUNIT_CUBEMAP;
    
    static const int ATTRIBUTE_VERTEX_UNIT_TANGENT_XYZ_MODELSPACE;
    static const int ATTRIBUTE_VERTEX_UNIT_BITANGENT_XYZ_MODELSPACE;


  };
  
  //-------------------------------------------------------------------------------------------------------------------------
  
} //namespace
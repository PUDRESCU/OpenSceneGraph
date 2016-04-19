#include "Shader_Generic.h"
#include "ShaderConstants.h"
#include "Material.h"
#include <sstream>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>
#include <osgDB/ReaderWriter>
#include <osgDB/PluginQuery>

////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma warning (disable : 4267)

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace ImageMetrics
{
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  Shader_Generic::Shader_Generic()
  : ShaderBase()
  {
    m_usesLights = true;
    m_nLights = -1;
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  Shader_Generic::Shader_Generic(const std::string& shaderName)
  : ShaderBase()
  {
    m_name = shaderName;
    m_usesLights = true;
    m_nLights = -1;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  bool Shader_Generic::IsMojoRGBAShader()
  {
    return (m_name.find("mojo_rgba") != std::string::npos);
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  bool Shader_Generic::IsTextureRGBAShader()
  {
    return (m_name.find("texture_rgba") != std::string::npos);
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void Shader_Generic::PrepareToRender(osg::ref_ptr<osg::StateSet> stateSet)
  {
    m_nLights = m_lights.size();
    
    std::stringstream ss;
    ss << m_lights.size();
    std::string lightNum = ss.str();
    std::string colorchannel = m_isBGRAColorChannels ? ".bgr" : ".rgb";
    
    std::string vertexShaderPrefix =
    "#ifdef GL_ES\n"
    "#else\n"
    "#define lowp\n"
    "#define mediump\n"
    "#define highp\n"
    "#endif\n";
    
    
    m_VertexShaderCode = vertexShaderPrefix;
    
    // Add lights for standard shaders
    if(IsStandardShader())
    {
      m_VertexShaderCode +=
      "#define NUM_LIGHTS " + lightNum + "\n"
      "uniform highp vec4 " + ShaderConstants::UNIFORM_LIGHT_POS_XYZ_WORLDSPACE + "[NUM_LIGHTS];"
      "uniform mat4 " + ShaderConstants::UNIFORM_MVP + ";"
      "uniform mat4 " + ShaderConstants::UNIFORM_V + ";"
      "uniform mat4 " + ShaderConstants::UNIFORM_M + ";"
      "varying vec3 LightDirection_computespace[NUM_LIGHTS];"
      "varying mediump vec2 UV;"
      "varying vec3 EyeDirection_computespace;";

      
      if(IsNormalMapSupport())
      {
        m_VertexShaderCode +=
        "attribute vec4 " + ShaderConstants::ATTRIBUTE_VERTEX_TANGENT_XYZ_MODELSPACE + ";"
        "attribute vec4 " + ShaderConstants::ATTRIBUTE_VERTEX_BITANGENT_XYZ_MODELSPACE + ";";
      }
      else
      {
        m_VertexShaderCode +=
        "varying mediump vec3 Normal_computespace;";
      }
      
      m_VertexShaderCode +=
      "void main() {"
      "  gl_Position = " + ShaderConstants::UNIFORM_MVP + " * gl_Vertex;"
      "  UV = gl_MultiTexCoord0.xy;"
      "  highp vec4 pos_worldspace= " + ShaderConstants::UNIFORM_M + " * gl_Vertex;"
      "  vec3 vertexPosition_cameraspace = (" + ShaderConstants::UNIFORM_V + " * pos_worldspace).xyz ;"
      "  vec3 n = normalize ((" + ShaderConstants::UNIFORM_V + " * " + ShaderConstants::UNIFORM_M + " * vec4(gl_Normal,0)).xyz);";

      if(IsNormalMapSupport())
      {
        m_VertexShaderCode +=
        "  vec3 t = normalize ((" + ShaderConstants::UNIFORM_V + " * " + ShaderConstants::UNIFORM_M + " * vec4(" + ShaderConstants::ATTRIBUTE_VERTEX_TANGENT_XYZ_MODELSPACE + ".xyz, 0)).xyz);"
        "  vec3 b = normalize ((" + ShaderConstants::UNIFORM_V + " * " + ShaderConstants::UNIFORM_M + " * vec4(" + ShaderConstants::ATTRIBUTE_VERTEX_BITANGENT_XYZ_MODELSPACE + ".xyz, 0)).xyz);"
        "  EyeDirection_computespace.x = dot(-vertexPosition_cameraspace, t);"
        "  EyeDirection_computespace.y = dot(-vertexPosition_cameraspace, b);"
        "  EyeDirection_computespace.z = dot(-vertexPosition_cameraspace, n);"
        "  EyeDirection_computespace = normalize(EyeDirection_computespace);";
      }
      else
      {
        m_VertexShaderCode +=
        "  Normal_computespace = n;"
        "  EyeDirection_computespace= - vertexPosition_cameraspace;";
      }
      
      m_VertexShaderCode +=
      "" // Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.
      "  int i;"
      "  for (i=0; i<NUM_LIGHTS; ++i)"
      "  {"
      "    vec3 lightDir;"
      "    if("+ShaderConstants::UNIFORM_LIGHT_POS_XYZ_WORLDSPACE + "[i].w == 0.0)" // directional light
      "    {"
      "      lightDir = " + ShaderConstants::UNIFORM_LIGHT_POS_XYZ_WORLDSPACE + "[i].xyz;"
      "    }"
      "    else"
      "    {"
      "      vec3 LightPosition_cameraspace = ( " + ShaderConstants::UNIFORM_V + " * vec4(" + ShaderConstants::UNIFORM_LIGHT_POS_XYZ_WORLDSPACE + "[i].xyz,1)).xyz;"
      "      lightDir = LightPosition_cameraspace - vertexPosition_cameraspace;"
      "    }";
      
      if(IsNormalMapSupport())
      {
        m_VertexShaderCode +=
        "    vec3 v;"
        "    v.x = dot(lightDir, t);"
        "    v.y = dot(lightDir, b);"
        "    v.z = dot(lightDir, n);"
        "    LightDirection_computespace[i] = normalize(v);";
      }
      else
      {
        m_VertexShaderCode +=
        "    LightDirection_computespace[i] = lightDir;";
      }
      
      m_VertexShaderCode +=
      "  }"
      "}";
      
      m_Uniforms[ShaderConstants::UNIFORM_MVP] = -1;
      m_Uniforms[ShaderConstants::UNIFORM_V] = -1;
      m_Uniforms[ShaderConstants::UNIFORM_M] = -1;
      m_Uniforms[ShaderConstants::UNIFORM_LIGHT_POS_XYZ_WORLDSPACE] = -1;
    }
    else if(IsMojoRGBAShader())
    {
      m_VertexShaderCode +=
      "varying mediump vec2 UV;"
      "varying mediump vec2 backgroundTexcoord;"
      "uniform mat4 " + ShaderConstants::UNIFORM_MVP + ";"
      "void main() {"
      "  gl_Position = " + ShaderConstants::UNIFORM_MVP + " * gl_Vertex;"
      "  UV = gl_MultiTexCoord0.xy;"
      "  backgroundTexcoord = 0.5 * gl_Position.xy / gl_Position.w + 0.5;"
      "}";
      
      m_Uniforms[ShaderConstants::UNIFORM_MVP] = -1;
    }
    else if(IsTextureRGBAShader())
    {
      m_VertexShaderCode +=
      "varying mediump vec2 UV;"
      "uniform mat4 " + ShaderConstants::UNIFORM_MVP + ";"
      "void main() {"
      "  gl_Position = " + ShaderConstants::UNIFORM_MVP + " * gl_Vertex;"
      "  UV = gl_MultiTexCoord0.xy;"
      "}";
      
      m_Uniforms[ShaderConstants::UNIFORM_MVP] = -1;
    }
    
    // Fragment shader
    std::string fragmentShaderPrefix =
    "#ifdef GL_ES\n"
    "precision mediump float;                  \n"
    "#else\n"
    "#define lowp\n"
    "#define mediump\n"
    "#define highp\n"
    "#endif\n";
    
    m_FragmentShaderCode = fragmentShaderPrefix;
    
    // Add lights for standard shaders
    if(IsStandardShader())
    {
      m_FragmentShaderCode +=
      "#define NUM_LIGHTS " + lightNum + "\n"
      "varying highp vec3 LightDirection_computespace[NUM_LIGHTS];"
      "varying mediump vec2 UV;"
      "varying highp vec3 EyeDirection_computespace;"

      "uniform lowp sampler2D " + ShaderConstants::UNIFORM_TEXTURESAMPLER + ";"
      "uniform lowp vec3 " + ShaderConstants::UNIFORM_LIGHT_RGB_DIFFUSE + "[NUM_LIGHTS];"
      "uniform lowp vec3 " + ShaderConstants::UNIFORM_LIGHT_RGB_AMBIENT + "[NUM_LIGHTS];"
      "uniform lowp vec3 " + ShaderConstants::UNIFORM_LIGHT_RGB_SPECULAR + "[NUM_LIGHTS];"
      "uniform lowp float " + ShaderConstants::UNIFORM_LIGHT_POWER + "[NUM_LIGHTS];"
      "uniform lowp vec3 " + ShaderConstants::UNIFORM_MATERIAL_RGB_AMBIENT + ";"
      "uniform lowp vec3 " + ShaderConstants::UNIFORM_MATERIAL_RGB_DIFFUSE + ";"
      "uniform lowp vec3 " + ShaderConstants::UNIFORM_MATERIAL_RGB_SPECULAR + ";"
      "uniform lowp float " + ShaderConstants::UNIFORM_MATERIAL_OPACITY + ";"
      "uniform lowp float " + ShaderConstants::UNIFORM_MATERIAL_SPECULAR_POWER + ";"
      "uniform float " + ShaderConstants::UNIFORM_MATERIAL_SPECULAR_FALLOFF + ";"
      "uniform lowp float " + ShaderConstants::UNIFORM_MATERIAL_SPECULARGLOW_POWER + ";"
      "uniform float " + ShaderConstants::UNIFORM_MATERIAL_SPECULARGLOW_FALLOFF + ";";
      
      if(!IsLambertShader())
      {
        m_FragmentShaderCode +=
        "uniform lowp sampler2D " + ShaderConstants::UNIFORM_SPECULARSAMPLER + ";";
      }
      
      if(IsNormalMapSupport())
      {
        m_FragmentShaderCode +=
        "uniform lowp sampler2D " + ShaderConstants::UNIFORM_NORMALMAPSAMPLER + ";";
      }
      else
      {
        m_FragmentShaderCode +=
        "varying vec3 Normal_computespace;";
      }
      
      if(IsReflectiveSupport())
      {
        m_FragmentShaderCode +=
        "uniform lowp samplerCube " + ShaderConstants::UNIFORM_CUBEMAPSAMPLER + ";"
        "uniform lowp sampler2D " + ShaderConstants::UNIFORM_TRANSPARENCYSAMPLER + ";"
        "uniform lowp vec3 " + ShaderConstants::UNIFORM_MATERIAL_RGB_REFLECTION + ";"
        "uniform lowp vec3 " + ShaderConstants::UNIFORM_MATERIAL_RGB_IRIDESCENCE + ";"
        "uniform lowp float " + ShaderConstants::UNIFORM_MATERIAL_REFLECTIVITY + ";"
        "uniform mat4 " + ShaderConstants::UNIFORM_V_INV + ";"
        "uniform mat4 " + ShaderConstants::UNIFORM_P + ";"
        "void alphaBlend(in vec3 src_rgb, in float src_a, inout vec3 dst_rgb, inout float dst_a) {"
        "  float out_a= src_a + dst_a * (1.0 - src_a);"
        "  dst_rgb = (src_rgb * src_a + dst_rgb * dst_a * (1.0 - src_a)) / out_a;"
        "  dst_a= out_a;"
        "  if (dst_a == 0.0)"
        "    dst_rgb= vec3(0.0,0.0,0.0);"
        "}";
      }
      
      m_FragmentShaderCode +=
      "void main() {"
      "  lowp vec3 MaterialColour = pow(texture2D( " + ShaderConstants::UNIFORM_TEXTURESAMPLER + ", UV )" + colorchannel + ", vec3(2.2));";
      
      if(!IsLambertShader())
      {
        m_FragmentShaderCode +=
        "  lowp vec3 SpecularColour = pow(texture2D( " + ShaderConstants::UNIFORM_SPECULARSAMPLER + ", UV )" + colorchannel + ", vec3(2.2));";
      }
      
      m_FragmentShaderCode +=
      "  lowp vec3 finalColour = " + ShaderConstants::UNIFORM_MATERIAL_RGB_AMBIENT + " * MaterialColour;";
      
      if(IsNormalMapSupport())
      {
        m_FragmentShaderCode +=
        "  vec3 MaterialNormal_tangentspace = normalize( texture2D( " + ShaderConstants::UNIFORM_NORMALMAPSAMPLER + ", UV ).rgb*2.0 - 1.0 );"
        "  vec3 n = normalize(MaterialNormal_tangentspace);";
      }
      else
      {
        m_FragmentShaderCode +=
        "  vec3 n = normalize(Normal_computespace);";
      }

      m_FragmentShaderCode +=
      "  vec3 E = normalize(EyeDirection_computespace);"
      "  int i;"
      "  for (i=0; i<NUM_LIGHTS; ++i)"
      "  {"
      "    vec3 l = normalize(LightDirection_computespace[i]);"
      "    float cosTheta = clamp( dot( n,l ), 0.0,1.0 );"
      "    if(cosTheta > 0.0)"
      "    {"
      "      lowp vec3 MaterialAmbientColor = " + ShaderConstants::UNIFORM_LIGHT_RGB_AMBIENT + "[i] * " + ShaderConstants::UNIFORM_LIGHT_POWER + "[i] * " + ShaderConstants::UNIFORM_MATERIAL_RGB_DIFFUSE + " * MaterialColour;"
      "      lowp vec3 MaterialDiffuseColor = " + ShaderConstants::UNIFORM_LIGHT_RGB_DIFFUSE + "[i] * " + ShaderConstants::UNIFORM_LIGHT_POWER + "[i] * " + ShaderConstants::UNIFORM_MATERIAL_RGB_DIFFUSE + " * MaterialColour;"
      "      finalColour = finalColour + "
      "      MaterialAmbientColor + "
      "      MaterialDiffuseColor * cosTheta;";
      
      if(!IsLambertShader())
      {
        m_FragmentShaderCode +=
        "      lowp vec3 MaterialSpecularColor = " + ShaderConstants::UNIFORM_LIGHT_RGB_SPECULAR + "[i] * " + ShaderConstants::UNIFORM_LIGHT_POWER + "[i] * " + ShaderConstants::UNIFORM_MATERIAL_RGB_SPECULAR + " * SpecularColour;";
        if(IsBlinnShader())
        {
          m_FragmentShaderCode +=
          "      vec3 R = normalize(l+E);";
        }
        else
        {
          m_FragmentShaderCode +=
          "      vec3 R = reflect(-l, n);";
        }
        
        m_FragmentShaderCode +=
        "      float cosAlpha = clamp( dot( E,R ), 0.0,1.0 );"
        "      finalColour = finalColour + "
        "      MaterialSpecularColor * ( " + ShaderConstants::UNIFORM_MATERIAL_SPECULARGLOW_POWER + " * pow(cosAlpha," + ShaderConstants::UNIFORM_MATERIAL_SPECULARGLOW_FALLOFF + ")  + "
        "      " + ShaderConstants::UNIFORM_MATERIAL_SPECULAR_POWER + " * pow(cosAlpha," + ShaderConstants::UNIFORM_MATERIAL_SPECULAR_FALLOFF + ") );";
      }
      
      m_FragmentShaderCode +=
      "    }"
      "  }";
    
      if(IsReflectiveSupport())
      {
        m_FragmentShaderCode +=
        "  mediump vec3 reflected = reflect (-E, n);"
        "  reflected = vec3 (" + ShaderConstants::UNIFORM_V_INV + " * vec4 (reflected, 0.0));"
        "  float cosTheta = clamp( dot( n,E ), 0.0,1.0 );"
        "  cosTheta = 1.0 / (1.0 + exp(-15.0 * (cosTheta - 0.75))); "
        "  float reflectionIntensity = " + ShaderConstants::UNIFORM_MATERIAL_REFLECTIVITY + ";"
        "  lowp vec3 reflectionColour = vec3( textureCube( " + ShaderConstants::UNIFORM_CUBEMAPSAMPLER + ", reflected )" + colorchannel + ") ; "
        "  lowp float alpha= texture2D( " + ShaderConstants::UNIFORM_TEXTURESAMPLER + ", UV ).a * " + ShaderConstants::UNIFORM_MATERIAL_OPACITY + ";"
        "  reflectionColour= reflectionColour * (cosTheta * " + ShaderConstants::UNIFORM_MATERIAL_RGB_REFLECTION + " + (1.0 - cosTheta) * " + ShaderConstants::UNIFORM_MATERIAL_RGB_IRIDESCENCE + "); "
        "  alphaBlend(reflectionColour, reflectionIntensity, finalColour, alpha); ";
      }
      else
      {
        m_FragmentShaderCode +=
        "  lowp float alpha= texture2D( " + ShaderConstants::UNIFORM_TEXTURESAMPLER + ", UV ).a * " + ShaderConstants::UNIFORM_MATERIAL_OPACITY + ";";
      }
      
      m_FragmentShaderCode +=
      "  gl_FragColor.rgb = pow(finalColour, vec3(1.0/2.2));"
      "  gl_FragColor.a = alpha;"
      "}";
      
      
      m_Uniforms[ShaderConstants::UNIFORM_TEXTURESAMPLER] = -1;
      m_Uniforms[ShaderConstants::UNIFORM_LIGHT_RGB_AMBIENT] = -1;
      m_Uniforms[ShaderConstants::UNIFORM_LIGHT_RGB_DIFFUSE] = -1;
      m_Uniforms[ShaderConstants::UNIFORM_LIGHT_RGB_SPECULAR] = -1;
      m_Uniforms[ShaderConstants::UNIFORM_LIGHT_POWER] = -1;
      m_Uniforms[ShaderConstants::UNIFORM_MATERIAL_RGB_AMBIENT] = -1;
      m_Uniforms[ShaderConstants::UNIFORM_MATERIAL_RGB_DIFFUSE] = -1;
      m_Uniforms[ShaderConstants::UNIFORM_MATERIAL_RGB_SPECULAR] = -1;
      m_Uniforms[ShaderConstants::UNIFORM_MATERIAL_SPECULAR_POWER] = -1;
      m_Uniforms[ShaderConstants::UNIFORM_MATERIAL_SPECULAR_FALLOFF] = -1;
      m_Uniforms[ShaderConstants::UNIFORM_MATERIAL_SPECULARGLOW_POWER] = -1;
      m_Uniforms[ShaderConstants::UNIFORM_MATERIAL_SPECULARGLOW_FALLOFF] = -1;
      m_Uniforms[ShaderConstants::UNIFORM_MATERIAL_OPACITY] = -1;
      
      if(!IsLambertShader())
      {
        m_Uniforms[ShaderConstants::UNIFORM_SPECULARSAMPLER] = -1;
      }
      
      if(IsNormalMapSupport())
      {
        m_Uniforms[ShaderConstants::UNIFORM_NORMALMAPSAMPLER] = -1;
      }
      
      if(IsReflectiveSupport())
      {
        m_Uniforms[ShaderConstants::UNIFORM_CUBEMAPSAMPLER] = -1;
        m_Uniforms[ShaderConstants::UNIFORM_V_INV] = -1;
        m_Uniforms[ShaderConstants::UNIFORM_P] = -1;
        m_Uniforms[ShaderConstants::UNIFORM_MATERIAL_RGB_REFLECTION] = -1;
        m_Uniforms[ShaderConstants::UNIFORM_MATERIAL_RGB_IRIDESCENCE] = -1;
        m_Uniforms[ShaderConstants::UNIFORM_MATERIAL_REFLECTIVITY] = -1;
      }
    }
    else if(IsMojoRGBAShader())
    {
      m_FragmentShaderCode +=
      "varying mediump vec2 UV;"
      "varying mediump vec2 backgroundTexcoord;"
      ""
      "uniform sampler2D " + ShaderConstants::UNIFORM_MOJO_BACKGROUND_SAMPLER + ";"
      "uniform sampler2D " + ShaderConstants::UNIFORM_TEXTURESAMPLER + ";"
      "uniform float " + ShaderConstants::UNIFORM_MOJO_LIGHTINGWEIGHT + ";"
      "uniform float " + ShaderConstants::UNIFORM_MOJO_PAINTREFERENCEINTENSITY + ";"
      "uniform float " + ShaderConstants::UNIFORM_MOJO_IMAGEREFERENCEINTENSITY + ";"
      ""
      "const mediump vec3 rgb2grayVec = vec3(0.2990,0.5870,0.1140);"
      ""
      "void main() {"
      "  gl_FragColor = texture2D(" + ShaderConstants::UNIFORM_TEXTURESAMPLER + ", UV); "
      "  gl_FragColor.rgb = gl_FragColor"+colorchannel +";"
      "  vec3 bgColor = texture2D(" + ShaderConstants::UNIFORM_MOJO_BACKGROUND_SAMPLER + ", backgroundTexcoord)" + colorchannel +";"
      "  float imgIntensity = dot(rgb2grayVec, bgColor);"
      "  float globalScale = " + ShaderConstants::UNIFORM_MOJO_IMAGEREFERENCEINTENSITY + " / " + ShaderConstants::UNIFORM_MOJO_PAINTREFERENCEINTENSITY + ";"
      "  gl_FragColor.rgb = globalScale * gl_FragColor.rgb;"
      "  gl_FragColor.rgb = ((imgIntensity / " + ShaderConstants::UNIFORM_MOJO_IMAGEREFERENCEINTENSITY + " - 1.0) * " + ShaderConstants::UNIFORM_MOJO_LIGHTINGWEIGHT + " + 1.0) * gl_FragColor.rgb;"
      "}";
      
      m_Uniforms[ShaderConstants::UNIFORM_MOJO_BACKGROUND_SAMPLER] = -1;
      m_Uniforms[ShaderConstants::UNIFORM_TEXTURESAMPLER] = -1;
      m_Uniforms[ShaderConstants::UNIFORM_MOJO_LIGHTINGWEIGHT] = -1;
      m_Uniforms[ShaderConstants::UNIFORM_MOJO_PAINTREFERENCEINTENSITY] = -1;
      m_Uniforms[ShaderConstants::UNIFORM_MOJO_IMAGEREFERENCEINTENSITY] = -1;
    }
    else if(IsTextureRGBAShader())
    {
      m_FragmentShaderCode +=
      "varying mediump vec2 UV;"
      "uniform sampler2D " + ShaderConstants::UNIFORM_TEXTURESAMPLER + ";"
      "void main() {"
      "  gl_FragColor = texture2D(" + ShaderConstants::UNIFORM_TEXTURESAMPLER + ", UV); "
      "  gl_FragColor.rgb = gl_FragColor"+colorchannel +";"     
      "}";
      
      m_Uniforms[ShaderConstants::UNIFORM_TEXTURESAMPLER] = -1;
    }
    
    ShaderBase::PrepareToRender(stateSet);
   
    // Initialize uniforms
    //
    if(IsStandardShader())
    {
      m_UniformMap[ShaderConstants::UNIFORM_MVP] = new osg::Uniform(ShaderConstants::UNIFORM_MVP.c_str(), osg::Matrixf());
      m_UniformMap[ShaderConstants::UNIFORM_V] = new osg::Uniform(ShaderConstants::UNIFORM_V.c_str(), osg::Matrixf());
      m_UniformMap[ShaderConstants::UNIFORM_M] = new osg::Uniform(ShaderConstants::UNIFORM_M.c_str(), osg::Matrixf());
      
      if(IsReflectiveSupport())
      {
        m_UniformMap[ShaderConstants::UNIFORM_V_INV] = new osg::Uniform(ShaderConstants::UNIFORM_V_INV.c_str(), osg::Matrixf());
        m_UniformMap[ShaderConstants::UNIFORM_P] = new osg::Uniform(ShaderConstants::UNIFORM_P.c_str(), osg::Matrixf());
      }
      
      if(IsNormalMapSupport())
      {
        m_prog->addBindAttribLocation(ShaderConstants::ATTRIBUTE_VERTEX_TANGENT_XYZ_MODELSPACE, ShaderConstants::ATTRIBUTE_VERTEX_UNIT_TANGENT_XYZ_MODELSPACE);
        m_prog->addBindAttribLocation(ShaderConstants::ATTRIBUTE_VERTEX_BITANGENT_XYZ_MODELSPACE, ShaderConstants::ATTRIBUTE_VERTEX_UNIT_BITANGENT_XYZ_MODELSPACE);
      }
    }
    else if(IsMojoRGBAShader())
    {
      m_UniformMap[ShaderConstants::UNIFORM_MVP] = new osg::Uniform(ShaderConstants::UNIFORM_MVP.c_str(), osg::Matrixf());
//      m_UniformMap[ShaderConstants::UNIFORM_MOJO_LIGHTINGWEIGHT] = new osg::Uniform(ShaderConstants::UNIFORM_MOJO_LIGHTINGWEIGHT.c_str(), 1.0f);
//      m_UniformMap[ShaderConstants::UNIFORM_MOJO_PAINTREFERENCEINTENSITY] = new osg::Uniform(ShaderConstants::UNIFORM_MOJO_PAINTREFERENCEINTENSITY.c_str(), 0.5f);
//      m_UniformMap[ShaderConstants::UNIFORM_MOJO_IMAGEREFERENCEINTENSITY] = new osg::Uniform(ShaderConstants::UNIFORM_MOJO_IMAGEREFERENCEINTENSITY.c_str(), 0.5f);
    }
    else if(IsTextureRGBAShader())
    {
      m_UniformMap[ShaderConstants::UNIFORM_MVP] = new osg::Uniform(ShaderConstants::UNIFORM_MVP.c_str(), osg::Matrixf());
    }
  }
  
  
}

#include <stdio.h>
#include <stdlib.h>
#include "Material.h"
#include "ShaderBase.h"
#include "ShaderConstants.h"

#include <exception>
#include <numeric>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>
#include <osgDB/ReaderWriter>
#include <osgDB/PluginQuery>
#include <osg/ImageSequence>
#include <sstream>
#include "Texture2DImageSequence.h"
#include "IMTexture2DOSG.h"

//NOTE - BYTEORDERING: even though our texture buffer is RGB order on the GPU for consistency between OpenGL and OpenGLES we upload 
// all colours the wrong way around as BGR.  Shaders deal with this reverse ordering if required.  This is for efficiency and the 
// restrictions of OpenGLES to RGB ordering on upload

//-------------------------------------------------------------------------------------------------------------------------

#pragma warning (disable : 4267) // check http://www.viva64.com/content/articles/64-bit-development/?f=size_t_and_ptrdiff_t.html&lang=en&content=64-bit-development 

//-------------------------------------------------------------------------------------------------------------------------

#define MAX_COLOR_CHANNEL 3

//-------------------------------------------------------------------------------------------------------------------------

using namespace std;

//-------------------------------------------------------------------------------------------------------------------------

namespace ImageMetrics
{
  //-------------------------------------------------------------------------------------------------------------------------

  Material::Material()
  {
    //m_normalMap.reset(new NormalMap());
    m_colour = osg::Vec3(1.0, 1.0, 1.0);
    m_diffuse = osg::Vec3(0.0, 0.0, 0.0);
    m_ambient = osg::Vec3(0.0, 0.0, 0.0);
    m_specular = osg::Vec3(0.0, 0.0, 0.0);
    m_transparency_color = osg::Vec3(0.0, 0.0, 0.0);
    m_background_color = osg::Vec3(0.0, 0.0, 0.0);

    m_opacity = 0.0;
    m_specular_power = 0.0;
    m_specular_falloff = 0.0;
    m_specularglow_power = 0.0;
    m_specularglow_falloff = 24.0;
    m_reflectivity = 0.0;
    m_refractiveindex = 0.99f;
    
    m_animationFPS = 24;
    
    m_cubeMapImages.clear();
    m_cubeMapImages.push_back("");
    m_cubeMapImages.push_back("");
    m_cubeMapImages.push_back("");
    m_cubeMapImages.push_back("");
    m_cubeMapImages.push_back("");
    m_cubeMapImages.push_back("");

  }

  //-------------------------------------------------------------------------------------------------------------------------

  Material::~Material()
  {
    Destroy();
  }

  //-------------------------------------------------------------------------------------------------------------------------

  void Material::Destroy()
  {
  }


  //-------------------------------------------------------------------------------------------------------------------------

  void Material::SetTexture(const std::string &filename)
  {
    m_texture = new osg::Texture2D;
    osg::ref_ptr<osg::Image> image = osgDB::readImageFile(filename);
    m_texture->setImage(image);
    m_texture->setResizeNonPowerOfTwoHint(false);
  }

  //-------------------------------------------------------------------------------------------------------------------------
  
  void Material::SetBackgroundTexture(const std::string &filename)
  {
    m_background_texture = new osg::Texture2D;
    osg::ref_ptr<osg::Image> image = osgDB::readImageFile(filename);
    m_background_texture->setImage(image);
    m_background_texture->setResizeNonPowerOfTwoHint(false);
  }
  
  
//  //-------------------------------------------------------------------------------------------------------------------------
//
//  void Material::SetNormalMap(const std::string &filename)
//  {
//    GetCachedNormalMap(filename,m_normalMap);
//    //m_normalMap->Set(filename);
//  }
//
//  //-------------------------------------------------------------------------------------------------------------------------
//
//  void Material::SetNormalMap(const IMImagePtr image)
//  {
//    m_normalMap->Set(image);
//  }
//
//  //-------------------------------------------------------------------------------------------------------------------------
//
//  void Material::ClearNormalMap()
//  {
//    m_normalMap->Clear();
//  }

  //-------------------------------------------------------------------------------------------------------------------------

  void Material::SetCubeMap(const vector<string> &filenames)
  {
    m_cubeMapImages = filenames;
  }

  //-------------------------------------------------------------------------------------------------------------------------

  void Material::ClearCubeMap()
  {
    m_cubeMap = NULL;
  }

  //-------------------------------------------------------------------------------------------------------------------------

  void Material::SetOpacity(float opacity)
  {
    m_opacity= opacity;
  }

  //-------------------------------------------------------------------------------------------------------------------------
  void Material::PrepareToRender(ShaderBase* shader, osg::ref_ptr<osg::StateSet> stateSet)
  {
    //NOTE: see note at top of file BYTEORDERING
    unsigned int rInd, gInd, bInd;
    rInd = 0; gInd = 1; bInd = 2;

    if(!shader->IsBGRAColorChannels())
    {
      rInd = 2;
      gInd = 1;
      bInd = 0;
    }
    
    if (shader->HasUniformLocation(ShaderConstants::UNIFORM_MATERIAL_RGB_COLOUR))
    {
      shader->m_UniformMap[ShaderConstants::UNIFORM_MATERIAL_RGB_COLOUR] = new osg::Uniform(ShaderConstants::UNIFORM_MATERIAL_RGB_COLOUR.c_str(), osg::Vec3(m_colour[bInd], m_colour[gInd], m_colour[rInd]));
    }
    
    if (shader->HasUniformLocation(ShaderConstants::UNIFORM_MATERIAL_RGB_AMBIENT))
    {
      shader->m_UniformMap[ShaderConstants::UNIFORM_MATERIAL_RGB_AMBIENT] = new osg::Uniform(ShaderConstants::UNIFORM_MATERIAL_RGB_AMBIENT.c_str(), osg::Vec3(m_ambient[bInd], m_ambient[gInd], m_ambient[rInd]));
    }
    
    if (shader->HasUniformLocation(ShaderConstants::UNIFORM_MATERIAL_RGB_DIFFUSE))
    {
      shader->m_UniformMap[ShaderConstants::UNIFORM_MATERIAL_RGB_DIFFUSE] = new osg::Uniform(ShaderConstants::UNIFORM_MATERIAL_RGB_DIFFUSE.c_str(), osg::Vec3(m_diffuse[bInd], m_diffuse[gInd], m_diffuse[rInd]));
    }
    
    if (shader->HasUniformLocation(ShaderConstants::UNIFORM_MATERIAL_RGB_SPECULAR))
    {
      shader->m_UniformMap[ShaderConstants::UNIFORM_MATERIAL_RGB_SPECULAR] = new osg::Uniform(ShaderConstants::UNIFORM_MATERIAL_RGB_SPECULAR.c_str(), osg::Vec3(m_specular[bInd], m_specular[gInd], m_specular[rInd]));
    }
    
    if (shader->HasUniformLocation(ShaderConstants::UNIFORM_MATERIAL_RGB_REFLECTION))
    {
      shader->m_UniformMap[ShaderConstants::UNIFORM_MATERIAL_RGB_REFLECTION] = new osg::Uniform(ShaderConstants::UNIFORM_MATERIAL_RGB_REFLECTION.c_str(), osg::Vec3(m_reflection[bInd], m_reflection[gInd], m_reflection[rInd]));
    }
    
    if (shader->HasUniformLocation(ShaderConstants::UNIFORM_MATERIAL_RGB_IRIDESCENCE))
    {
      shader->m_UniformMap[ShaderConstants::UNIFORM_MATERIAL_RGB_IRIDESCENCE] = new osg::Uniform(ShaderConstants::UNIFORM_MATERIAL_RGB_IRIDESCENCE.c_str(), osg::Vec3(m_iridescence[bInd], m_iridescence[gInd], m_iridescence[rInd]));
    }
    
    if (shader->HasUniformLocation(ShaderConstants::UNIFORM_MATERIAL_SPECULAR_POWER))
    {
      shader->m_UniformMap[ShaderConstants::UNIFORM_MATERIAL_SPECULAR_POWER] = new osg::Uniform(ShaderConstants::UNIFORM_MATERIAL_SPECULAR_POWER.c_str(), m_specular_power);
    }
    
    if (shader->HasUniformLocation(ShaderConstants::UNIFORM_MATERIAL_SPECULAR_FALLOFF))
    {
      shader->m_UniformMap[ShaderConstants::UNIFORM_MATERIAL_SPECULAR_FALLOFF] = new osg::Uniform(ShaderConstants::UNIFORM_MATERIAL_SPECULAR_FALLOFF.c_str(), m_specular_falloff);
    }
    
    if (shader->HasUniformLocation(ShaderConstants::UNIFORM_MATERIAL_SPECULARGLOW_POWER))
    {
      shader->m_UniformMap[ShaderConstants::UNIFORM_MATERIAL_SPECULARGLOW_POWER] = new osg::Uniform(ShaderConstants::UNIFORM_MATERIAL_SPECULARGLOW_POWER.c_str(), m_specularglow_power);
    }
    
    if (shader->HasUniformLocation(ShaderConstants::UNIFORM_MATERIAL_SPECULARGLOW_FALLOFF))
    {
      shader->m_UniformMap[ShaderConstants::UNIFORM_MATERIAL_SPECULARGLOW_FALLOFF] = new osg::Uniform(ShaderConstants::UNIFORM_MATERIAL_SPECULARGLOW_FALLOFF.c_str(), m_specularglow_falloff);
    }
    
    if (shader->HasUniformLocation(ShaderConstants::UNIFORM_MATERIAL_OPACITY))
    {
      shader->m_UniformMap[ShaderConstants::UNIFORM_MATERIAL_OPACITY] = new osg::Uniform(ShaderConstants::UNIFORM_MATERIAL_OPACITY.c_str(), m_opacity);
    }
    
    if (shader->HasUniformLocation(ShaderConstants::UNIFORM_MATERIAL_REFLECTIVITY))
    {
      shader->m_UniformMap[ShaderConstants::UNIFORM_MATERIAL_REFLECTIVITY] = new osg::Uniform(ShaderConstants::UNIFORM_MATERIAL_REFLECTIVITY.c_str(), m_reflectivity);
    }
    
    if (shader->HasUniformLocation(ShaderConstants::UNIFORM_MATERIAL_REFRACTIVEINDEX))
    {
      shader->m_UniformMap[ShaderConstants::UNIFORM_MATERIAL_REFRACTIVEINDEX] = new osg::Uniform(ShaderConstants::UNIFORM_MATERIAL_REFRACTIVEINDEX.c_str(), m_refractiveindex);
    }
    
    if (shader->HasUniformLocation(ShaderConstants::UNIFORM_TEXTURESAMPLER))
    {
      if(!m_diffuseImageSequence.empty())
      {
        LoadImageSequenceUniform(shader, stateSet, m_diffuseImageSequence, ShaderConstants::UNIFORM_TEXTURESAMPLER, m_texture, ShaderConstants::TEXUREUNIT_DIFFUSE);
      }
      else
      {
        LoadTextureUniform(shader, stateSet, m_diffuse_texture_filename, ShaderConstants::UNIFORM_TEXTURESAMPLER,
                         m_colour, m_opacity, m_texture, ShaderConstants::TEXUREUNIT_DIFFUSE);
      }
    }
    else
    {
      RemoveRedundantTexture(stateSet, ShaderConstants::TEXUREUNIT_DIFFUSE);
    }
    
    if (shader->HasUniformLocation(ShaderConstants::UNIFORM_TRANSPARENCYSAMPLER))
    {
      LoadTextureUniform(shader, stateSet, m_transparency_texture_filename, ShaderConstants::UNIFORM_TRANSPARENCYSAMPLER,
                         osg::Vec3(1-m_opacity, 1-m_opacity, 1-m_opacity), m_opacity, m_transparency_texture, ShaderConstants::TEXUREUNIT_TRANSPARENCY);
    }
    else
    {
      RemoveRedundantTexture(stateSet, ShaderConstants::TEXUREUNIT_TRANSPARENCY);
    }
    
    if (shader->HasUniformLocation(ShaderConstants::UNIFORM_SPECULARSAMPLER))
    {
      LoadTextureUniform(shader, stateSet, m_specular_texture_filename, ShaderConstants::UNIFORM_SPECULARSAMPLER,
                         m_specular, 1, m_specular_texture, ShaderConstants::TEXUREUNIT_SPECULAR);
    }
    else
    {
      RemoveRedundantTexture(stateSet, ShaderConstants::TEXUREUNIT_SPECULAR);
    }
    
    if (shader->HasUniformLocation(ShaderConstants::UNIFORM_MOJO_BACKGROUND_SAMPLER))
    {
      //background texture is from camera image
      osg::ref_ptr<osg::IMImageOSG> inputImage = new osg::IMImageOSG();
      osg::ref_ptr<osg::IMTexture2DOSG> inputTexture = new osg::IMTexture2DOSG;
      inputTexture->setDataVariance(osg::Object::DYNAMIC);
      inputTexture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
      inputTexture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
      inputTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
      inputTexture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
      inputTexture->setIMImageOSG(inputImage);
      inputTexture->setResizeNonPowerOfTwoHint(false);
      
      shader->m_UniformMap[ShaderConstants::UNIFORM_MOJO_BACKGROUND_SAMPLER] = new osg::Uniform(ShaderConstants::UNIFORM_MOJO_BACKGROUND_SAMPLER.c_str(), ShaderConstants::TEXUREUNIT_BACKGROUND);
      stateSet->setTextureAttributeAndModes(ShaderConstants::TEXUREUNIT_BACKGROUND, inputTexture, osg::StateAttribute::ON);
    }
    else
    {
      RemoveRedundantTexture(stateSet, ShaderConstants::TEXUREUNIT_BACKGROUND);
    }
    
    if (shader->HasUniformLocation(ShaderConstants::UNIFORM_CUBEMAPSAMPLER))
    {
      if(m_cubeMapImages.size() >= 6)
      {
        osg::notify(osg::NOTICE)<<"load cubemap"<< std::endl;
        osg::notify(osg::NOTICE)<<m_cubeMapImages[0]<<", "<<m_cubeMapImages[1]<<", "<<m_cubeMapImages[2]<<", "<<
        m_cubeMapImages[3]<<", "<<m_cubeMapImages[4]<<", "<<m_cubeMapImages[5]<<std::endl;

        
        m_cubeMap = new osg::TextureCubeMap;
        
        unsigned char* data = new unsigned char[4];
        data[0] = 0;//255;
        data[1] = 0;//255;
        data[2] = 0;//255;
        data[3] = 255;
        //Generate the texture
        osg::ref_ptr<osg::Image> whiteImage = new osg::Image;
        whiteImage->setImage( 1, 1, 1,								// 1=r? depth perhaps?
                        GL_RGBA,									// internal format
                        GL_RGBA,GL_UNSIGNED_BYTE,// pixelformat, type
                        data,									// data
                        osg::Image::USE_NEW_DELETE,				// mode
                        1 );
        
        if(m_cubeMapImages[0] == "")
        {
					m_cubeMapImages[0] = "whiteNegX.png";
          osgDB::writeImageFile(*whiteImage, m_cubeMapImages[0]);
        }
        
        if(m_cubeMapImages[1] == "")
        {
          m_cubeMapImages[1] = "whiteNegY.png";
          osgDB::writeImageFile(*whiteImage, m_cubeMapImages[1]);
        }
        
        if(m_cubeMapImages[2] == "")
        {
          m_cubeMapImages[2] = "whiteNegZ.png";
          osgDB::writeImageFile(*whiteImage, m_cubeMapImages[2]);
        }
        
        if(m_cubeMapImages[3] == "")
        {
          m_cubeMapImages[3] = "whitePosX.png";
          osgDB::writeImageFile(*whiteImage, m_cubeMapImages[3]);
        }
        
        if(m_cubeMapImages[4] == "")
        {
          m_cubeMapImages[4] = "whitePosY.png";
          osgDB::writeImageFile(*whiteImage, m_cubeMapImages[4]);
        }
        
        if(m_cubeMapImages[5] == "")
        {
          m_cubeMapImages[5] = "whitePosZ.png";
          osgDB::writeImageFile(*whiteImage, m_cubeMapImages[5]);
        }
        
        
        // set up the textures
        osg::ref_ptr<osg::Image> imageNegX = osgDB::readImageFile(m_cubeMapImages[0]);
        osg::ref_ptr<osg::Image> imageNegY = osgDB::readImageFile(m_cubeMapImages[1]);
        osg::ref_ptr<osg::Image> imageNegZ = osgDB::readImageFile(m_cubeMapImages[2]);
        osg::ref_ptr<osg::Image> imagePosX = osgDB::readImageFile(m_cubeMapImages[3]);
        osg::ref_ptr<osg::Image> imagePosY = osgDB::readImageFile(m_cubeMapImages[4]);
        osg::ref_ptr<osg::Image> imagePosZ = osgDB::readImageFile(m_cubeMapImages[5]);
        
        m_cubeMap->setImage(osg::TextureCubeMap::POSITIVE_X, imagePosX);
        m_cubeMap->setImage(osg::TextureCubeMap::NEGATIVE_X, imageNegX);
        m_cubeMap->setImage(osg::TextureCubeMap::POSITIVE_Y, imagePosY);
        m_cubeMap->setImage(osg::TextureCubeMap::NEGATIVE_Y, imageNegY);
        m_cubeMap->setImage(osg::TextureCubeMap::POSITIVE_Z, imagePosZ);
        m_cubeMap->setImage(osg::TextureCubeMap::NEGATIVE_Z, imageNegZ);
        
        m_cubeMap->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
        m_cubeMap->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
        m_cubeMap->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
        m_cubeMap->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
        shader->m_UniformMap[ShaderConstants::UNIFORM_CUBEMAPSAMPLER] = new osg::Uniform(ShaderConstants::UNIFORM_CUBEMAPSAMPLER.c_str(), ShaderConstants::TEXUREUNIT_CUBEMAP);
        stateSet->setTextureAttributeAndModes(ShaderConstants::TEXUREUNIT_CUBEMAP, m_cubeMap, osg::StateAttribute::ON);
      }
    }
    else
    {
      RemoveRedundantTexture(stateSet, ShaderConstants::TEXUREUNIT_CUBEMAP);
    }
    
    if (shader->HasUniformLocation(ShaderConstants::UNIFORM_NORMALMAPSAMPLER))
    {
      if(!m_normalMapImageSequence.empty())
      {
        LoadImageSequenceUniform(shader, stateSet, m_normalMapImageSequence, ShaderConstants::UNIFORM_NORMALMAPSAMPLER, m_normalMap, ShaderConstants::TEXUREUNIT_NORMALMAP);
      }
      else
      {
        LoadTextureUniform(shader, stateSet, m_normal_map_texture_filename, ShaderConstants::UNIFORM_NORMALMAPSAMPLER,
                           m_colour, m_opacity, m_normalMap, ShaderConstants::TEXUREUNIT_NORMALMAP);
      }
    }
    else
    {
      RemoveRedundantTexture(stateSet, ShaderConstants::TEXUREUNIT_NORMALMAP);
    }

    if (shader->HasUniformLocation(ShaderConstants::UNIFORM_MOJO_LIGHTINGWEIGHT))
    {
      shader->m_UniformMap[ShaderConstants::UNIFORM_MOJO_LIGHTINGWEIGHT] = new osg::Uniform(ShaderConstants::UNIFORM_MOJO_LIGHTINGWEIGHT.c_str(), m_mojoLightingWeight);
    }
 
    if (shader->HasUniformLocation(ShaderConstants::UNIFORM_MOJO_IMAGEREFERENCEINTENSITY))
    {
      shader->m_UniformMap[ShaderConstants::UNIFORM_MOJO_IMAGEREFERENCEINTENSITY] = new osg::Uniform(ShaderConstants::UNIFORM_MOJO_IMAGEREFERENCEINTENSITY.c_str(), m_mojoImageReferenceIntensity);
    }
    
    if (shader->HasUniformLocation(ShaderConstants::UNIFORM_MOJO_PAINTREFERENCEINTENSITY))
    {
      shader->m_UniformMap[ShaderConstants::UNIFORM_MOJO_PAINTREFERENCEINTENSITY] = new osg::Uniform(ShaderConstants::UNIFORM_MOJO_PAINTREFERENCEINTENSITY.c_str(), m_mojoPaintReferenceIntensity);
    }
  }

  //-------------------------------------------------------------------------------------------------------------------------
  void Material::LoadTextureUniform(ShaderBase* shader, osg::ref_ptr<osg::StateSet> stateSet,
                                    const std::string& textureFileName, const std::string& uniformName,
                                    const osg::Vec3& defaultColor, float opacity,
                                    osg::ref_ptr<osg::Texture2D> texture, int textureUnit)
  {
    osg::notify(osg::NOTICE)<<"LoadTextureUniform: textureFileName:"<<textureFileName<<", uniformName:"<<uniformName<<std::endl;

    texture = new osg::Texture2D;

    if(textureFileName != "")
    {
      osg::ref_ptr<osg::Image> image = osgDB::readImageFile(textureFileName);
      image->setPixelFormat(GL_RGBA);
      texture->setImage(image);
      texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
      texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
    }
    else
    {
      unsigned int rInd, gInd, bInd;
      rInd = 0; gInd = 1; bInd = 2;
      
      //if(!shader->IsBGRAColorChannels())
      {
        rInd = 2;
        gInd = 1;
        bInd = 0;
      }
      
      //Generate the texture
      unsigned char* data = new unsigned char[4];
      //add color with gamma correction
      data[0] = static_cast<unsigned char>(pow(defaultColor[bInd], 1.0/2.2) * 255.0);
      data[1] = static_cast<unsigned char>(pow(defaultColor[gInd], 1.0/2.2) * 255.0);
      data[2] = static_cast<unsigned char>(pow(defaultColor[rInd], 1.0/2.2) * 255.0);
      data[3] = static_cast<unsigned char>(opacity * 255.0);
      osg::ref_ptr<osg::Image> image = new osg::Image;
      image->setImage( 1, 1, 1,								// 1=r? depth perhaps?
                       GL_RGBA,									// internal format
                       GL_RGBA,GL_UNSIGNED_BYTE,// pixelformat, type
                       data,									// data
                       osg::Image::USE_NEW_DELETE,				// mode
                       1 );									// packing
      std::stringstream ss;
      ss << (int)(data[0])<<(int)(data[1])<<(int)(data[2])<<(int)(data[3])<<".png";
      image->setFileName(ss.str());
      
      osg::notify(osg::NOTICE)<<"texture file not exist: create texture from color: "<<ss.str()<<std::endl;

      // write image to file
      osgDB::writeImageFile(*image, ss.str());
      
      image->setWriteHint(osg::Image::EXTERNAL_FILE);
      texture->setImage(image);
      
      texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
      texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
      texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::NEAREST);
      texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::NEAREST);
    }
    
    texture->setResizeNonPowerOfTwoHint(false);
    shader->m_UniformMap[uniformName] = new osg::Uniform(uniformName.c_str(), textureUnit);
    stateSet->setTextureAttributeAndModes(textureUnit, texture, osg::StateAttribute::ON);
  }
  
  //-------------------------------------------------------------------------------------------------------------------------
  void Material::LoadImageSequenceUniform(ShaderBase* shader, osg::ref_ptr<osg::StateSet> stateSet,
                                          const std::vector<std::string>& textureFileNames, const std::string& uniformName,
                                          osg::ref_ptr<osg::Texture2D> texture, int textureUnit)
  {
    osg::ref_ptr<osg::ImageSequence> imageSequence = new osg::ImageSequence;
    //imageSequence->setMode(osg::ImageSequence::PAGE_AND_RETAIN_IMAGES);
    imageSequence->setMode(osg::ImageSequence::PAGE_AND_DISCARD_USED_IMAGES);

    double length = -1.0;
    double fps = m_animationFPS;
    
    for(size_t i = 0; i < textureFileNames.size(); i++)
    {
      osg::notify(osg::NOTICE)<<"LoadImageSequenceUniform: "<<textureFileNames[i]<<std::endl;
      imageSequence->addImageFile(textureFileNames[i]);
    }
    
    if(length > 0.0)
    {
      imageSequence->setLength(length);
    }
    else
    {
      unsigned int maxNum = imageSequence->getNumImageData();
      imageSequence->setLength(double(maxNum)*(1.0/fps));
      osg::notify(osg::NOTICE)<<"image sequence length: "<<m_animationFPS<<", "<<maxNum<<", "<<double(maxNum)*(1.0/fps)<<std::endl;
      
    }
    
    //start play image sequence
    if(m_shouldPlayAnimation)
    {
      imageSequence->play();
    }
    
    imageSequence->setLoopingMode(m_animationLoopingMode);
    
    texture = new osg::Texture2DImageSequence;
    
    texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
    texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
    texture->setWrap(osg::Texture::WRAP_R,osg::Texture::REPEAT);
    texture->setResizeNonPowerOfTwoHint(false);
    (dynamic_cast<osg::Texture2DImageSequence*>(texture.get()))->setImageSequence(imageSequence.get());

    shader->m_UniformMap[uniformName] = new osg::Uniform(uniformName.c_str(), textureUnit);
    stateSet->setTextureAttributeAndModes(textureUnit, texture, osg::StateAttribute::ON);
    
    //osgDB::Registry::instance()->writeObject(*imageSequence,"imageSequence.osgt",osgDB::Registry::instance()->getOptions());
  }
  
  //-------------------------------------------------------------------------------------------------------------------------
  void Material::RemoveRedundantTexture(osg::ref_ptr<osg::StateSet> stateSet, int textureUnit)
  {
    stateSet->removeTextureAttribute(textureUnit, osg::StateAttribute::TEXTURE);
  }

}

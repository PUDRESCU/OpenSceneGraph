#pragma once

#include <osg/Group>
#include <osg/Notify>
#include <osg/Vec3>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/BlendFunc>
#include <osg/Timer>
#include <osg/Material>
#include <osg/TextureCubeMap>
#include <osg/Texture2D>
#include <osg/ImageSequence>

//-------------------------------------------------------------------------------------------------------------------------
namespace osg {
  
  class MaterialParameterNode : public osg::Node
  {
  public:
    MaterialParameterNode();
    
    MaterialParameterNode(const MaterialParameterNode& copy,
                          const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
    
    META_Node(osg, MaterialParameterNode)
    
    void setSpecularGlowPower(float value) {m_specularGlowPower = value;}
    float getSpecularGlowPower() const {return m_specularGlowPower;}
    
    void setSpecularGlowFallOff(float value) {m_specularGlowFallOff = value;}
    float getSpecularGlowFallOff() const {return m_specularGlowFallOff;}
    
    void setType( const  std::string & type ) { m_type = type; }
    inline const std::string& getType() const { return m_type; }
    
    void setName( const  std::string & name ) { m_name = name; }
    inline const std::string& getName() const { return m_name; }
    
    void setTransparencyTextureFileName( const  std::string & fileName ) { m_transparencyTextureFileName = fileName; }
    inline const std::string& getTransparencyTextureFileName() const { return m_transparencyTextureFileName; }
    
    void setCubemapNegXImageName( const  std::string & fileName ) { m_cubemapNegXImageName = fileName; }
    inline const std::string& getCubemapNegXImageName() const { return m_cubemapNegXImageName; }
    
    void setCubemapNegYImageName( const  std::string & fileName ) { m_cubemapNegYImageName = fileName; }
    inline const std::string& getCubemapNegYImageName() const { return m_cubemapNegYImageName; }
    
    void setCubemapNegZImageName( const  std::string & fileName ) { m_cubemapNegZImageName = fileName; }
    inline const std::string& getCubemapNegZImageName() const { return m_cubemapNegZImageName; }
    
    void setCubemapPosXImageName( const  std::string & fileName ) { m_cubemapPosXImageName = fileName; }
    inline const std::string& getCubemapPosXImageName() const { return m_cubemapPosXImageName; }
    
    void setCubemapPosYImageName( const  std::string & fileName ) { m_cubemapPosYImageName = fileName; }
    inline const std::string& getCubemapPosYImageName() const { return m_cubemapPosYImageName; }
    
    void setCubemapPosZImageName( const  std::string & fileName ) { m_cubemapPosZImageName = fileName; }
    inline const std::string& getCubemapPosZImageName() const { return m_cubemapPosZImageName; }
    
    void setDiffuseTextureFileName( const  std::string & fileName ) { m_diffuseTextureFileName = fileName; }
    inline const std::string& getDiffuseTextureFileName() const { return m_diffuseTextureFileName; }
    
    void setReflectionTextureFileName( const  std::string & fileName ) { m_reflectionTextureFileName = fileName; }
    inline const std::string& getReflectionTextureFileName() const { return m_reflectionTextureFileName; }
    
    void setEmissiveTextureFileName( const  std::string & fileName ) { m_emissiveTextureFileName = fileName; }
    inline const std::string& getEmissiveTextureFileName() const { return m_emissiveTextureFileName; }
    
    void setSpecularTextureFileName( const  std::string & fileName ) { m_specularTextureFileName = fileName; }
    inline const std::string& getSpecularTextureFileName() const { return m_specularTextureFileName; }
    
    void setAmbientTextureFileName( const  std::string & fileName ) { m_ambientTextureFileName = fileName; }
    inline const std::string& getAmbientTextureFileName() const { return m_ambientTextureFileName; }
    
    void setDiffuseImageSequence( const std::vector<std::string> & fileNames ) { m_diffuseImageSequence = fileNames; }
    inline const std::vector<std::string>& getDiffuseImageSequence() const { return m_diffuseImageSequence; }
    
    void setNormalMapTextureFileName( const  std::string & fileName ) { m_normalMapTextureFileName = fileName; }
    inline const std::string& getNormalMapTextureFileName() const { return m_normalMapTextureFileName; }
    
    void setNormalMapImageSequence( const std::vector<std::string> & fileNames ) { m_normalMapImageSequence = fileNames; }
    inline const std::vector<std::string>& getNormalMapImageSequence() const { return m_normalMapImageSequence; }
    
    void setShouldPlayAnimation( bool value) { m_shouldPlayAnimation = value; }
    inline bool getShouldPlayAnimation() const { return m_shouldPlayAnimation; }
    
    void setLoopingMode(osg::ImageSequence::LoopingMode mode) { m_animationLoopingMode = mode; }
    inline osg::ImageSequence::LoopingMode getLoopingMode() const { return m_animationLoopingMode; }
    
    void setAnimationFPS(float fps) { m_animationFPS = fps; }
    inline float getAnimationFPS() const { return m_animationFPS; }
    
    void setMojoLightingWeight(float w) { m_mojoLightingWeight = w; }
    inline float getMojoLightingWeight() const { return m_mojoLightingWeight; }
    
    void setMojoPaintReferenceIntensity(float i) { m_mojoPaintReferenceIntensity = i; }
    inline float getMojoPaintReferenceIntensity() const { return m_mojoPaintReferenceIntensity; }
    
    void setMojoImageReferenceIntensity(float i) { m_mojoImageReferenceIntensity = i; }
    inline float getMojoImageReferenceIntensity() const { return m_mojoImageReferenceIntensity; }
    
  protected:
    std::string m_type;
    std::string m_name;
    float m_specularGlowPower;
    float m_specularGlowFallOff;
    std::string m_transparencyTextureFileName;
    std::string m_diffuseTextureFileName;
    std::string m_normalMapTextureFileName;
    std::string m_reflectionTextureFileName;
    std::string m_emissiveTextureFileName;
    std::string m_ambientTextureFileName;
    std::string m_specularTextureFileName;

    std::string m_cubemapNegXImageName;
    std::string m_cubemapNegYImageName;
    std::string m_cubemapNegZImageName;
    std::string m_cubemapPosXImageName;
    std::string m_cubemapPosYImageName;
    std::string m_cubemapPosZImageName;
    
    std::vector<std::string> m_diffuseImageSequence;
    std::vector<std::string> m_normalMapImageSequence;
    bool m_shouldPlayAnimation;
    osg::ImageSequence::LoopingMode m_animationLoopingMode;
    float m_animationFPS;
    float m_mojoLightingWeight;
    float m_mojoPaintReferenceIntensity;
    float m_mojoImageReferenceIntensity;
  };
  
}

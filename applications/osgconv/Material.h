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


namespace ImageMetrics
{
  class ShaderBase;
  //-------------------------------------------------------------------------------------------------------------------------

  class Material
  {
  public:
    Material();

    ~Material();


    const std::string &GetType() const { return m_type; }
    void SetType(const std::string &type) { m_type= type; }

    const std::string &GetName() const { return m_name; }
    void SetName(const std::string &name) { m_name= name; }
    
    const std::string &GetDiffuseTextureFileName() const { return m_diffuse_texture_filename; }
    void SetDiffuseTextureFileName(const std::string &name) { m_diffuse_texture_filename= name; }
    
    const std::string &GetReflectionTextureFileName() const { return m_reflection_texture_filename; }
    void SetReflectionTextureFileName(const std::string &name) { m_reflection_texture_filename= name; }
    
    const std::string &GetEmissiveTextureFileName() const { return m_emissive_texture_filename; }
    void SetEmissiveTextureFileName(const std::string &name) { m_emissive_texture_filename= name; }
    
    const std::string &GetAmbientTextureFileName() const { return m_ambient_texture_filename; }
    void SetAmbientTextureFileName(const std::string &name) { m_ambient_texture_filename= name; }
    
    void setDiffuseImageSequence( const std::vector<std::string> & fileNames ) { m_diffuseImageSequence = fileNames; }
    inline const std::vector<std::string>& getDiffuseImageSequence() const { return m_diffuseImageSequence; }
    
    const std::string &GetBackgroundTextureFileName() const { return m_background_texture_filename; }
    void SetBackgroundTextureFileName(const std::string &name) { m_background_texture_filename= name; }
    
    const std::string &GetTransparencyTextureFileName() const { return m_transparency_texture_filename; }
    void SetTransparencyTextureFileName(const std::string &name) { m_transparency_texture_filename= name; }
    
    const std::string &GetSpecularTextureFileName() const { return m_specular_texture_filename; }
    void SetSpecularTextureFileName(const std::string &name) { m_specular_texture_filename= name; }
    
    osg::ref_ptr<osg::Texture2D> GetTexture() const { return m_texture; }
    void SetTexture(const std::string &filename);
    void SetTexture(const osg::ref_ptr<osg::Texture2D> texture) { m_texture = texture; }

    osg::ref_ptr<osg::Texture2D> GetBackgroundTexture() const { return m_background_texture; }
    void SetBackgroundTexture(const std::string &filename);
    void SetBackgroundTexture(const osg::ref_ptr<osg::Texture2D> texture) { m_background_texture = texture; }
    
    const std::string &GetNormalMapTextureFileName() const { return m_normal_map_texture_filename; }
    void SetNormalMapTextureFileName(const std::string &name) { m_normal_map_texture_filename= name; }
    void setNormalMapImageSequence( const std::vector<std::string> & fileNames ) { m_normalMapImageSequence = fileNames; }
    inline const std::vector<std::string>& getNormalMapImageSequence() const { return m_normalMapImageSequence; }
    
    
//    void SetNormalMap(const std::string &filename);
//    void SetNormalMap(const IMImagePtr image);
//    void ClearNormalMap();

    osg::ref_ptr<osg::TextureCubeMap> GetCubeMap() const { return m_cubeMap; }
    void SetCubeMap(const std::vector<std::string> &filenames);
    void ClearCubeMap();

    void SetColour(const osg::Vec3 &rgb) {m_colour = rgb;}
    void SetAmbient(const osg::Vec3 &rgb) {m_ambient = rgb;}
    void SetDiffuse(const osg::Vec3 &rgb) {m_diffuse = rgb;}
    void SetSpecular(const osg::Vec3 &rgb) { m_specular= rgb; }
    void SetOpacity(float opacity);
    void SetSpecularPower(float sp) { m_specular_power = sp; }
    void SetReflection(const osg::Vec3 &rgb) {m_reflection = rgb;}
    void SetIridescence(const osg::Vec3 &rgb) {m_iridescence = rgb;}
    void SetSpecularFallOff(float sp) {m_specular_falloff = sp;}
    void SetSpecularGlowPower(float sp) {m_specularglow_power = sp;}
    void SetSpecularGlowFallOff(float sp) {m_specularglow_falloff = sp;}
    void SetReflectivity(float r) {m_reflectivity = r;}
    float GetReflectivity() const {return m_reflectivity;}

    void SetRefractiveIndex(float r) {m_refractiveindex = r;}
    void SetTransparencyColour(const osg::Vec3 &rgb) {m_transparency_color = rgb;}
    void SetBackgroundColour(const osg::Vec3 &rgb) {m_background_color = rgb;}

    void SetShouldPlayAnimation( bool value) { m_shouldPlayAnimation = value; }
    inline bool GetShouldPlayAnimation() const { return m_shouldPlayAnimation; }
    
    void SetLoopingMode(osg::ImageSequence::LoopingMode mode) { m_animationLoopingMode = mode; }
    inline osg::ImageSequence::LoopingMode GetLoopingMode() const { return m_animationLoopingMode; }
  
    void SetAnimationFPS(float fps) { m_animationFPS = fps; }
    inline float GetAnimationFPS() const { return m_animationFPS; }
    
    void SetMojoLightingWeight(float w) { m_mojoLightingWeight = w; }
    inline float GetMojoLightingWeight() const { return m_mojoLightingWeight; }
    
    void SetMojoPaintReferenceIntensity(float i) { m_mojoPaintReferenceIntensity = i; }
    inline float GetMojoPaintReferenceIntensity() const { return m_mojoPaintReferenceIntensity; }
    
    void SetMojoImageReferenceIntensity(float i) { m_mojoImageReferenceIntensity = i; }
    inline float GetMojoImageReferenceIntensity() const { return m_mojoImageReferenceIntensity; }
    
    virtual void PrepareToRender(ShaderBase* shader, osg::ref_ptr<osg::StateSet> stateSet);

  protected:

    virtual void Destroy();

  private:
    void LoadTextureUniform(ShaderBase* shader, osg::ref_ptr<osg::StateSet> stateSet,
                            const std::string& textureFileName, const std::string& uniformName,
                            const osg::Vec3& defaultColor, float opacity,
                            osg::ref_ptr<osg::Texture2D> texture, int textureUnit);
    
    void LoadImageSequenceUniform(ShaderBase* shader, osg::ref_ptr<osg::StateSet> stateSet,
                                  const std::vector<std::string>& textureFileNames, const std::string& uniformName,
                                  osg::ref_ptr<osg::Texture2D> texture, int textureUnit);
    
    void RemoveRedundantTexture(osg::ref_ptr<osg::StateSet> stateSet, int textureUnit);

  protected:

    std::string m_type;
    std::string m_name;
    
    std::string m_diffuse_texture_filename;
    std::string m_reflection_texture_filename;
    std::string m_emissive_texture_filename;
    std::string m_ambient_texture_filename;

    std::string m_background_texture_filename;
    std::string m_transparency_texture_filename;
    std::string m_specular_texture_filename;
    std::string m_normal_map_texture_filename;
    
    // Texture
    osg::ref_ptr<osg::Texture2D> m_texture;

    // Texture
    osg::ref_ptr<osg::Texture2D> m_background_texture;
    
    // Normal Map
    osg::ref_ptr<osg::Texture2D> m_normalMap;

    //CubeMap
    osg::ref_ptr<osg::TextureCubeMap> m_cubeMap;

    // Opacity
    osg::ref_ptr<osg::Texture2D> m_transparency_texture;

    //Specularity
    osg::ref_ptr<osg::Texture2D> m_specular_texture;

    osg::Vec3 m_colour;
    osg::Vec3 m_diffuse;
    osg::Vec3 m_ambient;
    osg::Vec3 m_specular;
    osg::Vec3 m_reflection;
    osg::Vec3 m_iridescence;
    osg::Vec3 m_transparency_color;
    osg::Vec3 m_background_color;

    float m_opacity;
    float m_specular_power;
    float m_specular_falloff;
    float m_specularglow_power;
    float m_specularglow_falloff;
    float m_reflectivity;
    float m_refractiveindex;
    
    std::vector<std::string> m_cubeMapImages;
    std::vector<std::string> m_diffuseImageSequence;
    std::vector<std::string> m_normalMapImageSequence;

    bool m_shouldPlayAnimation;
    osg::ImageSequence::LoopingMode m_animationLoopingMode;
    float m_animationFPS;
    float m_mojoLightingWeight;
    float m_mojoPaintReferenceIntensity;
    float m_mojoImageReferenceIntensity;
  };

  //-------------------------------------------------------------------------------------------------------------------------

}


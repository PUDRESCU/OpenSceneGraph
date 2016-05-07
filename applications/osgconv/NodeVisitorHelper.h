#pragma once

#include <stdio.h>
#include <algorithm>
#include <string>
#if defined(WIN32)
#include <direct.h>
#include <io.h>   // For access().
#else
#include <unistd.h>
#endif
#include <sys/types.h>  // For stat().
#include <sys/stat.h>   // For stat().
#include <cstdlib>

#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>
#include <osg/Group>
#include <osg/Notify>
#include <osg/Vec3>
#include <osg/ProxyNode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/TextureCubeMap>

#include <osg/BlendFunc>
#include <osg/Timer>
#include <osg/Material>
#include <osg/ImageSequence>

#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>
#include <osgDB/ReaderWriter>
#include <osgDB/PluginQuery>

#include <osgUtil/Optimizer>
#include <osgUtil/Simplifier>
#include <osgUtil/SmoothingVisitor>
#include <osgUtil/TangentSpaceGenerator>

#include <osgViewer/GraphicsWindow>
#include <osgViewer/Version>
#include <osgAnimation/AnimationManagerBase>
#include <osgAnimation/Bone>
#include <osgAnimation/BoneMapVisitor>
#include <osgAnimation/BasicAnimationManager>
#include <osgAnimation/Skeleton>
#include <iostream>

#include "ShaderBase.h"
#include "Light.h"
#include "Material.h"
#include "ShaderManager.h"
#include "MaterialParameterNode.h"
#include "ShaderConstants.h"
#include "TriggerParameterNode.h"
#include "GeodePropertyParameterNode.h"

// Get attribute by name
class GetAttributeByNameVisitor : public osg::NodeVisitor
{
public:
  GetAttributeByNameVisitor(const std::string &name):osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN), m_name(name)
  {
  }
  
  virtual void apply(osg::Node& node)
  {
    traverse(node);
  }
  
  virtual void apply(osg::Geode& geode)
  {
    for (int i = 0; i< geode.getNumDrawables(); i++)
    {
      osg::Drawable* dr = geode.getDrawable(i);
      
      osg::StateSet* stateSet =  dr->getStateSet();
      
      for(int j = 0; j < stateSet->getAttributeList().size(); j++)
      {
        osg::ref_ptr<osg::Material> mat = dynamic_cast<osg::Material*>(stateSet->getAttribute(osg::StateAttribute::MATERIAL));
        if(mat.get() && (mat->getName() == m_name))
        {
          m_result = dr;
          return;
        }
      }
    }
    traverse(geode);
  }
 
  virtual void apply(osg::Transform& node)
  {
    apply((osg::Node&)node);
  }
 
  osg::ref_ptr<osg::Drawable> getResult(){return m_result;}
  
private:
  std::string m_name;
  osg::ref_ptr<osg::Drawable> m_result;
};

class FindNodeVisitor : public osg::NodeVisitor
{
public:
  
  FindNodeVisitor(const std::string &name):osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN), m_name(name)
  {
  }
  
  virtual void apply(osg::Node& node)
  {
    if(node.getName() == m_name)
    {
      m_result.push_back(&node);
    }
    traverse(node);
  }
  
  void apply(osg::Transform &node)
  {
    apply((osg::Node&)node);
  }
  
  void setNameToFind(const std::string &searchName)
  {
    m_name = searchName;
    m_result.clear();
  }
  

  osg::Node* getFirst()
  {
    return *(m_result.begin());
  }
  
  typedef std::vector<osg::Node*> nodeListType;
  
  nodeListType& getNodeList() { return m_result; }
  
private:
  std::string m_name;

  nodeListType m_result;
};

class FindSwitchNodeVisitor : public osg::NodeVisitor
{
public:
  
  FindSwitchNodeVisitor(const std::string &name):osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN), m_name(name)
  {
  }
  
  virtual void apply(osg::Node& node)
  {
    traverse(node);
  }
  
  void apply(osg::Transform &node)
  {
    traverse(node);
  }
  
  virtual void apply(osg::Switch& node)
  {
    if(node.getName() == m_name)
    {
      m_result.push_back(&node);
    }
    traverse(node);
  }
  
  void setNameToFind(const std::string &searchName)
  {
    m_name = searchName;
    m_result.clear();
  }
  
  
  osg::Node* getFirst()
  {
    return *(m_result.begin());
  }
  
  typedef std::vector<osg::Node*> nodeListType;
  
  nodeListType& getNodeList() { return m_result; }
  
private:
  std::string m_name;
  
  nodeListType m_result;
};

// Find geode node with name
class FindGeodeNodeVisitor : public osg::NodeVisitor
{
public:
  
  FindGeodeNodeVisitor(const std::string &name):osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN), m_name(name)
  {
  }
  
  virtual void apply(osg::Node& node)
  {
    traverse(node);
  }
  
  virtual void apply(osg::Geode& geode)
  {
    if(geode.getName() == m_name)
    {
      m_result.push_back(&geode);
    }
  }
  
  void setNameToFind(const std::string &searchName)
  {
    m_name = searchName;
    m_result.clear();
  }
  
  
  osg::Node* getFirst()
  {
    if(m_result.empty())
      return NULL;
    else
      return *(m_result.begin());
  }
  
  typedef std::vector<osg::Node*> nodeListType;
  
  nodeListType& getNodeList() { return m_result; }
  
private:
  std::string m_name;
  
  nodeListType m_result;
};

// Find Geode
class FindGeodeVisitor : public osg::NodeVisitor
{
public:
  FindGeodeVisitor():osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
  {
  }
  
  virtual void apply(osg::Node& node)
  {
    traverse(node);
  }
  
  virtual void apply(osg::Geode& geode)
  {
    osg::ref_ptr<osg::Geode> geodePtr = &geode;
    m_geodes.push_back(geodePtr);
  }
  
  virtual void apply(osg::Transform& node)
  {
    apply((osg::Node&)node);
  }
  
  std::vector<osg::ref_ptr<osg::Geode> > getGeodes()
  {
    return m_geodes;
  }
  
private:
  std::vector<osg::ref_ptr<osg::Geode> > m_geodes;
  
};

struct AnimationManagerFinder : public osg::NodeVisitor
{
  osg::ref_ptr<osgAnimation::BasicAnimationManager> _am;
  AnimationManagerFinder() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
  void apply(osg::Node& node) {
    if (_am.valid())
      return;
    if (node.getUpdateCallback()) {
      _am = dynamic_cast<osgAnimation::BasicAnimationManager*>(node.getUpdateCallback());
      return;
//      if (b) {
//        _am = new osgAnimation::BasicAnimationManager(*b);
//        return;
//      }
    }
    traverse(node);
  }
};

struct AnimationManagerBaseFinder : public osg::NodeVisitor
{
  osg::ref_ptr<osgAnimation::AnimationManagerBase> _am;
  AnimationManagerBaseFinder() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
  void apply(osg::Node& node) {
    if (_am.valid())
      return;
    if (node.getUpdateCallback()) {
      _am = dynamic_cast<osgAnimation::AnimationManagerBase*>(node.getUpdateCallback());
      return;
    }
    traverse(node);
  }
};

// Replace standard light
class ReplaceLightVisitor : public osg::NodeVisitor
{
public:
  ReplaceLightVisitor():osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
  {
  }
  
  virtual void apply(osg::Node& node)
  {
    traverse(node);
  }
  
  virtual void apply(osg::LightSource& node)
  {
    osg::Matrix transform = osg::computeLocalToWorld(node.getParentalNodePaths()[0]);
    ImageMetrics::Light oneLight;
    
    std::string lowerStr;
    // light source name is in the parent node
    std::string name = node.getParent(0)->getName();
    lowerStr.resize(name.size());
    
    std::transform(name.begin(), name.end(), lowerStr.begin(), ::tolower);
    osg::notify(osg::NOTICE)<<"Find light: "<<name<<std::endl;
    osg::ref_ptr<osg::Light> osgLight = node.getLight();

    // Directional light
    if(lowerStr.find("directional") != std::string::npos || lowerStr.find("ambient") != std::string::npos || lowerStr.find("pointlight") != std::string::npos)
    {
      osg::notify(osg::NOTICE)<<"Add light: "<<name<<std::endl;

      const osg::Vec4& a = osgLight->getAmbient();
      oneLight.SetAmbient(osg::Vec3(a.x(), a.y(), a.z()));
      
      const osg::Vec4& d = osgLight->getDiffuse();
      oneLight.SetDiffuse(osg::Vec3(d.x(), d.y(), d.z()));
      
      const osg::Vec4& s = osgLight->getSpecular();
      oneLight.SetSpecular(osg::Vec3(s.x(), s.y(), s.z()));
      
      float p = 1.0f;
      oneLight.SetPower(p);

      const osg::Vec4& pos = osgLight->getPosition();
      oneLight.SetPos(osg::Vec4(transform(3, 0), transform(3, 1), transform(3, 2), pos.w()));
      m_lights.push_back(oneLight);
      m_lightParentNodes.push_back(node.getParent(0));
    }

    traverse(node);
  }
  
  virtual void apply(osg::Transform& node)
  {
    apply((osg::Node&)node);
  }
  
  std::vector<ImageMetrics::Light>& getLights()
  {
    return m_lights;
  }
  
  std::vector<osg::Group*>& getParentNodes()
  {
    return m_lightParentNodes;
  }
  
private:
  std::vector<ImageMetrics::Light> m_lights;
  std::vector<osg::Group*> m_lightParentNodes;
};

// this is a visitor class that prepares all geometries in a subgraph
// by calling prepareGeometry() which in turn generates tangent-space
// basis vectors
class TsgVisitor: public osg::NodeVisitor
{
public:
  TsgVisitor(): osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
  
  virtual void apply(osg::Geode& geode)
  {
    for (unsigned i=0; i<geode.getNumDrawables(); ++i)
    {
      osg::Geometry* geo = dynamic_cast<osg::Geometry* >(geode.getDrawable(i));
      if (geo)
      {
        osg::ref_ptr<osgUtil::TangentSpaceGenerator> tsg = new osgUtil::TangentSpaceGenerator;
        // normal map texture uv is same as diffuse texture uv
        tsg->generate(geo, 0);
        if (!geo->getVertexAttribArray(ImageMetrics::ShaderConstants::ATTRIBUTE_VERTEX_UNIT_TANGENT_XYZ_MODELSPACE))
        {
          geo->setVertexAttribArray(ImageMetrics::ShaderConstants::ATTRIBUTE_VERTEX_UNIT_TANGENT_XYZ_MODELSPACE, tsg->getTangentArray());
        }
        if (!geo->getVertexAttribArray(ImageMetrics::ShaderConstants::ATTRIBUTE_VERTEX_UNIT_BITANGENT_XYZ_MODELSPACE))
        {
          geo->setVertexAttribArray(ImageMetrics::ShaderConstants::ATTRIBUTE_VERTEX_UNIT_BITANGENT_XYZ_MODELSPACE, tsg->getBinormalArray());
        }
      }
    }
    osg::NodeVisitor::apply(geode);
  }
  
  virtual void apply(osg::Node& node)
  {
    traverse(node);
  }
private:
};




// Replace standard material with Phong-shader
class ReplaceMaterialVisitor : public osg::NodeVisitor
{
public:
  ReplaceMaterialVisitor(osg::ref_ptr<osg::Group> params, const std::string& srcFileName):osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
  {
    m_params = params;
    m_srcFileName = srcFileName;
  }
  
  virtual void apply(osg::Node& node)
  {
    traverse(node);
  }
  
  virtual void apply(osg::Geode& geode)
  {
    for (int i = 0; i< geode.getNumDrawables(); i++)
    {
      osg::Drawable* dr = geode.getDrawable(i);
      
      osg::StateSet* stateSet =  dr->getStateSet();
      
      for(int j = 0; j < stateSet->getAttributeList().size(); j++)
      {
        osg::ref_ptr<osg::Material> material = dynamic_cast<osg::Material*>(stateSet->getAttribute(osg::StateAttribute::MATERIAL));
        if(material)
        {
          osg::notify(osg::NOTICE)<<"------------- "<< material->getName()<<std::endl;
          ImageMetrics::Material oneMaterial;

          std::string lowerStrMaterial;
          std::string name = material->getName();
          lowerStrMaterial.resize(name.size());
          std::transform(name.begin(), name.end(), lowerStrMaterial.begin(), ::tolower);
          
          osg::Vec4 ambient = material->getAmbient(osg::Material::FRONT);
          osg::Vec4 diffuse = material->getDiffuse(osg::Material::FRONT);
          osg::Vec4 specular = material->getSpecular(osg::Material::FRONT);
          osg::Vec4 emission = material->getEmission(osg::Material::FRONT);
          float shiness = material->getShininess(osg::Material::FRONT);
          // Restore to maya's original cosine power
          shiness = pow(2, (shiness * 5.0 / 64.0));
          
          osg::Vec4 transparent = material->getTransparent(osg::Material::FRONT);
          float transparencyFactor = material->getTransparencyFactor(osg::Material::FRONT);
          osg::Vec4 reflection = material->getReflection(osg::Material::FRONT);
          float reflectionFactor = material->getReflectionFactor(osg::Material::FRONT);
          
          oneMaterial.SetName(material->getName());
//          oneMaterial.SetType("phong");
//          
//          // Blinn shader
//          if(lowerStrMaterial.find("blinn") != std::string::npos)
//          {
//            oneMaterial.SetType("blinn");
//          }
          oneMaterial.SetColour(osg::Vec3(diffuse.r(), diffuse.g(), diffuse.b()));
          oneMaterial.SetAmbient(osg::Vec3(ambient.r(), ambient.g(), ambient.b()));
          oneMaterial.SetDiffuse(osg::Vec3(diffuse.r(), diffuse.g(), diffuse.b()));
          oneMaterial.SetSpecular(osg::Vec3(specular.r(), specular.g(), specular.b()));
          oneMaterial.SetReflectivity(reflectionFactor);
          oneMaterial.SetReflection(osg::Vec3(reflection.r(), reflection.g(), reflection.b()));

          if(reflectionFactor > 0)
          {
//            oneMaterial.SetType("reflective");
            oneMaterial.SetIridescence(osg::Vec3(1, 1, 1));
            if(reflection.r()+reflection.g()+reflection.b() <= 0)
            {
              oneMaterial.SetReflection(osg::Vec3(1, 1, 1));
            }
          }
          
          osg::StateSet::TextureAttributeList textList = stateSet->getTextureAttributeList();
          if(textList.size() != 0)
          {
            for(int uid = 0; uid < 10; uid++)
            {
              osg::ref_ptr<osg::Texture2D> tex = dynamic_cast<osg::Texture2D*>( stateSet->getTextureAttribute(uid, osg::StateAttribute::TEXTURE ) );
              if (tex)
              {
                std::string texFileName = tex->getImage()->getFileName();
                //texFileName = texFileName.substr(texFileName.find_last_of("/\\") + 1);
                texFileName = osgDB::getPathRelative(m_srcFileName, texFileName);
                
                // There is a '/' in the beginning of returned relative path
                if(texFileName[0] == '/')
                {
                  texFileName = texFileName.substr(1);
                }

                switch(uid)
                {
                  case 0: // diffuse texture
                    osg::notify(osg::NOTICE)<<"diffuse texture: "<<texFileName<<std::endl;
                    
                    oneMaterial.SetDiffuseTextureFileName(texFileName);
                    break;
                  case 1: // opacity texture
                    osg::notify(osg::NOTICE)<<"opacity texture: "<<texFileName<<std::endl;
                    oneMaterial.SetTransparencyTextureFileName(texFileName);
                    break;
                  case 2:
                    osg::notify(osg::NOTICE)<<"Reflection texture: "<<texFileName<<std::endl;
                    oneMaterial.SetReflectionTextureFileName(texFileName);
                    break;
                  case 3:
                    osg::notify(osg::NOTICE)<<"emissive texture: "<<texFileName<<std::endl;
                    oneMaterial.SetEmissiveTextureFileName(texFileName);
                    break;
                  case 4:
                    osg::notify(osg::NOTICE)<<"ambient texture: "<<texFileName<<std::endl;
                    oneMaterial.SetAmbientTextureFileName(texFileName);
                    break;
                  case 5:
                    osg::notify(osg::NOTICE)<<"normal map texture: "<<texFileName<<std::endl;
                    oneMaterial.SetNormalMapTextureFileName(texFileName);
                    break;
                  case 6:
                    osg::notify(osg::NOTICE)<<"specular texture: "<<texFileName<<std::endl;
                    oneMaterial.SetSpecularTextureFileName(texFileName);
                    break;
                  default:
                    osg::notify(osg::NOTICE)<<"other texture: "<<uid<<", "<<texFileName<<std::endl;

                    break;
                }
              }
            }
          }


          if(m_params)
          {
            // get material parameters
            osg::Group* matParams = NULL;
            for(int k = 0; k < m_params->getNumChildren(); k++)
            {
              if(m_params->getChild(k)->getName() == "material")
              {
                matParams = dynamic_cast<osg::Group*>(m_params->getChild(k));
                break;
              }
            }
            
            if(matParams != NULL)
            {
              for(int k = 0; k < matParams->getNumChildren(); k++)
              {
                osg::ref_ptr<osg::MaterialParameterNode> oneParam = dynamic_cast<osg::MaterialParameterNode*>(matParams->getChild(k));
                
                osg::notify(osg::NOTICE)<<"check material: m type:"<<oneMaterial.GetType()<<", p type:"<<oneParam->getType()<<", m name:"<<oneMaterial.GetName()<<", p name:"<<oneParam->getName()<<std::endl;

                // Find matched parameter node
                if(oneParam != NULL &&
                   ((oneMaterial.GetType() == oneParam->getType() && (oneParam->getName() == "" || oneMaterial.GetName() == oneParam->getName())) ||
                   (oneMaterial.GetName() == oneParam->getName())))
                 {
                   osg::notify(osg::NOTICE)<<"set material type: "<<oneParam->getType()<<std::endl;
                   // we only set shader type when it is not default
                   if(oneParam->getType() != "phong")
                   {
                     oneMaterial.SetType(oneParam->getType());
                   }
                   
                   oneMaterial.SetMojoLightingWeight(oneParam->getMojoLightingWeight());
                   oneMaterial.SetMojoPaintReferenceIntensity(oneParam->getMojoPaintReferenceIntensity());
                   oneMaterial.SetMojoImageReferenceIntensity(oneParam->getMojoImageReferenceIntensity());
                   
                   oneMaterial.SetSpecularGlowPower(oneParam->getSpecularGlowPower());
                   oneMaterial.SetSpecularGlowFallOff(oneParam->getSpecularGlowFallOff());
                   
                   oneMaterial.SetShouldPlayAnimation(oneParam->getShouldPlayAnimation());
                   oneMaterial.SetLoopingMode(oneParam->getLoopingMode());
                   oneMaterial.SetAnimationFPS(oneParam->getAnimationFPS());

                   if(oneParam->getTransparencyTextureFileName() != "")
                   {
                     oneMaterial.SetTransparencyTextureFileName(oneParam->getTransparencyTextureFileName());
                   }
                  
                   if(oneParam->getDiffuseTextureFileName() != "")
                   {
                     oneMaterial.SetDiffuseTextureFileName(oneParam->getDiffuseTextureFileName());
                   }
                  
                   if(oneParam->getReflectionTextureFileName() != "")
                   {
                     oneMaterial.SetReflectionTextureFileName(oneParam->getReflectionTextureFileName());
                   }
                  
                   if(oneParam->getEmissiveTextureFileName() != "")
                   {
                     oneMaterial.SetEmissiveTextureFileName(oneParam->getEmissiveTextureFileName());
                   }
                  
                   if(oneParam->getAmbientTextureFileName() != "")
                   {
                     oneMaterial.SetAmbientTextureFileName(oneParam->getAmbientTextureFileName());
                   }
                  
                   if(oneParam->getSpecularTextureFileName() != "")
                   {
                     oneMaterial.SetSpecularTextureFileName(oneParam->getSpecularTextureFileName());
                   }
                   
                   if(!oneParam->getDiffuseImageSequence().empty())
                   {
                     oneMaterial.setDiffuseImageSequence(oneParam->getDiffuseImageSequence());
                   }
                   
                   if(oneParam->getNormalMapTextureFileName() != "")
                   {
                     oneMaterial.SetNormalMapTextureFileName(oneParam->getNormalMapTextureFileName());
                   }
                   
                   if(!oneParam->getNormalMapImageSequence().empty())
                   {
                     oneMaterial.setNormalMapImageSequence(oneParam->getNormalMapImageSequence());
                   }
                  
                   std::vector<std::string> cubemapImages;
                   cubemapImages.push_back(oneParam->getCubemapNegXImageName());
                   cubemapImages.push_back(oneParam->getCubemapNegYImageName());
                   cubemapImages.push_back(oneParam->getCubemapNegZImageName());
                   cubemapImages.push_back(oneParam->getCubemapPosXImageName());
                   cubemapImages.push_back(oneParam->getCubemapPosYImageName());
                   cubemapImages.push_back(oneParam->getCubemapPosZImageName());
                   oneMaterial.SetCubeMap(cubemapImages);
                   

                }
              }
            }
          }
          
          oneMaterial.SetOpacity(1.0f - transparent.r());
          stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
          stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
//          if(transparent.r() > 0 || oneMaterial.GetTransparencyTextureFileName() != "")
//          {
//            stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
//            stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
//            // For glasses, we should render it last avoid transparency problem
//            if(oneMaterial.GetTransparencyTextureFileName() == "")
//            {
//               stateSet->setRenderBinDetails(100, "RenderBin");
//            }
//            
//            std::string lowerStrGeodeName;
//            std::string geodeName = geode.getName();
//            lowerStrGeodeName.resize(geodeName.size());
//            std::transform(geodeName.begin(), geodeName.end(), lowerStrGeodeName.begin(), ::tolower);
//          
//            // occlusion head mesh
//            if(lowerStrGeodeName.find("occlusionmesh") != std::string::npos)
//            {
//              stateSet->setRenderBinDetails(-1, "RenderBin");
//            }
//          }
//          else
//          {
//            stateSet->setMode(GL_BLEND, osg::StateAttribute::OFF);
//            stateSet->setRenderingHint(osg::StateSet::OPAQUE_BIN);
//          }
          oneMaterial.SetSpecularPower(specular.a());
          oneMaterial.SetSpecularFallOff(shiness);
          
          osg::notify(osg::NOTICE)<<"Material parameter: "<<std::endl;
          osg::notify(osg::NOTICE)<<"ambient: "<<ambient.r()<<", "<<ambient.g()<<", "<<ambient.b()<<", "<<ambient.a()<<std::endl;
          osg::notify(osg::NOTICE)<<"diffuse: "<<diffuse.r()<<", "<<diffuse.g()<<", "<<diffuse.b()<<", "<<diffuse.a()<<std::endl;
          osg::notify(osg::NOTICE)<<"specular: "<<specular.r()<<", "<<specular.g()<<", "<<specular.b()<<", "<<specular.a()<<std::endl;
          osg::notify(osg::NOTICE)<<"emission: "<<emission.r()<<", "<<emission.g()<<", "<<emission.b()<<", "<<emission.a()<<std::endl;
          osg::notify(osg::NOTICE)<<"shiness: "<<shiness<<std::endl;
          osg::notify(osg::NOTICE)<<"transparent: "<<transparent.r()<<", "<<transparent.g()<<", "<<transparent.b()<<", "<<transparent.a()<<std::endl;
          osg::notify(osg::NOTICE)<<"transparencyFactor: "<<transparencyFactor<<std::endl;
          osg::notify(osg::NOTICE)<<"reflection: "<<reflection.r()<<", "<<reflection.g()<<", "<<reflection.b()<<", "<<reflection.a()<<std::endl;
          osg::notify(osg::NOTICE)<<"reflectionFactor: "<<reflectionFactor<<std::endl;
          
          // Get shader
          ImageMetrics::ShaderBase* shader = ImageMetrics::ShaderManager::getInstance().GetShaderByMaterial(oneMaterial);

          if(shader)
          {
            osg::notify(osg::NOTICE)<<"get shader: "<<shader->GetName()<<std::endl;

            // if it is normal mapping then we need to compute tangent and bitangent
            if(shader->IsNormalMapSupport())
            {
              TsgVisitor tsgVisitor;
              geode.accept(tsgVisitor);
            }
            
            // Remove old material
            stateSet->removeAttribute(osg::StateAttribute::MATERIAL);
            
            shader->PrepareToRender(stateSet);
            
            oneMaterial.PrepareToRender(shader, stateSet);
            
            //Add uniforms
            for(std::map<std::string, osg::ref_ptr<osg::Uniform> >::iterator unifomr_itr = shader->m_UniformMap.begin();
                unifomr_itr != shader->m_UniformMap.end(); unifomr_itr++)
            {
              stateSet->addUniform(unifomr_itr->second);
            }
          }
          else
          {
            osg::notify(osg::NOTICE)<<"not find shader: "<<std::endl;
          }
 
        }
      }
    }
    
    traverse(geode);

  }
  
  virtual void apply(osg::Transform& node)
  {
    apply((osg::Node&)node);
  }
  
private:
  osg::ref_ptr<osg::Group> m_params;
  std::string m_srcFileName;
};

// Find updating uniforms
class FindUniformVisitor : public osg::NodeVisitor
{
public:
  FindUniformVisitor():osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
  {
  }
  
  virtual void apply(osg::Node& node)
  {
    traverse(node);
  }
  
  virtual void apply(osg::Geode& geode)
  {
    for (int i = 0; i< geode.getNumDrawables(); i++)
    {
      osg::Drawable* dr = geode.getDrawable(i);
      
      osg::StateSet* stateSet =  dr->getStateSet();
      
      if(stateSet)
      {
        osg::Uniform* mvpUniform = stateSet->getUniform(ImageMetrics::ShaderConstants::UNIFORM_MVP);
        if(mvpUniform)
        {
          if(m_uniformMap.find(ImageMetrics::ShaderConstants::UNIFORM_MVP) == m_uniformMap.end())
          {
            m_uniformMap[ImageMetrics::ShaderConstants::UNIFORM_MVP] = std::vector<osg::ref_ptr<osg::Uniform> >();
          }
          m_uniformMap[ImageMetrics::ShaderConstants::UNIFORM_MVP].push_back(mvpUniform);
        }
        
        osg::Uniform* vUniform = stateSet->getUniform(ImageMetrics::ShaderConstants::UNIFORM_V);
        if(vUniform)
        {
          if(m_uniformMap.find(ImageMetrics::ShaderConstants::UNIFORM_V) == m_uniformMap.end())
          {
            m_uniformMap[ImageMetrics::ShaderConstants::UNIFORM_V] = std::vector<osg::ref_ptr<osg::Uniform> >();
          }
          m_uniformMap[ImageMetrics::ShaderConstants::UNIFORM_V].push_back(vUniform);
          
        }
        
        osg::Uniform* mUniform = stateSet->getUniform(ImageMetrics::ShaderConstants::UNIFORM_M);
        if(mUniform)
        {
          if(m_uniformMap.find(ImageMetrics::ShaderConstants::UNIFORM_M) == m_uniformMap.end())
          {
            m_uniformMap[ImageMetrics::ShaderConstants::UNIFORM_M] = std::vector<osg::ref_ptr<osg::Uniform> >();
          }
          m_uniformMap[ImageMetrics::ShaderConstants::UNIFORM_M].push_back(mUniform);
        }
        
        osg::Uniform* vInvUniform = stateSet->getUniform(ImageMetrics::ShaderConstants::UNIFORM_V_INV);
        if(vInvUniform)
        {
          if(m_uniformMap.find(ImageMetrics::ShaderConstants::UNIFORM_V_INV) == m_uniformMap.end())
          {
            m_uniformMap[ImageMetrics::ShaderConstants::UNIFORM_V_INV] = std::vector<osg::ref_ptr<osg::Uniform> >();
          }
          m_uniformMap[ImageMetrics::ShaderConstants::UNIFORM_V_INV].push_back(vInvUniform);
        }
        
        osg::Uniform* pUniform = stateSet->getUniform(ImageMetrics::ShaderConstants::UNIFORM_P);
        if(pUniform)
        {
          if(m_uniformMap.find(ImageMetrics::ShaderConstants::UNIFORM_P) == m_uniformMap.end())
          {
            m_uniformMap[ImageMetrics::ShaderConstants::UNIFORM_P] = std::vector<osg::ref_ptr<osg::Uniform> >();
          }
          m_uniformMap[ImageMetrics::ShaderConstants::UNIFORM_P].push_back(pUniform);
        }
        
        osg::Uniform* normalMatrixUniform = stateSet->getUniform(ImageMetrics::ShaderConstants::UNIFORM_NORMALMATRIX);
        if(normalMatrixUniform)
        {
          if(m_uniformMap.find(ImageMetrics::ShaderConstants::UNIFORM_NORMALMATRIX) == m_uniformMap.end())
          {
            m_uniformMap[ImageMetrics::ShaderConstants::UNIFORM_NORMALMATRIX] = std::vector<osg::ref_ptr<osg::Uniform> >();
          }
          m_uniformMap[ImageMetrics::ShaderConstants::UNIFORM_NORMALMATRIX].push_back(normalMatrixUniform);
        }

      }
    }
    traverse(geode);
  }
  
  virtual void apply(osg::Transform& node)
  {
    apply((osg::Node&)node);
  }

  std::map<std::string, std::vector<osg::ref_ptr<osg::Uniform> > > getUniformMap()
  {
    return m_uniformMap;
  }
  
private:
  std::map<std::string, std::vector<osg::ref_ptr<osg::Uniform> > > m_uniformMap;
};


class RemoveGLModeForGLESVisitor : public osg::NodeVisitor
{
public:
  
  RemoveGLModeForGLESVisitor():
  osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
  {
  }
  
  virtual void apply(osg::Node& node)
  {
    if(node.getStateSet())
    {
      removeGLMode(*node.getStateSet());
    }
    traverse(node);
  }
  
  virtual void apply(osg::Geode& node)
  {
    if(node.getStateSet())
    {
      removeGLMode(*node.getStateSet());
    }
    
    for(unsigned int i=0;i<node.getNumDrawables();++i)
    {
      osg::Drawable* drawable = node.getDrawable(i);
      if(drawable && drawable->getStateSet())
      {
        removeGLMode(*drawable->getStateSet());
      }
    }
    
    traverse(node);
  }
  
  void removeGLMode(osg::StateSet& stateset)
  {
    stateset.removeMode(GL_NORMALIZE);
    stateset.removeMode(GL_RESCALE_NORMAL);
    
    osg::StateSet::TextureModeList textList = stateset.getTextureModeList();
    if(textList.size() != 0)
    {
      for(int uid = 0; uid < 10; uid++)
      {
        stateset.removeTextureMode(uid, GL_TEXTURE_2D);
        stateset.removeTextureMode(uid, GL_TEXTURE_CUBE_MAP);

        stateset.removeTextureAttribute(uid, osg::StateAttribute::TEXENV);
      }
    }
  }
};

class FindImageStreamsVisitor : public osg::NodeVisitor
{
public:
  FindImageStreamsVisitor():
  osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN){}
  
  virtual void apply(osg::Geode& geode)
  {
    apply(geode.getStateSet());
    
    for(unsigned int i=0;i<geode.getNumDrawables();++i)
    {
      apply(geode.getDrawable(i)->getStateSet());
    }
  }
  
  virtual void apply(osg::Node& node)
  {
    apply(node.getStateSet());
    traverse(node);
  }
  
  inline void apply(osg::StateSet* stateset)
  {
    if (!stateset) return;
    
    osg::StateAttribute* attr = stateset->getTextureAttribute(0,osg::StateAttribute::TEXTURE);
    if (attr)
    {
      osg::Texture2D* texture2D = dynamic_cast<osg::Texture2D*>(attr);
      if (texture2D) apply(dynamic_cast<osg::ImageStream*>(texture2D->getImage()));
    }
  }
  
  inline void apply(osg::ImageStream* imagestream)
  {
    if (imagestream)
    {
      m_imageStreamList.push_back(imagestream);
    }
  }
  
  std::vector<osg::ImageStream*> getImageStreamList()
  {
    return m_imageStreamList;
  }
  
protected:
  std::vector<osg::ImageStream*> m_imageStreamList;
};


// Copy image files into a given folder
class CopyImageVisitor : public osg::NodeVisitor
{
public:
  CopyImageVisitor(const std::string& srcFolder, const std::string& destFolder):osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
  {
    m_srcFolder = srcFolder;
    m_destFolder = destFolder;
  }
  
  void copyOneImageFile(const std::string& imageFileName, osg::Image* imageObj, int pos)
  {
    if(imageFileName == "")
    {
      return;
    }
    
    //FIXME: should put all images inside a sub folder?
    std::string imageFolder = "";
    
    std::string srcTexFileName =  osgDB::isAbsolutePath(imageFileName) ? imageFileName : osgDB::concatPaths(m_srcFolder, imageFileName);
    std::string simpleImageName = osgDB::getSimpleFileName(imageFileName);
    std::string destTexFileName = osgDB::concatPaths(m_destFolder, osgDB::concatPaths(imageFolder, simpleImageName));
    std::string texFolder = osgDB::getFilePath(destTexFileName);
    
    // if texture is inside a subfolder
#if defined(WIN32)
    if(texFolder != "" &&  _access(texFolder.c_str(), 0) != 0)
    {
      _mkdir(texFolder.c_str());
    }
#else
    if(texFolder != "" &&  access(texFolder.c_str(), 0) != 0)
    {
      mkdir(texFolder.c_str(), 0777);
    }
#endif
    
    osg::ImageSequence* imgSeq = dynamic_cast<osg::ImageSequence*>(imageObj);
    if(imgSeq)
    {
      imgSeq->setImageFile(pos, simpleImageName);
    }
    else
    {
      imageObj->setFileName(simpleImageName);
    }
    std::string cpCommand = "cp \""+srcTexFileName+"\" \""+destTexFileName+"\"";
    std::system(cpCommand.c_str());
  }
  
  virtual void apply(osg::Node& node)
  {
    traverse(node);
  }
  
  virtual void apply(osg::Geode& geode)
  {
    for (int i = 0; i< geode.getNumDrawables(); i++)
    {
      osg::Drawable* dr = geode.getDrawable(i);
      
      if(dr)
      {
        osg::StateSet* stateSet =  dr->getStateSet();
        if(stateSet)
        {
          for(int j = 0; j < stateSet->getAttributeList().size(); j++)
          {
            osg::StateSet::TextureAttributeList textList = stateSet->getTextureAttributeList();
            if(textList.size() != 0)
            {
              for(int uid = 0; uid < 10; uid++)
              {
                osg::Texture2D* tex = dynamic_cast<osg::Texture2D*>(stateSet->getTextureAttribute(uid, osg::StateAttribute::TEXTURE));
                // 2d texture
                if(tex)
                {
                  osg::ImageSequence* imgSeq = dynamic_cast<osg::ImageSequence*>(tex->getImage());
                  // if it is image sequence
                  if(imgSeq)
                  {
                    for(int n = 0; n < imgSeq->getNumImageData(); n++)
                    {
                      copyOneImageFile(imgSeq->getImageFile(n), imgSeq, n);
                    }
                  }
                  else
                  {
                    copyOneImageFile(tex->getImage()->getFileName(), tex->getImage(), 0);
                  }
                }
                
                osg::TextureCubeMap* texCubeMap = dynamic_cast<osg::TextureCubeMap*>(stateSet->getTextureAttribute(uid, osg::StateAttribute::TEXTURE));
                // cube map texture
                if(texCubeMap)
                {
                  copyOneImageFile(texCubeMap->getImage(osg::TextureCubeMap::POSITIVE_X)->getFileName(),
                                   texCubeMap->getImage(osg::TextureCubeMap::POSITIVE_X), 0);
                  copyOneImageFile(texCubeMap->getImage(osg::TextureCubeMap::POSITIVE_Y)->getFileName(),
                                   texCubeMap->getImage(osg::TextureCubeMap::POSITIVE_Y), 0);
                  copyOneImageFile(texCubeMap->getImage(osg::TextureCubeMap::POSITIVE_Z)->getFileName(),
                                   texCubeMap->getImage(osg::TextureCubeMap::POSITIVE_Z), 0);
                  copyOneImageFile(texCubeMap->getImage(osg::TextureCubeMap::NEGATIVE_X)->getFileName(),
                                   texCubeMap->getImage(osg::TextureCubeMap::NEGATIVE_X), 0);
                  copyOneImageFile(texCubeMap->getImage(osg::TextureCubeMap::NEGATIVE_Y)->getFileName(),
                                   texCubeMap->getImage(osg::TextureCubeMap::NEGATIVE_Y), 0);
                  copyOneImageFile(texCubeMap->getImage(osg::TextureCubeMap::NEGATIVE_Z)->getFileName(),
                                   texCubeMap->getImage(osg::TextureCubeMap::NEGATIVE_Z), 0);
                }
              }
            }
          }
        }
      }
    }
    
    traverse(geode);
  }
  
  virtual void apply(osg::Transform& node)
  {
    apply((osg::Node&)node);
  }
  
private:
  std::string m_destFolder;
  std::string m_srcFolder;
};



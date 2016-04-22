#include "MaterialParameterNode.h"
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

namespace osg {
  
MaterialParameterNode::MaterialParameterNode()
:osg::Node()
,m_specularGlowPower(0)
,m_specularGlowFallOff(24)
,m_type("phong")
,m_shouldPlayAnimation(true)
,m_animationLoopingMode(osg::ImageSequence::LOOPING)
,m_animationFPS(24)
,m_mojoLightingWeight(1.0f)
,m_mojoPaintReferenceIntensity(0.5f)
,m_mojoImageReferenceIntensity(0.5f)
{}

MaterialParameterNode::MaterialParameterNode(const MaterialParameterNode& copy,
                      const osg::CopyOp& copyop)
: osg::Node(copy, copyop)
{}

}

// _fileNames
static bool checkDiffuseImageSequence( const osg::MaterialParameterNode& mat )
{
  return !mat.getDiffuseImageSequence().empty();
}

static bool readDiffuseImageSequence( osgDB::InputStream& is, osg::MaterialParameterNode& mat )
{
  std::vector<std::string> filenames;
  unsigned int files = 0;
  is >> files >> is.BEGIN_BRACKET;
  for(unsigned int i = 0; i < files; ++i)
  {
    std::string filename;
    is.readWrappedString( filename );
    filenames.push_back(filename);
  }
  is >> is.END_BRACKET;
  mat.setDiffuseImageSequence(filenames);
  return true;
}

static bool writeDiffuseImageSequence( osgDB::OutputStream& os, const osg::MaterialParameterNode& mat )
{
  const std::vector<std::string> & filenames = mat.getDiffuseImageSequence();
  os.writeSize(filenames.size());
  os << os.BEGIN_BRACKET << std::endl;
  for(std::vector<std::string>::const_iterator itr=filenames.begin(); itr!=filenames.end(); ++itr)
  {
    os.writeWrappedString(*itr);
    os << std::endl;
  }
  os << os.END_BRACKET << std::endl;
  return true;
}

static bool checkNormalMapImageSequence( const osg::MaterialParameterNode& mat )
{
  return !mat.getNormalMapImageSequence().empty();
}

static bool readNormalMapImageSequence( osgDB::InputStream& is, osg::MaterialParameterNode& mat )
{
  std::vector<std::string> filenames;
  unsigned int files = 0;
  is >> files >> is.BEGIN_BRACKET;
  for(unsigned int i = 0; i < files; ++i)
  {
    std::string filename;
    is.readWrappedString( filename );
    filenames.push_back(filename);
  }
  is >> is.END_BRACKET;
  mat.setNormalMapImageSequence(filenames);
  return true;
}

static bool writeNormalMapImageSequence( osgDB::OutputStream& os, const osg::MaterialParameterNode& mat )
{
  const std::vector<std::string> & filenames = mat.getNormalMapImageSequence();
  os.writeSize(filenames.size());
  os << os.BEGIN_BRACKET << std::endl;
  for(std::vector<std::string>::const_iterator itr=filenames.begin(); itr!=filenames.end(); ++itr)
  {
    os.writeWrappedString(*itr);
    os << std::endl;
  }
  os << os.END_BRACKET << std::endl;
  return true;
}


REGISTER_OBJECT_WRAPPER( MaterialParameterNode_Wrapper,
                        new osg::MaterialParameterNode,
                        osg::MaterialParameterNode,
                        "osg::Object osg::MaterialParameterNode" )
{
  ADD_STRING_SERIALIZER(Type, "phong");
  ADD_STRING_SERIALIZER(Name, "");
  
  ADD_FLOAT_SERIALIZER(MojoLightingWeight, 1.0f);
  ADD_FLOAT_SERIALIZER(MojoPaintReferenceIntensity, 0.5f);
  ADD_FLOAT_SERIALIZER(MojoImageReferenceIntensity, 0.5f);

  
  ADD_FLOAT_SERIALIZER(SpecularGlowPower, 0);
  ADD_FLOAT_SERIALIZER(SpecularGlowFallOff, 24);
  
  ADD_STRING_SERIALIZER(DiffuseTextureFileName, "");
  ADD_STRING_SERIALIZER(TransparencyTextureFileName, "");
  ADD_STRING_SERIALIZER(ReflectionTextureFileName, "");
  ADD_STRING_SERIALIZER(EmissiveTextureFileName, "");
  ADD_STRING_SERIALIZER(AmbientTextureFileName, "");
  ADD_STRING_SERIALIZER(NormalMapTextureFileName, "");

  ADD_STRING_SERIALIZER(CubemapNegXImageName, "");
  ADD_STRING_SERIALIZER(CubemapNegYImageName, "");
  ADD_STRING_SERIALIZER(CubemapNegZImageName, "");
  ADD_STRING_SERIALIZER(CubemapPosXImageName, "");
  ADD_STRING_SERIALIZER(CubemapPosYImageName, "");
  ADD_STRING_SERIALIZER(CubemapPosZImageName, "");
  
  ADD_USER_SERIALIZER(DiffuseImageSequence);
  ADD_USER_SERIALIZER(NormalMapImageSequence);
  
  ADD_BOOL_SERIALIZER(ShouldPlayAnimation, true);
  
  BEGIN_ENUM_SERIALIZER4(osg::ImageSequence, LoopingMode,LOOPING);
  ADD_ENUM_CLASS_VALUE(osg::ImageSequence, NO_LOOPING);
  ADD_ENUM_CLASS_VALUE(osg::ImageSequence, LOOPING);
  END_ENUM_SERIALIZER();
  ADD_FLOAT_SERIALIZER(AnimationFPS, 24);

}


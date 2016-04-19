#include <osg/GL>
#include <osg/GLU>

#include <osg/Image>
#include "IMImageOSG.h"
#include "IMTexture2DOSG.h"

#include <osg/Image>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

namespace osg {

IMTexture2DOSG::IMTexture2DOSG()
    :Texture2D()
{
}
  
IMTexture2DOSG::IMTexture2DOSG(IMImageOSG* image):Texture2D(image)
{
}

IMTexture2DOSG::IMTexture2DOSG(const IMTexture2DOSG& text,const CopyOp& copyop):Texture2D(text, copyop)
{
}
  
IMTexture2DOSG::~IMTexture2DOSG()
{
}

}

static bool checkIMImageOSG(const osg::IMTexture2DOSG& tex)
{
  return true;
}

static bool writeIMImageOSG(osgDB::OutputStream& os, const osg::IMTexture2DOSG& tex)
{
  os << os.BEGIN_BRACKET << std::endl;
  //os.writeObject(tex.getIMImageOSG());
  os << os.END_BRACKET << std::endl;
  return true;
}

static bool readIMImageOSG(osgDB::InputStream& is, osg::IMTexture2DOSG& tex)
{
  is >> is.BEGIN_BRACKET;

  //osg::ref_ptr<osg::Object> obj = is.readObject();
  osg::ref_ptr<osg::IMImageOSG> img = new osg::IMImageOSG;
  
  tex.setIMImageOSG(img);
  is >> is.END_BRACKET;
  return true;
}


REGISTER_OBJECT_WRAPPER( IMTexture2DOSG,
                        new osg::IMTexture2DOSG,
                        osg::IMTexture2DOSG,
                        "osg::Object osg::StateAttribute osg::Texture osg::IMTexture2DOSG" )
{
  ADD_USER_SERIALIZER(IMImageOSG);  // _image
}



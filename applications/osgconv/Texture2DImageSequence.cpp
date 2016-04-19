#include <osg/GL>
#include <osg/GLU>

#include <osg/Image>
#include "Texture2DImageSequence.h"

#include <osg/Image>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

namespace osg {

Texture2DImageSequence::Texture2DImageSequence()
    :Texture2D()
{
}

Texture2DImageSequence::Texture2DImageSequence(const Texture2DImageSequence& text,const CopyOp& copyop):Texture2D(text, copyop)
{
}
  
Texture2DImageSequence::~Texture2DImageSequence()
{
}

}


REGISTER_OBJECT_WRAPPER( Texture2DImageSequence,
                        new osg::Texture2DImageSequence,
                        osg::Texture2DImageSequence,
                        "osg::Object osg::StateAttribute osg::Texture osg::Texture2DImageSequence" )
{
  ADD_OBJECT_SERIALIZER(ImageSequence, osg::ImageSequence, NULL );
}



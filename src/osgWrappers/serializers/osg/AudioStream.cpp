#include <osg/AudioStream>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#ifndef IM_OSG_SIZE_REDUCTION

REGISTER_OBJECT_WRAPPER( AudioStream,
                         /*new osg::AudioStream*/NULL,
                         osg::AudioStream,
                         "osg::Object osg::AudioStream" )
{
}
#endif

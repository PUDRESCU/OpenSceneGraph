#include <osg/AudioStream>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#ifndef IM_OSG_SIZE_REDUCTION

REGISTER_OBJECT_WRAPPER( AudioSink,
                         /*new osg::AudioSink*/NULL,
                         osg::AudioSink,
                         "osg::Object osg::AudioSink" )
{
}
#endif

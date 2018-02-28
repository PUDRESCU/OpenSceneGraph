#include <osg/ImageStream>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( ImageStream,
                         new osg::ImageStream,
                         osg::ImageStream,
                         "osg::Object osg::Image osg::ImageStream" )
{
    BEGIN_ENUM_SERIALIZER( LoopingMode, NO_LOOPING );
        ADD_ENUM_VALUE( NO_LOOPING );
        ADD_ENUM_VALUE( LOOPING );
    END_ENUM_SERIALIZER();  // _loopingMode
#ifndef IM_OSG_SIZE_REDUCTION
    ADD_LIST_SERIALIZER( AudioStreams, osg::ImageStream::AudioStreams );  // _audioStreams
#endif
}

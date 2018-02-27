#include <osgGA/TrackballManipulator>

#ifndef IM_SIZE_REDUCTION

#define OBJECT_CAST dynamic_cast

#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgGA_TrackballManipulator,
                         new osgGA::TrackballManipulator,
                         osgGA::TrackballManipulator,
                         "osg::Object osgGA::TrackballManipulator" )
{
}
#endif

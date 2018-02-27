#include <osgGA/DriveManipulator>

#ifndef IM_SIZE_REDUCTION

#define OBJECT_CAST dynamic_cast

#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgGA_DriveManipulator,
                         new osgGA::DriveManipulator,
                         osgGA::DriveManipulator,
                         "osg::Object osgGA::DriveManipulator" )
{
}
#endif

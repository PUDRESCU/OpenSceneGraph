#include <osg/TransferFunction>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#ifndef IM_OSG_SIZE_REDUCTION
REGISTER_OBJECT_WRAPPER( TransferFunction,
                         new osg::TransferFunction,
                         osg::TransferFunction,
                         "osg::Object osg::TransferFunction" )
{
}
#endif

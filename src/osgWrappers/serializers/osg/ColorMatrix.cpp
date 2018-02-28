#include <osg/ColorMatrix>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#ifndef IM_OSG_SIZE_REDUCTION

REGISTER_OBJECT_WRAPPER( ColorMatrix,
                         new osg::ColorMatrix,
                         osg::ColorMatrix,
                         "osg::Object osg::StateAttribute osg::ColorMatrix" )
{
    ADD_MATRIX_SERIALIZER( Matrix, osg::Matrix() );  // _matrix
}
#endif

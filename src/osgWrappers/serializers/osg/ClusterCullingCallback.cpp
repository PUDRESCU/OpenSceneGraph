#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osg/ClusterCullingCallback>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#ifndef IM_OSG_SIZE_REDUCTION

REGISTER_OBJECT_WRAPPER( ClusterCullingCallback,
                         new osg::ClusterCullingCallback,
                         osg::ClusterCullingCallback,
                         "osg::Object osg::Callback osg::NodeCallback osg::ClusterCullingCallback" )
{
    ADD_VEC3_SERIALIZER( ControlPoint, osg::Vec3() );  // _controlPoint
    ADD_VEC3_SERIALIZER( Normal, osg::Vec3() );  // _normal
    ADD_FLOAT_SERIALIZER( Radius, -1.0f );  // _radius
    ADD_FLOAT_SERIALIZER( Deviation, -1.0f );  // _deviation
}
#endif
#undef OBJECT_CAST
#define OBJECT_CAST static_cast

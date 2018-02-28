#include <osg/ShadeModel>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#ifndef IM_OSG_SIZE_REDUCTION

REGISTER_OBJECT_WRAPPER( ShadeModel,
                         new osg::ShadeModel,
                         osg::ShadeModel,
                         "osg::Object osg::StateAttribute osg::ShadeModel" )
{
    BEGIN_ENUM_SERIALIZER( Mode, SMOOTH );
        ADD_ENUM_VALUE( FLAT );
        ADD_ENUM_VALUE( SMOOTH );
    END_ENUM_SERIALIZER();  // _mode
}
#endif

#include "GeodePropertyParameterNode.h"
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( GeodePropertyParameterNode_Wrapper,
                        new osg::GeodePropertyParameterNode,
                        osg::GeodePropertyParameterNode,
                        "osg::Object osg::GeodePropertyParameterNode" )
{
  ADD_STRING_SERIALIZER(NodeName, "");
  ADD_BOOL_SERIALIZER(IsStaticNode, false);
}


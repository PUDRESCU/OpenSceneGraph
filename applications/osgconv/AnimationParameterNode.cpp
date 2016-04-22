#include "AnimationParameterNode.h"
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( AnimationParameterNode_Wrapper,
                        new osg::AnimationParameterNode,
                        osg::AnimationParameterNode,
                        "osg::Object osg::AnimationParameterNode" )
{
  ADD_STRING_SERIALIZER(AnimationName, "animation0");
  ADD_FLOAT_SERIALIZER(StartTime, 0.0f );
  ADD_FLOAT_SERIALIZER(EndTime, 0.0f );
  ADD_FLOAT_SERIALIZER(Duration, 0.0f );
  ADD_FLOAT_SERIALIZER(OriginalFPS, 24.0f );
}


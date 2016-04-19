#include "TriggerParameterNode.h"
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( TriggerParameterNode_Wrapper,
                        new osg::TriggerParameterNode,
                        osg::TriggerParameterNode,
                        "osg::Object osg::TriggerParameterNode" )
{
  ADD_STRING_SERIALIZER(TriggerType, "");
  ADD_STRING_SERIALIZER(NodeName, "");
  ADD_STRING_SERIALIZER(TriggerAction, "");
  ADD_FLOAT_SERIALIZER(TriggerTimeoutSecs, -1.0f );  // _referenceValue
  ADD_BOOL_SERIALIZER(TriggerAutoTimeout, false);
  ADD_BOOL_SERIALIZER(TriggeredDefault, false);
  ADD_STRING_SERIALIZER(ControlType, "switch");
}


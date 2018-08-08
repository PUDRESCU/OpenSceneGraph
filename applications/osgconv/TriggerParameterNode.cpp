#include "TriggerParameterNode.h"
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

namespace osg {
  
TriggerParameterNode::TriggerParameterNode()
:osg::Node()
,m_triggerType("")
,m_nodeName("")
,m_triggerAction("")
,m_triggerLoopOption("")
,m_triggerTimeoutSecs(-1.0f)
,m_triggerDelaySecs(-1.0f)
,m_controlType("switch")
,m_triggerLimit(-1)
{}

TriggerParameterNode::TriggerParameterNode(const TriggerParameterNode& copy,
                     const osg::CopyOp& copyop)
: osg::Node(copy, copyop)
{}

TriggerParameterNode::~TriggerParameterNode()
{
}
  
}

extern "C" void wrapper_serializer_TriggerParameterNode(void) {}
extern void wrapper_propfunc_TriggerParameterNode(osgDB::ObjectWrapper*);
static osg::Object* wrapper_createinstancefuncTriggerParameterNode() { return new osg::TriggerParameterNode; }
static osgDB::RegisterWrapperProxy wrapper_proxy_TriggerParameterNode(
	wrapper_createinstancefuncTriggerParameterNode, "osg::TriggerParameterNode", "osg::Object osg::TriggerParameterNode", &wrapper_propfunc_TriggerParameterNode);
void wrapper_propfunc_TriggerParameterNode(osgDB::ObjectWrapper* wrapper)
{
  typedef osg::TriggerParameterNode MyClass;

  ADD_STRING_SERIALIZER(TriggerType, "");
  ADD_STRING_SERIALIZER(NodeName, "");
  ADD_STRING_SERIALIZER(TriggerAction, "");
  ADD_FLOAT_SERIALIZER(TriggerTimeoutSecs, -1.0f ); 
  ADD_STRING_SERIALIZER(TriggerLoopOption, "");
  ADD_FLOAT_SERIALIZER(TriggerDelaySecs, -1.0f );
  ADD_STRING_SERIALIZER(ControlType, "switch");
  ADD_INT_SERIALIZER(TriggerLimit, -1);
}

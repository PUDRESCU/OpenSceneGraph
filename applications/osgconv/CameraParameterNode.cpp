#include "CameraParameterNode.h"
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

namespace osg {
  
CameraParameterNode::CameraParameterNode()
:osg::Node()
,m_translate(DEFAULT_ASSET_CAMERA_POSITION)
,m_rotate(0, 0, 0)
,m_rotateOrder("xyz")
{}

CameraParameterNode::CameraParameterNode(const CameraParameterNode& copy,
                     const osg::CopyOp& copyop)
: osg::Node(copy, copyop)
{}

CameraParameterNode::~CameraParameterNode()
{
}
  
}

extern "C" void wrapper_serializer_CameraParameterNode(void) {}
extern void wrapper_propfunc_CameraParameterNode(osgDB::ObjectWrapper*);
static osg::Object* wrapper_createinstancefuncCameraParameterNode() { return new osg::CameraParameterNode; }
static osgDB::RegisterWrapperProxy wrapper_proxy_CameraParameterNode(
	wrapper_createinstancefuncCameraParameterNode, "osg::CameraParameterNode", "osg::Object osg::CameraParameterNode", &wrapper_propfunc_CameraParameterNode);
void wrapper_propfunc_CameraParameterNode(osgDB::ObjectWrapper* wrapper)
{
  typedef osg::CameraParameterNode MyClass;
  ADD_VEC3F_SERIALIZER(Translate, DEFAULT_ASSET_CAMERA_POSITION);
  ADD_VEC3F_SERIALIZER(Rotate, osg::Vec3(0, 0, 0));
  ADD_STRING_SERIALIZER(RotateOrder, "xyz");
}


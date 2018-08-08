#pragma once

#include <osg/Node>
#include <osg/Group>
#include <osg/Notify>
#include <osg/Vec3>

//LDSDK-574: in order to set static node correctly, we need to get camera position in design space.
//The default coordinate is (0, 2, 48)
#define DEFAULT_ASSET_CAMERA_POSITION osg::Vec3(0, 2, 48)

//-------------------------------------------------------------------------------------------------------------------------
namespace osg {
  
  class CameraParameterNode : public osg::Node
  {
  public:
    CameraParameterNode();
  
    CameraParameterNode(const CameraParameterNode& copy,
                        const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
    
    META_Node(osg, CameraParameterNode)

    void setTranslate(const Vec3 &translate) { m_translate = translate; }
    inline const Vec3 &getTranslate() const { return m_translate; }

    void setRotate(const Vec3 &rotate) { m_rotate = rotate; }
    inline const Vec3 &getRotate() const { return m_rotate; }
    
    void setRotateOrder(const std::string& rotateOrder) { m_rotateOrder = rotateOrder; }
    const std::string& getRotateOrder() const { return m_rotateOrder; }
    
  protected :
    virtual ~CameraParameterNode();
    
  protected:
    Vec3 m_translate;
    Vec3 m_rotate;
    std::string m_rotateOrder;
  };
  
}

#pragma once

#include <osg/Group>
#include <osg/Notify>
#include <osg/Vec3>


//-------------------------------------------------------------------------------------------------------------------------
namespace osg {
  
  class GeodePropertyParameterNode : public osg::Node
  {
  public:
    GeodePropertyParameterNode()
    :osg::Node()
    ,m_isStaticNode(false)
    ,m_nodeName("")
    {}
    
    GeodePropertyParameterNode(const GeodePropertyParameterNode& copy,
                        const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
    : osg::Node(copy, copyop)
    {}
    
    META_Node(osg, GeodePropertyParameterNode)

    
    void setIsStaticNode(bool value) { m_isStaticNode = value; }
    bool getIsStaticNode() const { return m_isStaticNode; }
    
    void setNodeName( const  std::string & name ) { m_nodeName = name; }
    inline const std::string& getNodeName() const { return m_nodeName; }
    
  protected:
    bool m_isStaticNode;
    std::string m_nodeName;
  };
}

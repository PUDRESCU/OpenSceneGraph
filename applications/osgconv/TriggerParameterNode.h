#pragma once

#include <osg/Group>
#include <osg/Notify>
#include <osg/Vec3>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/BlendFunc>
#include <osg/Timer>
#include <osg/Material>
#include <osg/TextureCubeMap>
#include <osg/Texture2D>

//-------------------------------------------------------------------------------------------------------------------------
namespace osg {
  
  class TriggerParameterNode : public osg::Node
  {
  public:
    TriggerParameterNode()
    :osg::Node()
    ,m_triggerType("")
    ,m_nodeName("")
    ,m_triggerAction("")
    ,m_triggerAutoTimeout(false)
    ,m_triggerTimeoutSecs(-1.0f)
    ,m_triggeredDefault(false)
    ,m_controlType("switch")
    {}
    
    TriggerParameterNode(const TriggerParameterNode& copy,
                        const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
    : osg::Node(copy, copyop)
    {}
    
    META_Node(osg, TriggerParameterNode)

    
    void setTriggerType( const  std::string & type ) { m_triggerType = type; }
    inline const std::string& getTriggerType() const { return m_triggerType; }
    
    void setNodeName( const  std::string & name ) { m_nodeName = name; }
    inline const std::string& getNodeName() const { return m_nodeName; }
    
    void setTriggeredDefault(bool value) {m_triggeredDefault = value;}
    bool getTriggeredDefault() const {return m_triggeredDefault;}
    
    void setControlType( const  std::string & type ) { m_controlType = type; }
    inline const std::string& getControlType() const { return m_controlType; }

    void setTriggerAction( const  std::string & action ) { m_triggerAction = action; }
    inline const std::string& getTriggerAction() const { return m_triggerAction; }
    
    void setTriggerAutoTimeout(bool value) {m_triggerAutoTimeout = value;}
    bool getTriggerAutoTimeout() const {return m_triggerAutoTimeout;}

    void setTriggerTimeoutSecs(float value) {m_triggerTimeoutSecs = value;}
    float getTriggerTimeoutSecs() const {return m_triggerTimeoutSecs;}
    
  protected:
    std::string m_triggerType;
    std::string m_nodeName;
    std::string m_triggerAction;
    bool m_triggerAutoTimeout;
    float m_triggerTimeoutSecs;
    bool m_triggeredDefault;
    std::string m_controlType;  //"switch", "animationmanager"
  };
  
}

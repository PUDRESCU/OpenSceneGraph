#pragma once

#include <osg/Node>
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
    TriggerParameterNode();
  
    TriggerParameterNode(const TriggerParameterNode& copy,
                         const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
    
    META_Node(osg, TriggerParameterNode)

    
    void setTriggerType( const  std::string & type ) { m_triggerType = type; }
    inline const std::string& getTriggerType() const { return m_triggerType; }
    
    void setNodeName( const  std::string & name ) { m_nodeName = name; }
    inline const std::string& getNodeName() const { return m_nodeName; }

    bool getTriggeredDefault() const { return false; }
    bool getTriggerAutoTimeout() const { return false; }
    
    void setControlType( const  std::string & type ) { m_controlType = type; }
    inline const std::string& getControlType() const { return m_controlType; }

    void setTriggerAction( const  std::string & action ) { m_triggerAction = action; }
    inline const std::string& getTriggerAction() const { return m_triggerAction; }
    
    void setTriggerLoopOption(const std::string & option) {m_triggerLoopOption = option;}
    inline const std::string& getTriggerLoopOption() const {return m_triggerLoopOption;}

    void setTriggerTimeoutSecs(float value) {m_triggerTimeoutSecs = value;}
    float getTriggerTimeoutSecs() const {return m_triggerTimeoutSecs;}
    
    void setTriggerDelaySecs(float value) {m_triggerDelaySecs = value;}
    float getTriggerDelaySecs() const {return m_triggerDelaySecs;}

    void setTriggerLimit(int value) {m_triggerLimit = value;}
    int getTriggerLimit() const {return m_triggerLimit;}
    
  protected :
    virtual ~TriggerParameterNode();
    
  protected:
    std::string m_triggerType;
    std::string m_nodeName;
    std::string m_triggerAction;
    std::string m_triggerLoopOption;
    float m_triggerTimeoutSecs;
    float m_triggerDelaySecs;

    std::string m_controlType;  //"switch", "animationmanager"
    int m_triggerLimit;
  };
  
}

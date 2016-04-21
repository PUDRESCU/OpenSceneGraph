#pragma once

#include <osg/Group>
#include <osg/Notify>
#include <osg/Vec3>


//-------------------------------------------------------------------------------------------------------------------------
namespace osg {
  
  class AnimationParameterNode : public osg::Node
  {
  public:
    AnimationParameterNode()
    :osg::Node()
    ,m_animationName("animation0")
    ,m_startTime(0.0f)
    ,m_endTime(0.0f)
    ,m_duration(0.0f)
    {}
    
    AnimationParameterNode(const AnimationParameterNode& copy,
                        const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
    : osg::Node(copy, copyop)
    {}
    
    META_Node(osg, AnimationParameterNode)

    
    void setAnimationName( const  std::string & name ) { m_animationName = name; }
    inline const std::string& getAnimationName() const { return m_animationName; }

    void setStartTime(float value) {m_startTime = value;}
    float getStartTime() const {return m_startTime;}
    
    void setEndTime(float value) {m_endTime = value;}
    float getEndTime() const {return m_endTime;}
    
    void setDuration(float value) {m_duration = value;}
    float getDuration() const {return m_duration;}

  protected:
    std::string m_animationName;
    float m_startTime;
    float m_endTime;
    float m_duration;
  };
  
}

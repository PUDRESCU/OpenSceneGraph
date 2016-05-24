//
//  UpdateNodeVisibility.h
//  OpenSceneGraph
//
//  Created by Cheng-Hua Pai on 2016/5/23.
//
//

#ifndef OSGANIMATION_UPDATE_NODE_VISIBILITY
#define OSGANIMATION_UPDATE_NODE_VISIBILITY 1

#include <osg/Callback>
#include <osgAnimation/AnimationUpdateCallback>

namespace ImageMetrics
{
  class UpdateNodeVisibility : public osgAnimation::AnimationUpdateCallback<osg::NodeCallback>
  {
  public:
    META_Object(ImageMetrics, UpdateNodeVisibility);
    
    UpdateNodeVisibility(const std::string& name = "");
    UpdateNodeVisibility(const UpdateNodeVisibility& apc,const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);
    
    // Callback method called by the NodeVisitor when visiting a node.
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
    virtual bool link(osgAnimation::Channel* channel);
    
  protected:

    osg::ref_ptr<osgAnimation::FloatTarget> _visibility;
  };
}

#endif /* UpdateNodeVisibility_h */

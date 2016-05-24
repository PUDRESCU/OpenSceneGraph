//
//  UpdateNodeVisibility.cpp
//  OpenSceneGraph
//
//  Created by Cheng-Hua Pai on 2016/5/23.
//
//

#include "UpdateNodeVisibility.h"

#include <osg/NodeVisitor>
#include <osg/Switch>

using namespace ImageMetrics;

UpdateNodeVisibility::UpdateNodeVisibility( const UpdateNodeVisibility& apc,const osg::CopyOp& copyop) : osg::Object(apc,copyop), AnimationUpdateCallback<osg::NodeCallback>(apc, copyop)
{
  _visibility = new osgAnimation::FloatTarget(apc._visibility->getValue());
}

UpdateNodeVisibility::UpdateNodeVisibility(const std::string& name) : AnimationUpdateCallback<osg::NodeCallback>(name)
{
  _visibility = new osgAnimation::FloatTarget(1.f);
}

/** Callback method called by the NodeVisitor when visiting a node.*/
void UpdateNodeVisibility::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
  if (nv && nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
  {
    osg::Switch *switchNode = node->asSwitch();
    if(switchNode)
    {
      switchNode->setValue(0, _visibility->getValue() != 0);
    }
  }
  traverse(node,nv);
}

bool UpdateNodeVisibility::link(osgAnimation::Channel* channel)
{
  // check if we can link a StackedTransformElement to the current Channel
  if(channel->getName() == "visibility")
  {
    channel->setTarget(_visibility.get());
    return true;
  }
  
  return false;
}


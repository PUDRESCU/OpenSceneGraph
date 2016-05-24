//
//  UpdateNodeVisibility.cpp
//  OpenSceneGraph
//
//  Created by Cheng-Hua Pai on 2016/5/23.
//
//

#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osgAnimation/UpdateNodeVisibility>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgAnimation_UpdateNodeVisibility,
                        new osgAnimation::UpdateNodeVisibility,
                        osgAnimation::UpdateNodeVisibility,
                        "osg::Object osg::Callback osg::NodeCallback osgAnimation::UpdateNodeVisibility" )
{
}

#undef OBJECT_CAST
#define OBJECT_CAST static_cast

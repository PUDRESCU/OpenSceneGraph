//
//  BackgroundRenderPass.hpp
//  OptasiaOpenSceneGraph
//
//  Created by Jie Xu on 1/19/16.
//  Copyright © 2016 Image Metrics, Inc. All rights reserved.
//

#ifndef BackgroundRenderPass_hpp
#define BackgroundRenderPass_hpp

#include <stdio.h>
#include <osg/ref_ptr>
#include <osg/Group>
#include <osg/Camera>
#include <osg/MatrixTransform>
#include <osg/Projection>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/TextureRectangle>
#include "IMTexture2DOSG.h"
#include <osg/FrameBufferObject>

namespace ImageMetrics
{
  
class BackgroundRenderPass
{
public:
  BackgroundRenderPass(osg::ref_ptr<osg::IMTexture2DOSG> inputTexture,
                       int width, int height,
                       osg::ref_ptr<osg::FrameBufferObject> fbo);
  ~BackgroundRenderPass();
  osg::ref_ptr<osg::Group> getRoot() { return m_rootGroup; }
  osg::ref_ptr<osg::Camera> getCamera() { return m_camera; }
  osg::ref_ptr<osg::MatrixTransform> getTransform() { return m_transform; }
  void setShader();
  
private:
  osg::ref_ptr<osg::Group> createGeometry();

  void setupCamera();
  
  osg::ref_ptr<osg::Group> m_rootGroup;
  osg::ref_ptr<osg::Camera> m_camera;
  osg::ref_ptr<osg::Geode> m_geode;
  osg::ref_ptr<osg::IMTexture2DOSG> m_inputTexture;
  int m_width;
  int m_height;
  osg::ref_ptr<osg::StateSet> m_stateSet;
  osg::ref_ptr<osg::MatrixTransform> m_transform;
  osg::ref_ptr<osg::FrameBufferObject> m_fbo;
};
  
}
#endif /* BackgroundRenderPass_hpp */

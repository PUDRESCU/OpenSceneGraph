//
//  OverlayRenderPass.hpp
//  OptasiaOpenSceneGraph
//
//  Created by Jie Xu on 1/19/16.
//  Copyright © 2016 Image Metrics, Inc. All rights reserved.
//

#ifndef OverlayRenderPass_hpp
#define OverlayRenderPass_hpp

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
#include <osg/FrameBufferObject>

namespace ImageMetrics
{
  
class OverlayRenderPass
{
public:
  OverlayRenderPass(bool isDeviceDeployment, osg::ref_ptr<osg::Texture2D> inputTexture,
                    int width, int height);
  ~OverlayRenderPass();
  osg::ref_ptr<osg::Group> getRoot() { return m_rootGroup; }
  osg::ref_ptr<osg::Camera> getCamera() { return m_camera; }
  osg::ref_ptr<osg::MatrixTransform> getTransform() { return m_transform; }
  osg::ref_ptr<osg::Node> getGeode() { return m_geode; }

  void setShader();
  
private:
  osg::ref_ptr<osg::Node> createGeometry();

  void setupCamera();
  
  osg::ref_ptr<osg::Group> m_rootGroup;
  osg::ref_ptr<osg::Node> m_geode;

  osg::ref_ptr<osg::Camera> m_camera;
  osg::ref_ptr<osg::Texture2D> m_inputTexture;
  int m_width;
  int m_height;
  osg::ref_ptr<osg::StateSet> m_stateSet;
  osg::ref_ptr<osg::MatrixTransform> m_transform;
  
  bool m_isDeviceDeployment;
  std::string m_vertexShaderCode;
  std::string m_fragmentShaderCode;
};
  
}
#endif /* OverlayRenderPass_hpp */

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
namespace ImageMetrics {
  
  class AlignmentInfoNode : public osg::Node
  {
  public:
    AlignmentInfoNode()
    :osg::Node()
    ,m_useRBF(false)
    {}
    
    AlignmentInfoNode(const AlignmentInfoNode& copy,
                      const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
    : osg::Node(copy, copyop)
    {}
    
    META_Node(ImageMetrics, AlignmentInfoNode)
    
    void setPoints3DObject(const std::vector<osg::Vec3> &points) { m_points3dObject = points; }
    inline const std::vector<osg::Vec3>& getPoints3DObject() const { return m_points3dObject; }

    void setOrigin3DObject(const std::vector<osg::Vec3> &points) { m_origin3dObject = points; }
    inline const std::vector<osg::Vec3>& getOrigin3DObject() const { return m_origin3dObject; }
  
    void setLandmarkNames(const std::vector<std::string> &names) { m_landmarkNames = names; }
    inline const std::vector<std::string>& getLandmarkNames() const { return m_landmarkNames; }
    
    void setAlignmentType(const std::string & type) { m_alignmentType = type; }
    inline const std::string& getAlignmentType() const { return m_alignmentType; }
    
    void setUseRBF(bool value) { m_useRBF = value; }
    inline bool getUseRBF() const { return m_useRBF; }
    
    void setDeltas(const std::vector<osg::ref_ptr<osg::Array> > &deltas) { m_deltas = deltas; }
    inline const std::vector<osg::ref_ptr<osg::Array> >& getDeltas() const { return m_deltas; }
    
    void setFittingLandmarkNames(const std::vector<std::string> &names) { m_fittingLandMarkNames = names; }
    inline const std::vector<std::string>& getFittingLandmarkNames() const { return m_fittingLandMarkNames; }
    
    void setTrackingInd(const std::vector<int> &ind) { m_trackingInd = ind; }
    inline const std::vector<int>& getTrackingInd() const { return m_trackingInd; }
    
    void setObjectInd(const std::vector<int> &ind) { m_objectInd = ind; }
    inline const std::vector<int>& getObjectInd() const { return m_objectInd; }
    
    void setFittingTrackingInd(const std::vector<int> &ind) { m_fittingTrackingInd = ind; }
    inline const std::vector<int>& getFittingTrackingInd() const { return m_fittingTrackingInd; }
    
    void setFittingObjectInd(const std::vector<int> &ind) { m_fittingObjectInd = ind; }
    inline const std::vector<int>& getFittingObjectInd() const { return m_fittingObjectInd; }
    
  protected:
    std::vector<osg::Vec3> m_points3dObject;
    std::vector<osg::Vec3> m_origin3dObject;
    std::vector<std::string> m_landmarkNames;
    std::string m_alignmentType;
    bool m_useRBF;
    std::vector<osg::ref_ptr<osg::Array> > m_deltas;
    std::vector<std::string> m_fittingLandMarkNames;
    std::vector<int> m_trackingInd;
    std::vector<int> m_objectInd;
    std::vector<int> m_fittingTrackingInd;
    std::vector<int> m_fittingObjectInd;

  };
  
}

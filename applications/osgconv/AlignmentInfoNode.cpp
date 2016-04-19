#include "AlignmentInfoNode.h"
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkDeltas(const ImageMetrics::AlignmentInfoNode& info)
{
  return info.getDeltas().size() > 0;
}

static bool readDeltas(osgDB::InputStream& is, ImageMetrics::AlignmentInfoNode& info)
{
  unsigned int size = is.readSize();
  std::vector<osg::ref_ptr<osg::Array> > deltas;
  
  is >> is.BEGIN_BRACKET;
  for(unsigned int i = 0; i < size; ++i)
  {
    is >> is.BEGIN_BRACKET;
    osg::ref_ptr<osg::Array> array = is.readArray();
    deltas.push_back(array);
    is >> is.END_BRACKET;
  }
  is >> is.END_BRACKET;
  return true;
}

static bool writeDeltas(osgDB::OutputStream& os, const ImageMetrics::AlignmentInfoNode& info)
{
  const std::vector<osg::ref_ptr<osg::Array> >& deltas = info.getDeltas();
  os.writeSize(deltas.size());
  os << os.BEGIN_BRACKET << std::endl;
  for(std::vector<osg::ref_ptr<osg::Array> >::const_iterator itr=deltas.begin();
       itr!=deltas.end(); ++itr )
  {
    os << os.BEGIN_BRACKET << std::endl;
    os << itr->get();
    os << os.END_BRACKET << std::endl;
  }
  os << os.END_BRACKET << std::endl;
  return true;
}

REGISTER_OBJECT_WRAPPER(AlignmentInfoNode_Wrapper,
                        new ImageMetrics::AlignmentInfoNode,
                        ImageMetrics::AlignmentInfoNode,
                        "osg::Object ImageMetrics::AlignmentInfoNode")
{
  ADD_LIST_SERIALIZER(Points3DObject, std::vector<osg::Vec3>);
  ADD_LIST_SERIALIZER(Origin3DObject, std::vector<osg::Vec3>);
  ADD_LIST_SERIALIZER(LandmarkNames, std::vector<std::string>);

  ADD_STRING_SERIALIZER(AlignmentType, "sRt");
  ADD_BOOL_SERIALIZER(UseRBF, false);
  ADD_USER_SERIALIZER(Deltas);

  ADD_LIST_SERIALIZER(FittingLandmarkNames, std::vector<std::string>);
  ADD_LIST_SERIALIZER(TrackingInd, std::vector<int>);
  ADD_LIST_SERIALIZER(ObjectInd, std::vector<int>);
  ADD_LIST_SERIALIZER(FittingTrackingInd, std::vector<int>);
  ADD_LIST_SERIALIZER(FittingObjectInd, std::vector<int>);
}


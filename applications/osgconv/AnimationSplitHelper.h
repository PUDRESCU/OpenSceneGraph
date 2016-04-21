#include <stdio.h>
#include <algorithm>
#include <string>
#include <iostream>

#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>
#include <osg/Group>
#include <osg/Notify>
#include <osg/Vec3>
#include <osg/ProxyNode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/BlendFunc>
#include <osg/Timer>
#include <osg/Material>
#include <osg/Node>

#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>
#include <osgDB/Registry>

//
//Finds an Animation by name, returning a pointer to the animation
//and a pointer to the manager it was found in
struct FindOsgAnimationByName : public osg::NodeVisitor
{
  //the name of the source animation we are looking for
  std::string _sourceAnimationName;
  
  //used to return the animation and the manager it was stored in
  osgAnimation::Animation* p_ani;
  osgAnimation::AnimationManagerBase* p_manager;
  
  FindOsgAnimationByName(std::string sourceAnimationName)
		: osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
		p_ani(NULL),
		p_manager(NULL),
		_sourceAnimationName(sourceAnimationName)
  {
  }
  
  void apply(osg::Node& node)
  {
    if(node.getUpdateCallback())
    {
      osgAnimation::AnimationManagerBase* b = dynamic_cast<osgAnimation::AnimationManagerBase*>(node.getUpdateCallback());
      if(b)
      {
        //we have found a valid osgAnimation manager, now look for the single long animation inside with the desired name
        osgAnimation::AnimationList aniList = b->getAnimationList();
        for(unsigned int i=0; i<aniList.size(); i++)
        {
          if(aniList[i]->getName() == _sourceAnimationName)
          {
            p_manager = b;
            p_ani = aniList[i].get();
            return;
          }
        }
      }
    }
    traverse(node);
  }
};

class AnimationUtils
{
public:
  
  //returns the index of the keyframe closest to the passed time
  //returns -1 if the time is out of range of the key container
  template <typename ContainerType>
  static int GetNearestKeyFrameIndex(ContainerType* keyContainer, double time)
  {
    if(!keyContainer)
    {
      return -1;
    }
    
    int closestFrame = -1;
    double closestDiff = 99999.99f;
    
    //loop all the keys
    for(unsigned int i=0; i<keyContainer->size(); i++)
    {
      double diff = fabs(time - (*keyContainer)[i].getTime());

      if(diff < closestDiff)
      {
        closestFrame = i;
        closestDiff = diff;
      }
    }
    return closestFrame;
  }
  
  //// Helper method for resampling channels
  template <typename ChannelType, typename ContainerType>
  static osg::ref_ptr<ChannelType> ResampleChannel(ChannelType* sourceChannel, double startTime, double endTime)
  {
    osg::ref_ptr<ChannelType> newChannel = NULL;
    if(!sourceChannel)
    {
      return newChannel;
    }
    
    //get the key frame container from the source channel
    ContainerType* sourceKeyCont  = sourceChannel->getSamplerTyped()->getKeyframeContainerTyped();
    
    if (sourceKeyCont)
    {
      unsigned int from;
      unsigned int to;
      
      //find the true key frame numbers as some animation channels contain more than the actual fps
      from = GetNearestKeyFrameIndex<ContainerType>(sourceKeyCont, startTime);
      if(from == -1)
      {
        return newChannel;
      }
      
      to = GetNearestKeyFrameIndex<ContainerType>(sourceKeyCont, endTime);
      if(to == -1)
      {
        return newChannel;
      }
      
      //get the offset time form the keys
      double offsetToZero = (*sourceKeyCont)[from].getTime();
      
      //create our new channel with same type, name and target name
      newChannel = new ChannelType();
      newChannel->setName(sourceChannel->getName());
      newChannel->setTargetName(sourceChannel->getTargetName());
      
      //get the new channels key containter, also creating the sampler and container if required
      ContainerType* destKeyCont = newChannel->getOrCreateSampler()->getOrCreateKeyframeContainer();
      
      //now copy all the frames between startTime and endTime
      for(unsigned int k = from; k <= to; k++)
      {
        //push source frame onto destination container
        destKeyCont->push_back((*sourceKeyCont)[k]);
        //adjust the new frames time so animation starts from 0.0
        double sourceTime = (*sourceKeyCont)[k].getTime();
        (*destKeyCont)[destKeyCont->size()-1].setTime(sourceTime-offsetToZero);
      }
      
      return newChannel;
    }
    else
    {
      OSG_WARN << "OsgAnimationTools ResampleChannel ERROR: source channel contains no key frame container," << std::endl;
      return newChannel;
    }
    return newChannel;
  }
  
  //
  //Create and return a new animation based on the start end frames of the source animation
  //creating all the relevant channels etc
  static osg::ref_ptr<osgAnimation::Animation> ResampleAnimation(osgAnimation::Animation* source,
                                                                 double startTime, double endTime, double duration,
                                                                 std::string newName)
  {
    osg::ref_ptr<osgAnimation::Animation> newAnimation = NULL;
    if(!source)
    {
      return newAnimation;
    }
    
    newAnimation = new osgAnimation::Animation();
    newAnimation->setName(newName);
    
    //loop all the source channels
    osgAnimation::ChannelList sourceChannels = source->getChannels();
    for(unsigned int i=0; i<sourceChannels.size(); i++)
    {
      //clone the channel type, name and target
      //this is a dumb method but I cant find a way to just clone the channel type,
      //so instead we copy the whole channel, keys and all, then delete the ones we don't want :(
      osgAnimation::Channel* pChannel = sourceChannels[i].get();
      osg::ref_ptr<osgAnimation::Channel> resampledChannel = NULL;
      
      //osgAnimation::Channel* pChannel = anim.getChannels()[i].get();
      
      osgAnimation::DoubleLinearChannel* pDlc = dynamic_cast<osgAnimation::DoubleLinearChannel*>(pChannel);
      if (pDlc)
      {
        resampledChannel = ResampleChannel<osgAnimation::DoubleLinearChannel, osgAnimation::DoubleKeyframeContainer>(pDlc, startTime, endTime);
        newAnimation->addChannel(resampledChannel.get());
        continue;
      }
      osgAnimation::FloatLinearChannel* pFlc = dynamic_cast<osgAnimation::FloatLinearChannel*>(pChannel);
      if (pFlc)
      {
        resampledChannel = ResampleChannel<osgAnimation::FloatLinearChannel, osgAnimation::FloatKeyframeContainer>(pFlc, startTime, endTime);
        newAnimation->addChannel(resampledChannel.get());
        continue;
      }
      osgAnimation::Vec2LinearChannel* pV2lc = dynamic_cast<osgAnimation::Vec2LinearChannel*>(pChannel);
      if (pV2lc)
      {
        resampledChannel = ResampleChannel<osgAnimation::Vec2LinearChannel, osgAnimation::Vec2KeyframeContainer>(pV2lc, startTime, endTime);
        newAnimation->addChannel(resampledChannel.get());
        continue;
      }
      osgAnimation::Vec3LinearChannel* pV3lc = dynamic_cast<osgAnimation::Vec3LinearChannel*>(pChannel);
      if (pV3lc)
      {
        resampledChannel = ResampleChannel<osgAnimation::Vec3LinearChannel, osgAnimation::Vec3KeyframeContainer>(pV3lc, startTime, endTime);
        newAnimation->addChannel(resampledChannel.get());
        continue;
      }
      osgAnimation::Vec4LinearChannel* pV4lc = dynamic_cast<osgAnimation::Vec4LinearChannel*>(pChannel);
      if (pV4lc)
      {
        resampledChannel = ResampleChannel<osgAnimation::Vec4LinearChannel, osgAnimation::Vec4KeyframeContainer>(pV4lc, startTime, endTime);
        newAnimation->addChannel(resampledChannel.get());
        continue;
      }
      osgAnimation::QuatSphericalLinearChannel* pQslc = dynamic_cast<osgAnimation::QuatSphericalLinearChannel*>(pChannel);
      if (pQslc)
      {
        resampledChannel = ResampleChannel<osgAnimation::QuatSphericalLinearChannel, osgAnimation::QuatKeyframeContainer>(pQslc, startTime, endTime);
        newAnimation->addChannel(resampledChannel.get());
        continue;
      }
      osgAnimation::FloatCubicBezierChannel* pFcbc = dynamic_cast<osgAnimation::FloatCubicBezierChannel*>(pChannel);
      if (pFcbc)
      {
        resampledChannel = ResampleChannel<osgAnimation::FloatCubicBezierChannel, osgAnimation::FloatCubicBezierKeyframeContainer>(pFcbc, startTime, endTime);
        newAnimation->addChannel(resampledChannel.get());
        continue;
      }
      osgAnimation::DoubleCubicBezierChannel* pDcbc = dynamic_cast<osgAnimation::DoubleCubicBezierChannel*>(pChannel);
      if (pDcbc)
      {
        resampledChannel = ResampleChannel<osgAnimation::DoubleCubicBezierChannel, osgAnimation::DoubleCubicBezierKeyframeContainer>(pDcbc, startTime, endTime);
        newAnimation->addChannel(resampledChannel.get());
        continue;
      }
      osgAnimation::Vec2CubicBezierChannel* pV2cbc = dynamic_cast<osgAnimation::Vec2CubicBezierChannel*>(pChannel);
      if (pV2cbc)
      {
        resampledChannel = ResampleChannel<osgAnimation::Vec2CubicBezierChannel, osgAnimation::Vec2CubicBezierKeyframeContainer>(pV2cbc, startTime, endTime);
        newAnimation->addChannel(resampledChannel.get());
        continue;
      }
      osgAnimation::Vec3CubicBezierChannel* pV3cbc = dynamic_cast<osgAnimation::Vec3CubicBezierChannel*>(pChannel);
      if (pV3cbc)
      {
        resampledChannel = ResampleChannel<osgAnimation::Vec3CubicBezierChannel, osgAnimation::Vec3CubicBezierKeyframeContainer>(pV3cbc, startTime, endTime);
        newAnimation->addChannel(resampledChannel.get());
        continue;
      }
      osgAnimation::Vec4CubicBezierChannel* pV4cbc = dynamic_cast<osgAnimation::Vec4CubicBezierChannel*>(pChannel);
      if (pV4cbc)
      {
        resampledChannel = ResampleChannel<osgAnimation::Vec4CubicBezierChannel, osgAnimation::Vec4CubicBezierKeyframeContainer>(pV4cbc, startTime, endTime);
        newAnimation->addChannel(resampledChannel.get());
        continue;
      }
    }//loop channel
    
    newAnimation->setDuration(duration);
    return newAnimation;
  }
};

 
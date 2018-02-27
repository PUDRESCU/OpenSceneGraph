#include <osgParticle/ExplosionDebrisEffect>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#ifndef IM_SIZE_REDUCTION

REGISTER_OBJECT_WRAPPER( osgParticleExplosionDebrisEffect,
                         new osgParticle::ExplosionDebrisEffect,
                         osgParticle::ExplosionDebrisEffect,
                         "osg::Object osg::Node osg::Group osgParticle::ParticleEffect osgParticle::ExplosionDebrisEffect" )
{
}

#endif

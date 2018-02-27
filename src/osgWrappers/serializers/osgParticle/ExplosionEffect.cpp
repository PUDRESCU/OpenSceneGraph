#include <osgParticle/ExplosionEffect>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#ifndef IM_SIZE_REDUCTION

REGISTER_OBJECT_WRAPPER( osgParticleExplosionEffect,
                         new osgParticle::ExplosionEffect,
                         osgParticle::ExplosionEffect,
                         "osg::Object osg::Node osg::Group osgParticle::ParticleEffect osgParticle::ExplosionEffect" )
{
}
#endif

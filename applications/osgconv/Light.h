#pragma once

#include <osg/Vec3>

//-------------------------------------------------------------------------------------------------------------------------
namespace ImageMetrics
{

//-------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------

class Light
{
public:
  Light() {}
  ~Light() {}

  void SetPos(const osg::Vec4 &p) { m_pos= p; }
  const osg::Vec4 &GetPos() const { return m_pos; }

  void SetDiffuse(const osg::Vec3 &d) { m_diffuse= d; }
  const osg::Vec3 &GetDiffuse() const { return m_diffuse; }

  void SetSpecular(const osg::Vec3 &s) { m_specular= s; }
  const osg::Vec3 &GetSpecular() const { return m_specular; }

  void SetAmbient(const osg::Vec3 &a) { m_ambient= a; }
  const osg::Vec3 &GetAmbient() const { return m_ambient; }

  void SetPower(float p) {m_power = p;}
  float GetPower() const { return m_power; }

private:
  
  std::string m_type;
  osg::Vec4 m_pos;
  osg::Vec3 m_diffuse;
  osg::Vec3 m_specular;
  osg::Vec3 m_ambient;
  float m_power;

};

//-------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------

}		// namespace ImageMetrics



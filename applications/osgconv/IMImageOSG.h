#pragma once

#include <osg/Image>

namespace osg {


/** Image class for encapsulating the storage texture image data. */
class OSG_EXPORT IMImageOSG : public Image
{
  public :

    IMImageOSG():Image(){}

    /** Copy constructor using CopyOp to manage deep vs shallow copy. */
    IMImageOSG(const IMImageOSG& image,const CopyOp& copyop=CopyOp::SHALLOW_COPY):Image(image, copyop){}
  
    virtual Object* cloneType() const { return new IMImageOSG(); }
    virtual Object* clone(const CopyOp& copyop) const { return new IMImageOSG(*this,copyop); }
    virtual bool isSameKindAs(const Object* obj) const { return dynamic_cast<const IMImageOSG*>(obj)!=0; }
    virtual const char* libraryName() const { return "osg"; }
    virtual const char* className() const { return "IMImageOSG"; }

    virtual osg::Image* asImage() { return this; }
    virtual const osg::Image* asImage() const { return this; }
  
};
}

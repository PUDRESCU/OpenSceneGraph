#pragma once

#include <osg/Texture2D>
#include "IMImageOSG.h"

namespace osg {


/** Image class for encapsulating the storage texture image data. */
class IMTexture2DOSG : public Texture2D
{
  public :

    IMTexture2DOSG();

    IMTexture2DOSG(IMImageOSG* image);


    /** Copy constructor using CopyOp to manage deep vs shallow copy. */
    IMTexture2DOSG(const IMTexture2DOSG& text,const CopyOp& copyop=CopyOp::SHALLOW_COPY);
  
    META_StateAttribute(osg, IMTexture2DOSG, TEXTURE);

    void setIMImageOSG(IMImageOSG* image) { setImage(image); }
  
    template<class T> void setIMImageOSG(const ref_ptr<T>& image) { setImage(image.get()); }

    IMImageOSG* getIMImageOSG() { return dynamic_cast<IMImageOSG*>(_image.get()); }
  
    /** Gets the const texture image. */
    inline const IMImageOSG* getIMImageOSG() const { return dynamic_cast<IMImageOSG*>(_image.get()); }
  
    /** Sets the texture image, ignoring face. */
    virtual void setIMImageOSG(unsigned int, IMImageOSG* image) { setIMImageOSG(image); }
  
    /** Gets the texture image, ignoring face. */
    virtual IMImageOSG* getIMImageOSG(unsigned int) { return dynamic_cast<IMImageOSG*>(_image.get()); }
  
    /** Gets the const texture image, ignoring face. */
    virtual const IMImageOSG* getIMImageOSG(unsigned int) const { return dynamic_cast<IMImageOSG*>(_image.get()); }
  
  private:
  
  protected :
  
    virtual ~IMTexture2DOSG();

  
 
};
}

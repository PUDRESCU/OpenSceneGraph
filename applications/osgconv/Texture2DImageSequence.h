#pragma once

#include <osg/Texture2D>
#include <osg/ImageSequence>

namespace osg {


/** Image class for encapsulating the storage texture image data. */
class Texture2DImageSequence : public Texture2D
{
  public :

    Texture2DImageSequence();

    /** Copy constructor using CopyOp to manage deep vs shallow copy. */
    Texture2DImageSequence(const Texture2DImageSequence& text,const CopyOp& copyop=CopyOp::SHALLOW_COPY);
  
    META_StateAttribute(osg, Texture2DImageSequence, TEXTURE);

    void setImageSequence(ImageSequence* image) { setImage(image); }
  
    template<class T> void setImageSequence(const ref_ptr<T>& image) { setImage(image.get()); }

    ImageSequence* getImageSequence() { return dynamic_cast<ImageSequence*>(_image.get()); }
  
    /** Gets the const texture image. */
    inline const ImageSequence* getImageSequence() const { return dynamic_cast<ImageSequence*>(_image.get()); }
  
    /** Sets the texture image, ignoring face. */
    virtual void setImageSequence(unsigned int, ImageSequence* image) { setImageSequence(image); }
  
    /** Gets the texture image, ignoring face. */
    virtual ImageSequence* getImageSequence(unsigned int) { return dynamic_cast<ImageSequence*>(_image.get()); }
  
    /** Gets the const texture image, ignoring face. */
    virtual const ImageSequence* getImageSequence(unsigned int) const { return dynamic_cast<ImageSequence*>(_image.get()); }
  
  private:
  
  protected :
  
    virtual ~Texture2DImageSequence();
};
}

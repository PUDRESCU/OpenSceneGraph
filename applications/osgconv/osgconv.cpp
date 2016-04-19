#include <stdio.h>
#include <algorithm>
#include <string>
#include <unistd.h>
//#include <io.h>   // For access().
#include <sys/types.h>  // For stat().
#include <sys/stat.h>   // For stat().
#include <cstdlib>

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
#include <osg/ImageSequence>

#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>
#include <osgDB/ReaderWriter>
#include <osgDB/PluginQuery>

#include <osgUtil/Optimizer>
#include <osgUtil/Simplifier>
#include <osgUtil/SmoothingVisitor>

#include <osgViewer/GraphicsWindow>
#include <osgViewer/Version>

#include <iostream>

#include "OrientationConverter.h"
#include "ShaderBase.h"
#include "Light.h"
#include "Material.h"
#include "ShaderManager.h"
#include "MaterialParameterNode.h"
#include "ShaderConstants.h"
#include "TriggerParameterNode.h"
#include "NodeVisitorHelper.h"
#include "IMImageOSG.h"
#include "IMTexture2DOSG.h"
#include "BackgroundRenderPass.hpp"
#include "AlignmentInfoNode.h"
#include "OverlayRenderPass.hpp"
#include "Texture2DImageSequence.h"

typedef std::vector<std::string> FileNameList;

std::map<std::string, ImageMetrics::ShaderBase*> gShaderLibrary;


//REGISTER_OBJECT_WRAPPER(StateAttributeCallback_WRAPPER,
//                        new osg::StateAttributeCallback,
//                        osg::StateAttributeCallback,
//                        "osg::Object osg::Callback osg::StateAttributeCallback" )
//{
//}


class MyGraphicsContext {
    public:
        MyGraphicsContext()
        {
            osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
            traits->x = 0;
            traits->y = 0;
            traits->width = 1;
            traits->height = 1;
            traits->windowDecoration = false;
            traits->doubleBuffer = false;
            traits->sharedContext = 0;
            traits->pbuffer = true;

            _gc = osg::GraphicsContext::createGraphicsContext(traits.get());

            if (!_gc)
            {
                osg::notify(osg::NOTICE)<<"Failed to create pbuffer, failing back to normal graphics window."<<std::endl;

                traits->pbuffer = false;
                _gc = osg::GraphicsContext::createGraphicsContext(traits.get());
            }

            if (_gc.valid())
            {
                _gc->realize();
                _gc->makeCurrent();
                if (dynamic_cast<osgViewer::GraphicsWindow*>(_gc.get()))
                {
                    std::cout<<"Realized graphics window for OpenGL operations."<<std::endl;
                }
                else
                {
                    std::cout<<"Realized pbuffer for OpenGL operations."<<std::endl;
                }
            }
        }

        bool valid() const { return _gc.valid() && _gc->isRealized(); }

    private:
        osg::ref_ptr<osg::GraphicsContext> _gc;
};

class DefaultNormalsGeometryVisitor
    : public osg::NodeVisitor
{
public:

    DefaultNormalsGeometryVisitor()
        : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ) {
    }

    virtual void apply( osg::Geode & geode )
    {
        for( unsigned int ii = 0; ii < geode.getNumDrawables(); ++ii )
        {
            osg::ref_ptr< osg::Geometry > geometry = dynamic_cast< osg::Geometry * >( geode.getDrawable( ii ) );
            if( geometry.valid() )
            {
                osg::ref_ptr< osg::Vec3Array > newnormals = new osg::Vec3Array;
                newnormals->push_back( osg::Z_AXIS );
                geometry->setNormalArray( newnormals.get(), osg::Array::BIND_OVERALL );
            }
        }
    }

    virtual void apply( osg::Node & node )
    {
        traverse( node );
    }

};

class CompressTexturesVisitor : public osg::NodeVisitor
{
public:

    CompressTexturesVisitor(osg::Texture::InternalFormatMode internalFormatMode):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _internalFormatMode(internalFormatMode) {}

    virtual void apply(osg::Node& node)
    {
        if (node.getStateSet()) apply(*node.getStateSet());
        traverse(node);
    }

    virtual void apply(osg::Geode& node)
    {
        if (node.getStateSet()) apply(*node.getStateSet());

        for(unsigned int i=0;i<node.getNumDrawables();++i)
        {
            osg::Drawable* drawable = node.getDrawable(i);
            if (drawable && drawable->getStateSet()) apply(*drawable->getStateSet());
        }

        traverse(node);
    }

    virtual void apply(osg::StateSet& stateset)
    {
        // search for the existence of any texture object attributes
        for(unsigned int i=0;i<stateset.getTextureAttributeList().size();++i)
        {
            osg::Texture* texture = dynamic_cast<osg::Texture*>(stateset.getTextureAttribute(i,osg::StateAttribute::TEXTURE));
            if (texture)
            {
                _textureSet.insert(texture);
            }
        }
    }

    void compress()
    {
        MyGraphicsContext context;
        if (!context.valid())
        {
            osg::notify(osg::NOTICE)<<"Error: Unable to create graphis context, problem with running osgViewer-"<<osgViewerGetVersion()<<", cannot run compression."<<std::endl;
            return;
        }

        osg::ref_ptr<osg::State> state = new osg::State;
        state->initializeExtensionProcs();

        for(TextureSet::iterator itr=_textureSet.begin();
            itr!=_textureSet.end();
            ++itr)
        {
            osg::Texture* texture = const_cast<osg::Texture*>(itr->get());

            osg::Texture2D* texture2D = dynamic_cast<osg::Texture2D*>(texture);
            osg::Texture3D* texture3D = dynamic_cast<osg::Texture3D*>(texture);

            osg::ref_ptr<osg::Image> image = texture2D ? texture2D->getImage() : (texture3D ? texture3D->getImage() : 0);
            if (image.valid() &&
                (image->getPixelFormat()==GL_RGB || image->getPixelFormat()==GL_RGBA) &&
                (image->s()>=32 && image->t()>=32))
            {
                texture->setInternalFormatMode(_internalFormatMode);

                // need to disable the unref after apply, otherwise the image could go out of scope.
                bool unrefImageDataAfterApply = texture->getUnRefImageDataAfterApply();
                texture->setUnRefImageDataAfterApply(false);

                // get OpenGL driver to create texture from image.
                texture->apply(*state);

                // restore the original setting
                texture->setUnRefImageDataAfterApply(unrefImageDataAfterApply);

                image->readImageFromCurrentTexture(0,true);

                texture->setInternalFormatMode(osg::Texture::USE_IMAGE_DATA_FORMAT);
            }
        }
    }

    void write(const std::string &dir)
    {
        for(TextureSet::iterator itr=_textureSet.begin();
            itr!=_textureSet.end();
            ++itr)
        {
            osg::Texture* texture = const_cast<osg::Texture*>(itr->get());

            osg::Texture2D* texture2D = dynamic_cast<osg::Texture2D*>(texture);
            osg::Texture3D* texture3D = dynamic_cast<osg::Texture3D*>(texture);

            osg::ref_ptr<osg::Image> image = texture2D ? texture2D->getImage() : (texture3D ? texture3D->getImage() : 0);
            if (image.valid())
            {
                std::string name = osgDB::getStrippedName(image->getFileName());
                name += ".dds";
                image->setFileName(name);
                std::string path = dir.empty() ? name : osgDB::concatPaths(dir, name);
                osgDB::writeImageFile(*image, path);
                osg::notify(osg::NOTICE) << "Image written to '" << path << "'." << std::endl;
            }
        }
    }

    typedef std::set< osg::ref_ptr<osg::Texture> > TextureSet;
    TextureSet                          _textureSet;
    osg::Texture::InternalFormatMode    _internalFormatMode;

};


class FixTransparencyVisitor : public osg::NodeVisitor
{
public:

    enum FixTransparencyMode
    {
        NO_TRANSPARANCY_FIXING,
        MAKE_OPAQUE_TEXTURE_STATESET_OPAQUE,
        MAKE_ALL_STATESET_OPAQUE
    };

    FixTransparencyVisitor(FixTransparencyMode mode=MAKE_OPAQUE_TEXTURE_STATESET_OPAQUE):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _numTransparent(0),
        _numOpaque(0),
        _numTransparentMadeOpaque(0),
        _mode(mode)
    {
        std::cout<<"Running FixTransparencyVisitor..."<<std::endl;
    }

    ~FixTransparencyVisitor()
    {
        std::cout<<"  Number of Transparent StateSet "<<_numTransparent<<std::endl;
        std::cout<<"  Number of Opaque StateSet "<<_numOpaque<<std::endl;
        std::cout<<"  Number of Transparent State made Opaque "<<_numTransparentMadeOpaque<<std::endl;
    }

    virtual void apply(osg::Node& node)
    {
        if (node.getStateSet()) isTransparent(*node.getStateSet());
        traverse(node);
    }

    virtual void apply(osg::Geode& node)
    {
        if (node.getStateSet()) isTransparent(*node.getStateSet());

        for(unsigned int i=0;i<node.getNumDrawables();++i)
        {
            osg::Drawable* drawable = node.getDrawable(i);
            if (drawable && drawable->getStateSet()) isTransparent(*drawable->getStateSet());
        }

        traverse(node);
    }

    virtual bool isTransparent(osg::StateSet& stateset)
    {
        bool hasTranslucentTexture = false;
        bool hasBlendFunc = dynamic_cast<osg::BlendFunc*>(stateset.getAttribute(osg::StateAttribute::BLENDFUNC))!=0;
        bool hasTransparentRenderingHint = stateset.getRenderingHint()==osg::StateSet::TRANSPARENT_BIN;
        bool hasDepthSortBin = (stateset.getRenderBinMode()==osg::StateSet::USE_RENDERBIN_DETAILS)?(stateset.getBinName()=="DepthSortedBin"):false;
        bool hasTexture = false;


        // search for the existence of any texture object attributes
        for(unsigned int i=0;i<stateset.getTextureAttributeList().size();++i)
        {
            osg::Texture* texture = dynamic_cast<osg::Texture*>(stateset.getTextureAttribute(i,osg::StateAttribute::TEXTURE));
            if (texture)
            {
                hasTexture = true;
                for (unsigned int im=0;im<texture->getNumImages();++im)
                {
                    osg::Image* image = texture->getImage(im);
                    if (image && image->isImageTranslucent()) hasTranslucentTexture = true;
                }
            }
        }

        if (hasTranslucentTexture || hasBlendFunc || hasTransparentRenderingHint || hasDepthSortBin)
        {
            ++_numTransparent;

            bool makeNonTransparent = false;

            switch(_mode)
            {
            case(MAKE_OPAQUE_TEXTURE_STATESET_OPAQUE):
                if (hasTexture && !hasTranslucentTexture)
                {
                    makeNonTransparent = true;
                }
                break;
            case(MAKE_ALL_STATESET_OPAQUE):
                makeNonTransparent = true;
                break;
            default:
                makeNonTransparent = false;
                break;
            }

            if (makeNonTransparent)
            {
                stateset.removeAttribute(osg::StateAttribute::BLENDFUNC);
                stateset.removeMode(GL_BLEND);
                stateset.setRenderingHint(osg::StateSet::DEFAULT_BIN);
                ++_numTransparentMadeOpaque;
            }


            return true;
        }
        else
        {
            ++_numOpaque;
            return false;
        }
    }

    unsigned int _numTransparent;
    unsigned int _numOpaque;
    unsigned int _numTransparentMadeOpaque;
    FixTransparencyMode _mode;
};

class PruneStateSetVisitor : public osg::NodeVisitor
{
public:

    PruneStateSetVisitor():
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _numStateSetRemoved(0)
    {
        std::cout<<"Running PruneStateSet..."<<std::endl;
    }

    ~PruneStateSetVisitor()
    {
        std::cout<<"  Number of StateState removed "<<_numStateSetRemoved<<std::endl;
    }

    virtual void apply(osg::Node& node)
    {
        if (node.getStateSet())
        {
            node.setStateSet(0);
            ++_numStateSetRemoved;
        }
        traverse(node);
    }

    virtual void apply(osg::Geode& node)
    {
        if (node.getStateSet())
        {
            node.setStateSet(0);
            ++_numStateSetRemoved;
        }

        traverse(node);
    }

    unsigned int _numStateSetRemoved;
};

/** Add missing colours to osg::Geometry.*/
class AddMissingColoursToGeometryVisitor : public osg::NodeVisitor
{
public:

    AddMissingColoursToGeometryVisitor():osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}

    virtual void apply(osg::Geode& geode)
    {
        for(unsigned int i=0;i<geode.getNumDrawables();++i)
        {
            osg::Geometry* geometry = dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
            if (geometry)
            {
                if (geometry->getColorArray()==0 || geometry->getColorArray()->getNumElements()==0)
                {
                    osg::Vec4Array* colours = new osg::Vec4Array(1);
                    (*colours)[0].set(1.0f,1.0f,1.0f,1.0f);
                    geometry->setColorArray(colours, osg::Array::BIND_OVERALL);
                }
            }
        }
    }

    virtual void apply(osg::Node& node) { traverse(node); }

};


static void usage( const char *prog, const char *msg )
{
    if (msg)
    {
        osg::notify(osg::NOTICE)<< std::endl;
        osg::notify(osg::NOTICE) << msg << std::endl;
    }

    // basic usage
    osg::notify(osg::NOTICE)<< std::endl;
    osg::notify(osg::NOTICE)<<"usage:"<< std::endl;
    osg::notify(osg::NOTICE)<<"    " << prog << " [options] infile1 [infile2 ...] outfile"<< std::endl;
    osg::notify(osg::NOTICE)<< std::endl;

    // print env options - especially since optimizer is always _on_
    osg::notify(osg::NOTICE)<<"environment:" << std::endl;
    osg::ApplicationUsage::UsageMap um = osg::ApplicationUsage::instance()->getEnvironmentalVariables();
    std::string envstring;
    osg::ApplicationUsage::instance()->getFormattedString( envstring, um );
    osg::notify(osg::NOTICE)<<envstring << std::endl;

    // print tool options
    osg::notify(osg::NOTICE)<<"options:"<< std::endl;
    osg::notify(osg::NOTICE)<<"    -O option          - ReaderWriter option"<< std::endl;
    osg::notify(osg::NOTICE)<< std::endl;
    osg::notify(osg::NOTICE)<<"    --compressed       - Enable the usage of compressed textures,"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         defaults to OpenGL ARB compressed textures."<< std::endl;
    osg::notify(osg::NOTICE)<<"    --compressed-arb   - Enable the usage of OpenGL ARB compressed textures"<< std::endl;
    osg::notify(osg::NOTICE)<<"    --compressed-dxt1  - Enable the usage of S3TC DXT1 compressed textures"<< std::endl;
    osg::notify(osg::NOTICE)<<"    --compressed-dxt3  - Enable the usage of S3TC DXT3 compressed textures"<< std::endl;
    osg::notify(osg::NOTICE)<<"    --compressed-dxt5  - Enable the usage of S3TC DXT5 compressed textures"<< std::endl;
    osg::notify(osg::NOTICE)<< std::endl;
    osg::notify(osg::NOTICE)<<"    --fix-transparency - fix statesets which are currently"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         declared as transparent, but should be opaque."<< std::endl;
    osg::notify(osg::NOTICE)<<"                         Defaults to using the fixTranspancyMode"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         MAKE_OPAQUE_TEXTURE_STATESET_OPAQUE."<< std::endl;
    osg::notify(osg::NOTICE)<<"    --fix-transparency-mode <mode_string>  - fix statesets which are currently"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         declared as transparent but should be opaque."<< std::endl;
    osg::notify(osg::NOTICE)<<"                         The mode_string determines which algorithm is used"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         to fix the transparency, options are:"<< std::endl;
    osg::notify(osg::NOTICE)<<"                                 MAKE_OPAQUE_TEXTURE_STATESET_OPAQUE,"<<std::endl;
    osg::notify(osg::NOTICE)<<"                                 MAKE_ALL_STATESET_OPAQUE."<<std::endl;

    osg::notify(osg::NOTICE)<< std::endl;
    osg::notify(osg::NOTICE)<<"    -l libraryName     - load plugin of name libraryName"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         i.e. -l osgdb_pfb"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         Useful for loading reader/writers which can load"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         other file formats in addition to its extension."<< std::endl;
    osg::notify(osg::NOTICE)<<"    -e extensionName   - load reader/wrter plugin for file extension"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         i.e. -e pfb"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         Useful short hand for specifying full library name as"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         done with -l above, as it automatically expands to the"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         full library name appropriate for each platform."<< std::endl;
    osg::notify(osg::NOTICE)<<"    -o orientation     - Convert geometry from input files to output files."<< std::endl;
    osg::notify(osg::NOTICE)<<
                              "                         Format of orientation argument must be the following:\n"
                              "\n"
                              "                             X1,Y1,Z1-X2,Y2,Z2\n"
                              "                         or\n"
                              "                             degrees-A0,A1,A2\n"
                              "\n"
                              "                         where X1,Y1,Z1 represent the UP vector in the input\n"
                              "                         files and X2,Y2,Z2 represent the UP vector of the\n"
                              "                         output file, or degrees is the rotation angle in\n"
                              "                         degrees around axis (A0,A1,A2).  For example, to\n"
                              "                         convert a model built in a Y-Up coordinate system to a\n"
                              "                         model with a Z-up coordinate system, the argument may\n"
                              "                         look like\n"
                              "\n"
                              "                             0,1,0-0,0,1"
                              "\n"
                              "                          or\n"
                              "                             -90-1,0,0\n"
                              "\n" << std::endl;
    osg::notify(osg::NOTICE)<<"    -t translation     - Convert spatial position of output files.  Format of\n"
                              "                         translation argument must be the following :\n"
                              "\n"
                              "                             X,Y,Z\n"
                              "\n"
                              "                         where X, Y, and Z represent the coordinates of the\n"
                              "                         absolute position in world space\n"
                              << std::endl;
    osg::notify(osg::NOTICE)<<"    --use-world-frame  - Perform transformations in the world frame, rather\n"
                              "                         than relative to the center of the bounding sphere.\n"
                              << std::endl;
    osg::notify(osg::NOTICE)<<"    --simplify n       - Run simplifier prior to output. Argument must be a" << std::endl
                            <<"                         normalized value for the resultant percentage" << std::endl
                            <<"                         reduction." << std::endl
                            <<"                         Example: --simplify .5" << std::endl
                            <<"                                 will produce a 50% reduced model." << std::endl
                            << std::endl;
    osg::notify(osg::NOTICE)<<"    -s scale           - Scale size of model.  Scale argument must be the \n"
                              "                         following :\n"
                              "\n"
                              "                             SX,SY,SZ\n"
                              "\n"
                              "                         where SX, SY, and SZ represent the scale factors\n"
                              "                         Caution: Scaling is done in destination orientation\n"
                              << std::endl;
    osg::notify(osg::NOTICE)<<"    --smooth           - Smooth the surface by regenerating surface normals on\n"
                              "                         all geometry nodes"<< std::endl;
    osg::notify(osg::NOTICE)<<"    --addMissingColors - Add a white color value to all geometry nodes\n"
                              "                         that don't have their own color values\n"
                              "                         (--addMissingColours also accepted)."<< std::endl;
    osg::notify(osg::NOTICE)<<"    --overallNormal    - Replace normals with a single overall normal."<< std::endl;
    osg::notify(osg::NOTICE)<<"    --enable-object-cache - Enable caching of objects, images, etc."<< std::endl;

    osg::notify( osg::NOTICE ) << std::endl;
    osg::notify( osg::NOTICE ) <<
        "    --formats          - List all supported formats and their supported options." << std::endl;
    osg::notify( osg::NOTICE ) <<
        "    --format <format>  - Display information about the specified <format>,\n"
        "                         where <format> is the file extension, such as \"flt\"." << std::endl;
    osg::notify( osg::NOTICE ) <<
        "    --plugins          - List all supported plugin files." << std::endl;
    osg::notify( osg::NOTICE ) <<
        "    --plugin <plugin>  - Display information about the specified <plugin>,\n"
        "                         where <plugin> is the plugin's full path and file name." << std::endl;
    osg::notify( osg::NOTICE ) <<
        "    --param <param>    - Add additional parameters,\n"
        "                         where <param> is the parameter file's full path and file name." << std::endl;
    osg::notify( osg::NOTICE ) <<
        "    --device           - create depolyment file for mobile device."<< std::endl;
    osg::notify( osg::NOTICE ) <<
        "    --width  <width>   - set viewport width default is 720."<< std::endl;
    osg::notify( osg::NOTICE ) <<
        "    --height <height>  - set viewport height default is 1280."<< std::endl;
  
    osg::notify( osg::NOTICE ) <<
        "    --addOverlay       - add an overlay layer."<< std::endl;
}

void replaceShaderMaterial(osg::ref_ptr<osg::Node> root)
{
  // Init shader library
  //gShaderLibrary["GLSLShader1"] = new ImageMetrics::ShaderBrix();
  
  for(std::map<std::string, ImageMetrics::ShaderBase*>::iterator itr = gShaderLibrary.begin(); itr != gShaderLibrary.end(); itr++)
  {
    GetAttributeByNameVisitor visitor(itr->first);
    root->accept(visitor);
    osg::ref_ptr<osg::Drawable> findNode = visitor.getResult();
    if (findNode)
    {
      osg::ref_ptr<osg::StateSet> stateSet = findNode->getOrCreateStateSet();
      
      // Get standard materail parameters
      osg::ref_ptr<osg::Material> material = dynamic_cast<osg::Material*>(stateSet->getAttribute(osg::StateAttribute::MATERIAL));
      if(material)
      {
        osg::notify(osg::NOTICE)<<material->getName()<<std::endl;

        osg::Vec4 ambient = material->getAmbient(osg::Material::FRONT);
        osg::Vec4 diffuse = material->getDiffuse(osg::Material::FRONT);
        osg::Vec4 specular = material->getSpecular(osg::Material::FRONT);
        osg::Vec4 emission = material->getEmission(osg::Material::FRONT);
        float shiness = material->getShininess(osg::Material::FRONT);
        osg::notify(osg::NOTICE)<<"Material parameter: "<<std::endl;
        osg::notify(osg::NOTICE)<<"ambient: "<<ambient.r()<<", "<<ambient.g()<<", "<<ambient.b()<<", "<<ambient.a()<<std::endl;
        osg::notify(osg::NOTICE)<<"diffuse: "<<diffuse.r()<<", "<<diffuse.g()<<", "<<diffuse.b()<<", "<<diffuse.a()<<std::endl;
        osg::notify(osg::NOTICE)<<"specular: "<<specular.r()<<", "<<specular.g()<<", "<<specular.b()<<", "<<specular.a()<<std::endl;
        osg::notify(osg::NOTICE)<<"emission: "<<emission.r()<<", "<<emission.g()<<", "<<emission.b()<<", "<<emission.a()<<std::endl;
        osg::notify(osg::NOTICE)<<"shiness: "<<shiness<<std::endl;
      }
      
      // Remove old material
      stateSet->removeAttribute(osg::StateAttribute::MATERIAL);
      osg::notify(osg::NOTICE)<<findNode->getName()<<std::endl;
      
      itr->second->PrepareToRender(stateSet);
     
      //Add uniforms
      for(std::map<std::string, osg::ref_ptr<osg::Uniform> >::iterator unifomr_itr = itr->second->m_UniformMap.begin();
          unifomr_itr != itr->second->m_UniformMap.end(); unifomr_itr++)
      {
        stateSet->addUniform(unifomr_itr->second);
      }
    }
  }
  
  
  for(std::map<std::string, ImageMetrics::ShaderBase*>::iterator itr = gShaderLibrary.begin(); itr != gShaderLibrary.end(); itr++)
  {
    delete itr->second;
  }
}

//void addAlignmentInfo(osg::ref_ptr<osg::Node> root)
//{
//  osg::ref_ptr<osg::Node> alignmentGroup = new osg::Group;
//  alignmentGroup->setName("alignmentinfo");
//}


//void prepareNormalMap(osg::ref_ptr<osg::Node> root)
//{
//  //FIXME: find the node need to add normal map
//  FindNodeVisitor findNodeVisitor("facemask");
//  root->accept(findNodeVisitor);
//  
//  osg::Node* normalMapNode = findNodeVisitor.getFirst();
//  if(normalMapNode != NULL)
//  {
//    TsgVisitor tsgVisitor;
//    normalMapNode->accept(tsgVisitor);
//  }
//}


// Post processing:
osg::ref_ptr<osg::Node> postProcess(osg::ref_ptr<osg::Node> root, bool isDeviceDeployment, int width, int height)
{
  if(isDeviceDeployment)
  {
    osg::ref_ptr<osg::Node> rootGroup = new osg::Group;
    
    osg::ref_ptr<osg::IMImageOSG> inputImage = new osg::IMImageOSG();
    osg::ref_ptr<osg::IMTexture2DOSG> inputTexture = new osg::IMTexture2DOSG;
    inputTexture->setDataVariance(osg::Object::DYNAMIC);
    inputTexture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
    inputTexture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    inputTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    inputTexture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    inputTexture->setIMImageOSG(inputImage);
    inputTexture->setResizeNonPowerOfTwoHint(false);
    
    // Add background render pass
    ImageMetrics::BackgroundRenderPass* backgroundRenderPass = new ImageMetrics::BackgroundRenderPass(inputTexture, width, height, NULL);
    
    osg::ref_ptr<osg::Camera> backgroundCamera = backgroundRenderPass->getCamera();
    rootGroup->asGroup()->addChild(backgroundCamera.get());
    
    // Add scene render pass
    osg::ref_ptr<osg::Camera> sceneCamera = new osg::Camera;
    sceneCamera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
    sceneCamera->setRenderOrder(osg::Camera::PRE_RENDER);
    sceneCamera->setProjectionMatrix(osg::Matrixd::identity());
//    sceneCamera->setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_FAR_USING_PRIMITIVES);
//    sceneCamera->setNearFarRatio(1e-6);
    sceneCamera->setDataVariance(osg::Object::DYNAMIC);
    sceneCamera->setAllowEventFocus(false);
    sceneCamera->setViewport(0, 0, width, height);
    sceneCamera->setClearMask(GL_DEPTH_BUFFER_BIT);
    sceneCamera->addChild(root.get());
    rootGroup->asGroup()->addChild(sceneCamera.get());
    
    RemoveGLModeForGLESVisitor removeGLModeForGLESVisitor;
    rootGroup->accept(removeGLModeForGLESVisitor);
    
    //addAlignmentInfo(rootGroup);
    
    delete backgroundRenderPass;
    return rootGroup;
  }

  RemoveGLModeForGLESVisitor removeGLModeForGLESVisitor;
  root->accept(removeGLModeForGLESVisitor);
  return root;
}

// add overlay layer
osg::ref_ptr<osg::Node> addOverlay(osg::ref_ptr<osg::Node> root, osg::ref_ptr<osg::Group> params, bool isDeviceDeployment, int width, int height)
{
  const int MAX_OVERLAY_NUM = 10;
  
  // Find overlay layers
  std::vector<osg::Node*> overlays;
  for(int i = 0; i < MAX_OVERLAY_NUM; i++)
  {
    std::stringstream ss;
    ss <<"overlay"<<i;
    FindNodeVisitor findNodeVisitor(ss.str());
    root->accept(findNodeVisitor);
  
    std::vector<osg::Node*> nodelist = findNodeVisitor.getNodeList();
    if(!nodelist.empty())
    {
      for(int i = nodelist.size()-1; i >= 0; i--)
      {
        osg::MatrixTransform* transform = dynamic_cast<osg::MatrixTransform*>(nodelist[i]);

        if(transform)
        {
          overlays.push_back(nodelist[i]);
          break;
        }
      }
    }
  }
  
  osg::notify(osg::NOTICE)<<"Num of overlays: "<<overlays.size()<< std::endl;

 
  if(params != NULL)
  {
   
    osg::notify(osg::NOTICE)<<"Num of params: "<<params->getNumChildren()<< std::endl;

    
    osg::Group* materialParams = NULL;
    for(int i = 0; i < params->getNumChildren(); i++)
    {
      if(params->getChild(i)->getName() == "material")
      {
        materialParams = dynamic_cast<osg::Group*>(params->getChild(i));
        break;
      }
    }
    
    if(materialParams != NULL)
    {
      for(size_t i = 0; i < overlays.size(); i++)
      {
        for(int j = 0; j < materialParams->getNumChildren(); j++)
        {
          osg::ref_ptr<osg::MaterialParameterNode> oneParam = dynamic_cast<osg::MaterialParameterNode*>(materialParams->getChild(j));
          osg::notify(osg::NOTICE)<<"find overlay parameter: parameter name:"<<oneParam->getName()<<", overlay name:"<<overlays[i]->getName()<<std::endl;

          if(oneParam->getName() == overlays[i]->getName() && !oneParam->getDiffuseImageSequence().empty())
          {
            osg::ref_ptr<osg::ImageSequence> imageSequence = new osg::ImageSequence;
            imageSequence->setMode(osg::ImageSequence::PAGE_AND_DISCARD_USED_IMAGES);
            double length = -1.0;
            double fps = oneParam->getAnimationFPS();
            
            for(size_t k = 0; k < oneParam->getDiffuseImageSequence().size(); k++)
            {
              imageSequence->addImageFile(oneParam->getDiffuseImageSequence()[k]);
            }
            
            if(length > 0.0)
            {
              imageSequence->setLength(length);
            }
            else
            {
              unsigned int maxNum = imageSequence->getNumImageData();
              imageSequence->setLength(double(maxNum)*(1.0/fps));
            }
            
            //start play image sequence
            if(oneParam->getShouldPlayAnimation())
            {
              imageSequence->play();
            }
            
            imageSequence->setLoopingMode(oneParam->getLoopingMode());
            
            osg::notify(osg::NOTICE)<<"looping mode: "<<oneParam->getLoopingMode()<< std::endl;

            osg::ref_ptr<osg::Texture2DImageSequence> inputTexture = new osg::Texture2DImageSequence;
            inputTexture->setDataVariance(osg::Object::DYNAMIC);
            inputTexture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
            inputTexture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
            inputTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
            inputTexture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
            inputTexture->setImageSequence(imageSequence.get());
            inputTexture->setResizeNonPowerOfTwoHint(false);
            
            // Add overlay render pass
            ImageMetrics::OverlayRenderPass* overlayRenderPass = new ImageMetrics::OverlayRenderPass(isDeviceDeployment, inputTexture, width, height);
            osg::ref_ptr<osg::Node> geode = overlayRenderPass->getGeode();
            geode->setName(overlays[i]->getName());
            overlays[i]->asGroup()->removeChildren(0, overlays[i]->asGroup()->getNumChildren());
            overlays[i]->asGroup()->addChild(geode.get());
            //root->asGroup()->addChild(overlayCamera.get());
            delete overlayRenderPass;
            break;
          }
        }

      }
    }
  }
  
  // Re-sort renderbins:
  FindGeodeVisitor findGeodeVisitor;
  root->accept(findGeodeVisitor);
  std::vector<osg::ref_ptr<osg::Geode> > geodes = findGeodeVisitor.getGeodes();
  for(size_t i = 0; i < geodes.size(); i++)
  {
    std::string lowerstr;
    std::string name = geodes[i]->getName();
    lowerstr.resize(name.size());
    std::transform(name.begin(), name.end(), lowerstr.begin(), ::tolower);
    
    // Only update render order except occlusion mesh
//    if(lowerstr.find("occlusionmesh") == std::string::npos)
//    {
//      osg::StateSet* stateSet =  geodes[i]->getOrCreateStateSet();
//      stateSet->setRenderBinDetails(i+1, "RenderBin");
//    }
//    else
//    {
//      osg::StateSet* stateSet =  geodes[i]->getOrCreateStateSet();
//      stateSet->setRenderBinDetails(0, "RenderBin");
//    }
    if(lowerstr.find("occlusionmesh") != std::string::npos)
    {
      for (int j = 0; j < geodes[i]->getNumDrawables(); j++)
      {
        osg::Drawable* dr = geodes[i]->getDrawable(j);
        
        osg::StateSet* stateSet = dr->getStateSet();
        
        if(stateSet)
        {
          osg::notify(osg::NOTICE)<<"set occlusion mesh renderbin"<< std::endl;

          stateSet->setRenderBinDetails(1, "RenderBin");
        }
      }
    }
  }
  
  /*
  if(isAdd)
  {
    osg::notify(osg::NOTICE)<<"Num of params: "<<params->getNumChildren()<< std::endl;

    if(params == NULL)
    {
      return root;
    }
    
    osg::Group* materialParams = NULL;
    for(int i = 0; i < params->getNumChildren(); i++)
    {
      if(params->getChild(i)->getName() == "material")
      {
        materialParams = dynamic_cast<osg::Group*>(params->getChild(i));
        break;
      }
    }
    
    if(materialParams != NULL)
    {
      for(int i = 0; i < materialParams->getNumChildren(); i++)
      {
        osg::ref_ptr<osg::MaterialParameterNode> oneParam = dynamic_cast<osg::MaterialParameterNode*>(materialParams->getChild(i));
        if(oneParam->getName() == "overlaylayer2d" && !oneParam->getDiffuseImageSequence().empty())
        {
          osg::ref_ptr<osg::ImageSequence> imageSequence = new osg::ImageSequence;
          imageSequence->setMode(osg::ImageSequence::PAGE_AND_RETAIN_IMAGES);
          double length = -1.0;
          double fps = 10.0;
          
          for(size_t i = 0; i < oneParam->getDiffuseImageSequence().size(); i++)
          {
            osg::notify(osg::NOTICE)<<"LoadImageSequenceUniform: "<<oneParam->getDiffuseImageSequence()[i]<<std::endl;
            imageSequence->addImageFile(oneParam->getDiffuseImageSequence()[i]);
          }
          
          if(length > 0.0)
          {
            imageSequence->setLength(length);
          }
          else
          {
            unsigned int maxNum = imageSequence->getNumImageData();
            imageSequence->setLength(double(maxNum)*(1.0/fps));
          }
          
          //start play image sequence
          imageSequence->play();
          
          osg::ref_ptr<osg::Texture2DImageSequence> inputTexture = new osg::Texture2DImageSequence;
          inputTexture->setDataVariance(osg::Object::DYNAMIC);
          inputTexture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
          inputTexture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
          inputTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
          inputTexture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
          inputTexture->setImageSequence(imageSequence.get());
          inputTexture->setResizeNonPowerOfTwoHint(false);
          
          // Add overlay render pass
          ImageMetrics::OverlayRenderPass* overlayRenderPass = new ImageMetrics::OverlayRenderPass(inputTexture, width, height);
          
          osg::ref_ptr<osg::Camera> overlayCamera = overlayRenderPass->getCamera();
          root->asGroup()->addChild(overlayCamera.get());
          
          delete overlayRenderPass;
          break;
        }
      }
    }
  }
  */
  return root;
}


void readAdditionalParameters(const std::string& fileName, osg::ref_ptr<osg::Group>& params)
{
  //        osg::ref_ptr<osg::MaterialParameterNode> node = new osg::MaterialParameterNode;
  //        osgDB::writeNodeFile( *node, "examplenode.osgt" );
  
  params = dynamic_cast<osg::Group*>(osgDB::readNodeFile(fileName));
}

void setupMatrixUniform(osg::ref_ptr<osg::Node> root)
{
  FindUniformVisitor findUniformVisitor;
  root->accept(findUniformVisitor);
  
  std::map<std::string, std::vector<osg::ref_ptr<osg::Uniform> > > uniformMap = findUniformVisitor.getUniformMap();

  osg::notify(osg::NOTICE)<<"g_UniformMap size: "<<uniformMap.size()<<std::endl;

  
  if(uniformMap.find(ImageMetrics::ShaderConstants::UNIFORM_P) != uniformMap.end())
  {
    for(size_t i = 0; i < uniformMap[ImageMetrics::ShaderConstants::UNIFORM_P].size(); i++)
    {
      uniformMap[ImageMetrics::ShaderConstants::UNIFORM_P][i]->setDataVariance(osg::Object::DYNAMIC);
    }
  }
  
  if(uniformMap.find(ImageMetrics::ShaderConstants::UNIFORM_MVP) != uniformMap.end())
  {
    for(size_t i = 0; i < uniformMap[ImageMetrics::ShaderConstants::UNIFORM_MVP].size(); i++)
    {
      uniformMap[ImageMetrics::ShaderConstants::UNIFORM_MVP][i]->setDataVariance(osg::Object::DYNAMIC);
    }
  }
  
  if(uniformMap.find(ImageMetrics::ShaderConstants::UNIFORM_V) != uniformMap.end())
  {
    for(size_t i = 0; i < uniformMap[ImageMetrics::ShaderConstants::UNIFORM_V].size(); i++)
    {
      uniformMap[ImageMetrics::ShaderConstants::UNIFORM_V][i]->setDataVariance(osg::Object::DYNAMIC);
    }
  }
  
  if(uniformMap.find(ImageMetrics::ShaderConstants::UNIFORM_M) != uniformMap.end())
  {
    for(size_t i = 0; i < uniformMap[ImageMetrics::ShaderConstants::UNIFORM_M].size(); i++)
    {
      uniformMap[ImageMetrics::ShaderConstants::UNIFORM_M][i]->setDataVariance(osg::Object::DYNAMIC);
    }
  }
  
  if(uniformMap.find(ImageMetrics::ShaderConstants::UNIFORM_V_INV) != uniformMap.end())
  {
    for(size_t i = 0; i < uniformMap[ImageMetrics::ShaderConstants::UNIFORM_V_INV].size(); i++)
    {
      uniformMap[ImageMetrics::ShaderConstants::UNIFORM_V_INV][i]->setDataVariance(osg::Object::DYNAMIC);
    }
  }
  
  if(uniformMap.find(ImageMetrics::ShaderConstants::UNIFORM_NORMALMATRIX) != uniformMap.end())
  {
    for(size_t i = 0; i < uniformMap[ImageMetrics::ShaderConstants::UNIFORM_NORMALMATRIX].size(); i++)
    {
      uniformMap[ImageMetrics::ShaderConstants::UNIFORM_NORMALMATRIX][i]->setDataVariance(osg::Object::DYNAMIC);
    }
  }
}

void addTriggerNodes(osg::ref_ptr<osg::Node> root, osg::ref_ptr<osg::Group> params)
{
  if(params == NULL)
  {
    return;
  }
  
  osg::Group* triggerParams = NULL;
  for(int i = 0; i < params->getNumChildren(); i++)
  {
    if(params->getChild(i)->getName() == "trigger")
    {
      triggerParams = dynamic_cast<osg::Group*>(params->getChild(i));
      break;
    }
  }
  
  if(triggerParams != NULL)
  {
    root->asGroup()->addChild(triggerParams);
  }
}

void createPackage(osg::ref_ptr<osg::Node> root, const std::string& absoluteOsgtFileName, bool isPackage)
{
  if(!isPackage)
  {
    return;
  }
  
  osg::notify(osg::NOTICE)<<"===================== STEP: Create output package ====================="<<std::endl;

  std::string curDir = osgDB::getFilePath(absoluteOsgtFileName);
  std::string packageDir = osgDB::concatPaths(curDir, "osg_package");
 
#if defined(WIN32)
  // The packagDir exists, then delete it
  if(_access(packageDir.c_str(), 0) == 0)
  {
    osg::notify(osg::NOTICE)<<"path: "<<packageDir<<" exist"<<std::endl;
    std::string rmCommand = "rd /s /q \""+packageDir+"\"";
    std::system(rmCommand.c_str());
  }
  
  _mkdir(packageDir.c_str());
#else
  // The packagDir exists, then delete it
  if(access(packageDir.c_str(), 0) == 0)
  {
    osg::notify(osg::NOTICE)<<"path: "<<packageDir<<" exist"<<std::endl;
    std::string rmCommand = "rm -fr \""+packageDir+"\"";
    std::system(rmCommand.c_str());
  }
  
  mkdir(packageDir.c_str(), 0777);
#endif
 
  // Find image files and copy them into packge folder
  CopyImageVisitor copyImageVisitor(curDir, packageDir);
  root->accept(copyImageVisitor);

  // save new osgt file
  osgDB::ReaderWriter::WriteResult result = osgDB::Registry::instance()->writeNode(*root,absoluteOsgtFileName,new osgDB::Options("WriteImageHint=UseExternal"));

  // Copy osg file
  std::string cpOsgCommand = "cp \""+absoluteOsgtFileName+"\" \""+packageDir+"\"";
  std::system(cpOsgCommand.c_str());
}



int main( int argc, char **argv )
{
  osg::notify(osg::NOTICE)<<"=====================             START           ====================="<<std::endl;

  // use an ArgumentParser object to manage the program arguments.
  osg::ArgumentParser arguments(&argc,argv);

  // set up the usage document, in case we need to print out how to use this program.
  arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
  arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is a utility for converting between various input and output databases formats.");
  arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
  arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display command line parameters");
  arguments.getApplicationUsage()->addCommandLineOption("--help-env","Display environmental variables available");
  arguments.getApplicationUsage()->addCommandLineOption("--device","Convert to device deployment format");
  arguments.getApplicationUsage()->addCommandLineOption("--package","Create a folder including osg file and images");
  arguments.getApplicationUsage()->addCommandLineOption("--width <width>","viewport width.");
  arguments.getApplicationUsage()->addCommandLineOption("--height <height>","viewport height.");
  //arguments.getApplicationUsage()->addCommandLineOption("--addOverlay","add an Overlay layer.");

  //arguments.getApplicationUsage()->addCommandLineOption("--formats","List supported file formats");
  //arguments.getApplicationUsage()->addCommandLineOption("--plugins","List database olugins");


  // if user request help write it out to cout.
  if (arguments.read("-h") || arguments.read("--help"))
  {
    osg::setNotifyLevel(osg::NOTICE);
    usage( arguments.getApplicationName().c_str(), 0 );
    //arguments.getApplicationUsage()->write(std::cout);
    return 1;
  }

  bool isDeviceDeployment = false;
  // if user request output for device deployment
  if (arguments.read("--device"))
  {
    isDeviceDeployment = true;
  }

  bool isPackage = false;
  if (arguments.read("--package"))
  {
    isPackage = true;
  }

//  bool isAddOverlay = false;
//  
//  // if user request output for device deployment
//  if (arguments.read("--addOverlay"))
//  {
//    isAddOverlay = true;
//  }

  
  if (arguments.read("--help-env"))
  {
    arguments.getApplicationUsage()->write(std::cout, osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE);
    return 1;
  }

  if (arguments.read("--plugins"))
  {
    osgDB::FileNameList plugins = osgDB::listAllAvailablePlugins();
    for(osgDB::FileNameList::iterator itr = plugins.begin();
        itr != plugins.end();
        ++itr)
    {
      std::cout<<"Plugin "<<*itr<<std::endl;
    }
    return 0;
  }

  std::string plugin;
  if (arguments.read("--plugin", plugin))
  {
    osgDB::outputPluginDetails(std::cout, plugin);
    return 0;
  }

  std::string ext;
  if (arguments.read("--format", ext))
  {
    plugin = osgDB::Registry::instance()->createLibraryNameForExtension(ext);
    osgDB::outputPluginDetails(std::cout, plugin);
    return 0;
  }

  if (arguments.read("--formats"))
  {
    osgDB::FileNameList plugins = osgDB::listAllAvailablePlugins();
    for(osgDB::FileNameList::iterator itr = plugins.begin();
        itr != plugins.end();
        ++itr)
    {
      osgDB::outputPluginDetails(std::cout,*itr);
    }
    return 0;
  }



  if (arguments.argc()<=1)
  {
    arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
    return 1;
  }

  FileNameList fileNames;
  OrientationConverter oc;
  bool do_convert = false;

  if (arguments.read("--use-world-frame"))
  {
    oc.useWorldFrame(true);
  }

  // Load additional parameters
  std::string paramsFileName;
  osg::ref_ptr<osg::Group> params;
  if (arguments.read("--param", paramsFileName))
  {
    readAdditionalParameters(paramsFileName, params);
  }

  std::string str;
  while (arguments.read("-O",str))
  {
    osgDB::ReaderWriter::Options* options = new osgDB::ReaderWriter::Options;
    options->setOptionString(str);
    osgDB::Registry::instance()->setOptions(options);
  }

  while (arguments.read("-e",ext))
  {
    std::string libName = osgDB::Registry::instance()->createLibraryNameForExtension(ext);
    osgDB::Registry::instance()->loadLibrary(libName);
  }

  std::string libName;
  while (arguments.read("-l",libName))
  {
    osgDB::Registry::instance()->loadLibrary(libName);
  }

  while (arguments.read("-o",str))
  {
    osg::Vec3 from, to;
    if( sscanf( str.c_str(), "%f,%f,%f-%f,%f,%f",
            &from[0], &from[1], &from[2],
            &to[0], &to[1], &to[2]  )
        != 6 )
    {
      float degrees;
      osg::Vec3 axis;
      // Try deg-axis format
      if( sscanf( str.c_str(), "%f-%f,%f,%f",
              &degrees, &axis[0], &axis[1], &axis[2]  ) != 4 )
      {
        usage( argv[0], "Orientation argument format incorrect." );
        return 1;
      }
      else
      {
        oc.setRotation( degrees, axis );
        do_convert = true;
      }
    }
    else
    {
      oc.setRotation( from, to );
      do_convert = true;
    }
  }

  while (arguments.read("-s",str))
  {
    osg::Vec3 scale(0,0,0);
    if( sscanf( str.c_str(), "%f,%f,%f",
            &scale[0], &scale[1], &scale[2] ) != 3 )
    {
      usage( argv[0], "Scale argument format incorrect." );
      return 1;
    }
    oc.setScale( scale );
    do_convert = true;
  }

  float simplifyPercent = 1.0;
  bool do_simplify = false;
  while ( arguments.read( "--simplify",str ) )
  {
    float nsimp = 1.0;
    if( sscanf( str.c_str(), "%f",
            &nsimp ) != 1 )
    {
      usage( argv[0], "Scale argument format incorrect." );
      return 1;
    }
    std::cout << str << " " << nsimp << std::endl;
    simplifyPercent = nsimp;
    osg::notify( osg::INFO ) << "Simplifying with percentage: " << simplifyPercent << std::endl;
    do_simplify = true;
  }

  while (arguments.read("-t",str))
  {
    osg::Vec3 trans(0,0,0);
    if( sscanf( str.c_str(), "%f,%f,%f",
            &trans[0], &trans[1], &trans[2] ) != 3 )
    {
      usage( argv[0], "Translation argument format incorrect." );
      return 1;
    }
    oc.setTranslation( trans );
    do_convert = true;
  }


  FixTransparencyVisitor::FixTransparencyMode fixTransparencyMode = FixTransparencyVisitor::NO_TRANSPARANCY_FIXING;
  std::string fixString;
  while(arguments.read("--fix-transparency")) fixTransparencyMode = FixTransparencyVisitor::MAKE_OPAQUE_TEXTURE_STATESET_OPAQUE;
  while(arguments.read("--fix-transparency-mode",fixString))
  {
    if (fixString=="MAKE_OPAQUE_TEXTURE_STATESET_OPAQUE") fixTransparencyMode = FixTransparencyVisitor::MAKE_OPAQUE_TEXTURE_STATESET_OPAQUE;
    if (fixString=="MAKE_ALL_STATESET_OPAQUE") fixTransparencyMode = FixTransparencyVisitor::MAKE_ALL_STATESET_OPAQUE;
  };

  bool pruneStateSet = false;
  while(arguments.read("--prune-StateSet")) pruneStateSet = true;

  osg::Texture::InternalFormatMode internalFormatMode = osg::Texture::USE_IMAGE_DATA_FORMAT;
  while(arguments.read("--compressed") || arguments.read("--compressed-arb")) { internalFormatMode = osg::Texture::USE_ARB_COMPRESSION; }

  while(arguments.read("--compressed-dxt1")) { internalFormatMode = osg::Texture::USE_S3TC_DXT1_COMPRESSION; }
  while(arguments.read("--compressed-dxt3")) { internalFormatMode = osg::Texture::USE_S3TC_DXT3_COMPRESSION; }
  while(arguments.read("--compressed-dxt5")) { internalFormatMode = osg::Texture::USE_S3TC_DXT5_COMPRESSION; }

  bool smooth = false;
  while(arguments.read("--smooth")) { smooth = true; }

  bool addMissingColours = false;
  while(arguments.read("--addMissingColours") || arguments.read("--addMissingColors")) { addMissingColours = true; }

  bool do_overallNormal = false;
  while(arguments.read("--overallNormal") || arguments.read("--overallNormal")) { do_overallNormal = true; }

  bool enableObjectCache = false;
  while(arguments.read("--enable-object-cache")) { enableObjectCache = true; }

  int width = 720;
  int height = 1280;
  if (arguments.read("--width", width))
  {
  }
  if (arguments.read("--height", height))
  {
  }

  
  // any option left unread are converted into errors to write out later.
  arguments.reportRemainingOptionsAsUnrecognized();

  // report any errors if they have occurred when parsing the program arguments.
  if (arguments.errors())
  {
    arguments.writeErrorMessages(std::cout);
    return 1;
  }

  for(int pos=1;pos<arguments.argc();++pos)
  {
    if (!arguments.isOption(pos))
    {
      fileNames.push_back(arguments[pos]);
    }
  }

  if (enableObjectCache)
  {
    if (osgDB::Registry::instance()->getOptions()==0) osgDB::Registry::instance()->setOptions(new osgDB::Options());
    osgDB::Registry::instance()->getOptions()->setObjectCacheHint(osgDB::Options::CACHE_ALL);
  }

  std::string fileNameOut("converted.osg");
  if (fileNames.size()>1)
  {
    fileNameOut = fileNames.back();
    fileNames.pop_back();
  }

  osg::Timer_t startTick = osg::Timer::instance()->tick();

  osg::notify(osg::NOTICE)<<"===================== STEP: loading original file ====================="<<std::endl;

  osg::ref_ptr<osg::Node> root = osgDB::readRefNodeFiles(fileNames);

  if (root.valid())
  {
    osg::Timer_t endTick = osg::Timer::instance()->tick();
    osg::notify(osg::INFO)<<"Time to load files "<<osg::Timer::instance()->delta_m(startTick, endTick)<<" ms"<<std::endl;
  }


  if (pruneStateSet)
  {
    PruneStateSetVisitor pssv;
    root->accept(pssv);
  }

  if (fixTransparencyMode != FixTransparencyVisitor::NO_TRANSPARANCY_FIXING)
  {
    FixTransparencyVisitor atv(fixTransparencyMode);
    root->accept(atv);
  }

  if ( root.valid() )
  {
    if (smooth)
    {
      osgUtil::SmoothingVisitor sv;
      root->accept(sv);
    }

    if (addMissingColours)
    {
      AddMissingColoursToGeometryVisitor av;
      root->accept(av);
    }

    // optimize the scene graph, remove redundant nodes and state etc.
    osgUtil::Optimizer optimizer;
    //optimizer.optimize(root.get());

    if( do_convert )
      root = oc.convert( root.get() );

    if (internalFormatMode != osg::Texture::USE_IMAGE_DATA_FORMAT)
    {
      std::string ext = osgDB::getFileExtension(fileNameOut);
      CompressTexturesVisitor ctv(internalFormatMode);
      root->accept(ctv);
      ctv.compress();

      osgDB::ReaderWriter::Options *options = osgDB::Registry::instance()->getOptions();
      if (ext!="ive" || (options && options->getOptionString().find("noTexturesInIVEFile")!=std::string::npos))
      {
        ctv.write(osgDB::getFilePath(fileNameOut));
      }
    }

    // scrub normals
    if ( do_overallNormal )
    {
      DefaultNormalsGeometryVisitor dngv;
      root->accept( dngv );
    }

    // apply any user-specified simplification
    if ( do_simplify )
    {
      osgUtil::Simplifier simple;
      simple.setSmoothing( smooth );
      osg::notify( osg::ALWAYS ) << " smoothing: " << smooth << std::endl;
      simple.setSampleRatio( simplifyPercent );
      root->accept( simple );
    }

    osg::notify(osg::NOTICE)<<"===================== STEP: replace lighting ====================="<<std::endl;
    ReplaceLightVisitor replaceLightVisitor;
    root->accept(replaceLightVisitor);
    std::vector<ImageMetrics::Light>& lights = replaceLightVisitor.getLights();
    for(size_t i = 0; i < lights.size(); i++)
    {
      const osg::Vec4& pos = lights[i].GetPos();
      osg::notify(osg::NOTICE)<<"light "<<i<<" is "<<(pos[3] == 0.0 ? "directional light" : "point light")<<std::endl;

      osg::notify(osg::NOTICE)<<"   pos: ("<<pos[0]<<", "<<pos[1]<<", "<<pos[2]<<", "<<pos[3]<<")"<< std::endl;
      const osg::Vec3& a = lights[i].GetAmbient();
      osg::notify(osg::NOTICE)<<"   ambient: ("<<a[0]<<", "<<a[1]<<", "<<a[2]<<")"<< std::endl;
      const osg::Vec3& d = lights[i].GetDiffuse();
      osg::notify(osg::NOTICE)<<"   diffuse: ("<<d[0]<<", "<<d[1]<<", "<<d[2]<<")"<< std::endl;
      const osg::Vec3& s = lights[i].GetSpecular();
      osg::notify(osg::NOTICE)<<"   specular: ("<<s[0]<<", "<<s[1]<<", "<<s[2]<<")"<< std::endl;
      float p = lights[i].GetPower();
      osg::notify(osg::NOTICE)<<"   power: "<<p<< std::endl;
    }
  
    // Remove light source
    std::vector<osg::Group*>& lightParents = replaceLightVisitor.getParentNodes();
    for(size_t i = 0; i < lightParents.size(); i++)
    {
      lightParents[i]->removeChildren(0, lightParents[i]->getNumChildren());
    }
  
    
    osg::notify(osg::NOTICE)<<"===================== STEP: create shaders ====================="<<std::endl;

    // Add shaders into shader manager
//    ImageMetrics::ShaderManager::getInstance().AddShader(new ImageMetrics::Shader_Phong());
//    ImageMetrics::ShaderManager::getInstance().AddShader(new ImageMetrics::Shader_Reflective());
//    ImageMetrics::ShaderManager::getInstance().AddShader(new ImageMetrics::Shader_Phong_NormalMap());
//    ImageMetrics::ShaderManager::getInstance().AddShader(new ImageMetrics::Shader_Mojo_RGBA());
//    ImageMetrics::ShaderManager::getInstance().AddShader(new ImageMetrics::Shader_Blinn());

    ImageMetrics::ShaderManager::getInstance().InitShaders(lights);
    ImageMetrics::ShaderManager::getInstance().SetIsBGRColorChannels(isDeviceDeployment);
    
    // Pass the absolute path of input file to replace material
    std::string curDir(getcwd(NULL,0));
    std::string absoluteOutPath;
    if(osgDB::isAbsolutePath(fileNameOut))
    {
      absoluteOutPath = fileNameOut;
    }
    else
    {
      absoluteOutPath = osgDB::concatPaths(curDir, fileNameOut);
    }
    
    ReplaceMaterialVisitor replaceStandardMaterialVisitor(params, osgDB::getFilePath(absoluteOutPath));
    root->accept(replaceStandardMaterialVisitor);

    // Replace with customize GLSL shader
    replaceShaderMaterial(root);

    //prepareNormalMap(root);
    
    // Set matrix uniform to be dynamic
    setupMatrixUniform(root);
    
    osg::notify(osg::NOTICE)<<"===================== STEP: add overlays   ====================="<<std::endl;

    // add overlay layer
    root = addOverlay(root, params, isDeviceDeployment, width, height);
    
    osg::notify(osg::NOTICE)<<"===================== STEP: optimzie       ====================="<<std::endl;

    // Optimize scene graph
    optimizer.optimize(root.get());
    
    osg::notify(osg::NOTICE)<<"===================== STEP: add triggers   ====================="<<std::endl;
    
    // Add trigger nodes
    addTriggerNodes(root, params);
    
    osg::notify(osg::NOTICE)<<"===================== STEP: postprocess    ====================="<<std::endl;
    // post process for device deployment
    root = postProcess(root, isDeviceDeployment, width, height);
    
    //osgDB::ReaderWriter::WriteResult result = osgDB::Registry::instance()->writeNode(*root,fileNameOut,new osgDB::Options("WriteImageHint=UseExternal Compressor=zlib"));
    osgDB::ReaderWriter::WriteResult result = osgDB::Registry::instance()->writeNode(*root,fileNameOut,new osgDB::Options("WriteImageHint=UseExternal"));

    if (result.success())
    {
      osg::notify(osg::NOTICE)<<"Data written to '"<<fileNameOut<<"'."<< std::endl;
    }
    else if  (result.message().empty())
    {
      osg::notify(osg::NOTICE)<<"Warning: file write to '"<<fileNameOut<<"' not supported."<< std::endl;
    }
    else
    {
      osg::notify(osg::NOTICE)<<result.message()<< std::endl;
    }
    
    // Create package
    createPackage(root, absoluteOutPath, isPackage);
    
    osg::notify(osg::NOTICE)<<"=====================         DONE         ====================="<<std::endl;

    return 0;
  }
  else
  {
    osg::notify(osg::NOTICE)<<"Error no data loaded."<< std::endl;
    return 1;
  }

  return 0;
}

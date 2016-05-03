import maya.cmds;
import os;

IMTOOLS_PLUGIN_FOLDER = os.environ.get('IMAGEMETRICS_TOOLS_PLUGIN_PATH', None)
custom_shader_path = os.path.join(IMTOOLS_PLUGIN_FOLDER, 'Scripts', 'shaders');
###################################################

# custom shader list
MOJORGBA_SHADER = 'mojo_rgba';
TEXTURERGBA_SHADER = 'texture_rgba';
OCCLUSION_SHADER = 'occlusion_rgba';
    
    
class OSGAddShaders:

        
    def createCustomGLSLShaders(self):
        curMaterials = maya.cmds.ls(materials=True);

        if MOJORGBA_SHADER in curMaterials:
            print('%s is created'%MOJORGBA_SHADER);
        else:    
            mojoRGBAShader = maya.cmds.shadingNode('GLSLShader', asShader=True, name=MOJORGBA_SHADER);
            mojoRGBASG = maya.cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name='%sSG'%MOJORGBA_SHADER);
            #cmds.connectAttr((mojoRGBAShader+'.outColor'),(mojoRGBASG+'.surfaceShader'),f=True);
            shaderSource = os.path.join(custom_shader_path, MOJORGBA_SHADER+'.ogsfx');
            maya.cmds.setAttr(mojoRGBAShader+'.shader', shaderSource, type='string');


        if TEXTURERGBA_SHADER in curMaterials:
            print('%s is created'%TEXTURERGBA_SHADER);
        else:  
            textureRGBAShader = maya.cmds.shadingNode('GLSLShader', asShader=True, name=TEXTURERGBA_SHADER);
            textureRGBASG = maya.cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name='%sSG'%TEXTURERGBA_SHADER);
            #cmds.connectAttr((textureRGBAShader+'.outColor'),(textureRGBASG+'.surfaceShader'),f=True);
            shaderSource = os.path.join(custom_shader_path, TEXTURERGBA_SHADER+'.ogsfx');
            maya.cmds.setAttr(textureRGBAShader+'.shader', shaderSource, type='string');
         
         
        if OCCLUSION_SHADER in curMaterials:
            print('%s is created'%OCCLUSION_SHADER);
        else:  
            occlusionRGBAShader = maya.cmds.shadingNode('GLSLShader', asShader=True, name=OCCLUSION_SHADER);
            occlusionRGBASG = maya.cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name='%sSG'%OCCLUSION_SHADER);
            #maya.cmds.connectAttr((occlusionRGBAShader+'.outColor'),(occlusionRGBASG+'.surfaceShader'),f=True);
            shaderSource = os.path.join(custom_shader_path, OCCLUSION_SHADER+'.ogsfx');
            maya.cmds.setAttr(occlusionRGBAShader+'.shader', shaderSource, type='string');
               
    def __init__(self):
        print('--------  load glsl plugin --------');
        maya.cmds.loadPlugin("glslShader");

        print('--------  Add custom GLSL shaders --------');
        self.createCustomGLSLShaders();

        print('--------  Done --------');


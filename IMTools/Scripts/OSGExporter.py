import maya.cmds;
import os;
import subprocess;
import zipfile;
import shutil;
import OSGAnimation;
import OSGAddShaders;

### OpenSceneGraph library and bin path setting ###
#default script folder:
IMTOOLS_PLUGIN_FOLDER = os.environ.get('IMAGEMETRICS_TOOLS_PLUGIN_PATH', None);

osg_library_path = os.path.join(IMTOOLS_PLUGIN_FOLDER , 'bin');
osg_execute_path = os.path.join(IMTOOLS_PLUGIN_FOLDER , 'bin');
###################################################

# define all custom shaders
CUSTOM_SHADERS = ['texture_rgba', 'mojo_rgba', 'occlusion_rgba'];
            
#define triggers dictionary and action dictionary
triggers_dict = dict([(0, 'none'), (1, 'mouthopen:start'), (2, 'mouthopen:end'), (3, 'eyesclosed:start'), (4, 'eyesclosed:end'), (5, 'browsraised:start'), (6, 'browsraised:end'), (7, 'headup:start'), (8, 'headup:end'), (9, 'headdown:start'), (10, 'headdown:end'), (11, 'headleft:start'), (12, 'headleft:end'), (13, 'headright:start'), (14, 'headright:end')]);
action_dict = dict([(0, 'run'), (1, 'pause'), (2, 'hide'), (3, 'show')]);
            
class OSGExporter:
    

    def get_image_sequence_name_pattern(self, file_name, scene_file_folder):
        relative_path = os.path.relpath(file_name, scene_file_folder)
        if '.<f>.' in relative_path:
            return relative_path;
        p1 = relative_path.rfind('.');
        p2 = relative_path.rfind('.', 0, p1-1);
        return relative_path[:p2+1]+'<f>'+relative_path[p1:];
    
    # Test whether a given file node is a valid texture sequence node
    def is_valid_texture_sequence(self, file_node, mat_list, scene_file_folder):
   
        # Check whether it is an image sequence
        if not maya.cmds.getAttr('%s.useFrameExtension'%file_node):
            return False;
    
        # Get the shader connected to the image    
        shaders = maya.cmds.listConnections('%s.outColor'%file_node, destination=True);
        if not shaders:
            shaders = maya.cmds.listConnections('%s.outAlpha'%file_node, destination=True);
            if not shaders:
                return False;
    
        for shader in shaders:
            mat_node = shader;
            is_bump_texture = False;
        
            # If it is bump mapping node
            if maya.cmds.nodeType(shader) == 'bump2d':
                bump_mat = maya.cmds.listConnections('%s.outNormal'%shader);
                if bump_mat:
                    mat_node = bump_mat[0];
                    is_bump_texture = True;
                
            if not mat_node:
                continue;
        

        
            try:
                colorConnections = maya.cmds.ls(maya.cmds.listConnections('%s.color'%mat_node, type = "file"))
                if colorConnections:
                    for con in colorConnections:
                        matColor = maya.cmds.getAttr(con + ".fileTextureName")
                        fileName = self.get_image_sequence_name_pattern(matColor, scene_file_folder);
                        print('.color:'+fileName);
            
                colorConnections = maya.cmds.ls(maya.cmds.listConnections('%s.transparency'%mat_node, type = "file"))
                if colorConnections:
                    for con in colorConnections:
                        matColor = maya.cmds.getAttr(con + ".fileTextureName")
                        fileName = self.get_image_sequence_name_pattern(matColor, scene_file_folder);
                        print('.transparency:'+fileName);
            
                colorConnections = maya.cmds.ls(maya.cmds.listConnections('%s.specularColor'%mat_node, type = "file"))
                if colorConnections:
                    for con in colorConnections:
                        matColor = maya.cmds.getAttr(con + ".fileTextureName")
                        fileName = self.get_image_sequence_name_pattern(matColor, scene_file_folder);
                        print('.specularColor:'+fileName);
                            
                bumpNode = maya.cmds.listConnections('%s.normalCamera'%mat_node);
                if bumpNode:
                    bumpNode = bumpNode[0];
                    bumpConnections = maya.cmds.listConnections('%s.bumpValue'%bumpNode, type='file')
                    if bumpConnections:
                        for con in bumpConnections:
                            bumpText = maya.cmds.getAttr(con + ".fileTextureName");
                            fileName = self.get_image_sequence_name_pattern(bumpText, scene_file_folder);
                            print('.bump:'+fileName);
            except:
                print('catch exception');
        
            try:
                groups = maya.cmds.listConnections('%s.outColor'%mat_node);
                if not groups: return False;
                for group in groups:
                    meshes = maya.cmds.listConnections(group, type='mesh');
                    if meshes:
                        # Add this material which including image sequence into mat_list            
                        mat_list.add(mat_node);
                        return True;
            except:
                return False;
                  
        return False;
 
 
    # Write image sequence files
    def write_texture_sequence(self, mat_node, f, unique_id_count, start_frame, end_frame, scene_file_folder):
        try:
            frame_num = end_frame - start_frame + 1;
            print('write_texture_sequence:'+mat_node);
            # texture_rgba shader
            if 'texture_rgba' in mat_node:
                colorConnections = maya.cmds.ls(maya.cmds.listConnections('%s.gTexture'%mat_node, type = "file"))
            else:
                colorConnections = maya.cmds.ls(maya.cmds.listConnections('%s.color'%mat_node, type = "file"))
            if colorConnections:
                for con in colorConnections:
                    if maya.cmds.getAttr('%s.useFrameExtension'%con):
                        matColor = maya.cmds.getAttr(con + ".fileTextureName")
                        fileName = self.get_image_sequence_name_pattern(matColor, scene_file_folder);
                        f.write('        DiffuseImageSequence %d { \n'%frame_num);
                        for i in range(0, frame_num):
                            one_file = fileName.replace('<f>', '{0:05d}'.format(i));
                            f.write('          "%s" \n'%one_file);
                        f.write('        } \n');
                        
            bumpNode = maya.cmds.listConnections('%s.normalCamera'%mat_node);
            if bumpNode:
                bumpNode = bumpNode[0];
                bumpConnections = maya.cmds.listConnections('%s.bumpValue'%bumpNode, type='file')
                if bumpConnections:
                    for con in bumpConnections:
                        if maya.cmds.getAttr('%s.useFrameExtension'%con):
                            bumpText = maya.cmds.getAttr(con + ".fileTextureName");
                            fileName = self.get_image_sequence_name_pattern(bumpText, scene_file_folder);
                            f.write('        NormalMapImageSequence %d { \n'%frame_num);
                            for i in range(0, frame_num):
                                one_file = fileName.replace('<f>', '{0:05d}'.format(i));
                                f.write('          "%s" \n'%one_file);
                            f.write('        } \n');
                    
        except:
            print('catch exception in writing image sequence');
 
    # Write triggers
    def write_triggers(self, f, unique_id_count):
        try:
            print('enter write_triggers');
            trigger_node_list = set();

            nodes = maya.cmds.ls(geometry=True, lights=True, transforms=True);
            global_3danimation_trigger_list = set();
            triggers_count = 0;


            if nodes:
                for one_node in nodes:
                    attrExist = maya.cmds.attributeQuery('triggers', node=one_node, exists=True);
                    if attrExist:
                        triggers = maya.cmds.listAttr(one_node+'.triggers', multi=True, st=['triggers']);
                        #print(triggers);
                        if triggers:
                            for one_trigger in triggers:
                                trigger_type = maya.cmds.getAttr(one_node+'.'+one_trigger+'.oneTrigger');
                                # Find one valid trigger (0 -- means no trigger)
                                if trigger_type != 0:
                                    triggers_count += 1;
                                    apply_to_global_3danimation = maya.cmds.getAttr(one_node+'.'+one_trigger+'.applyToGlobal3DAnimation');
                                    if apply_to_global_3danimation:
                                        global_3danimation_trigger_list.add(one_node);
                                    else:
                                        trigger_node_list.add(one_node);                    
                    
            print('add general triggers');  
            print(trigger_node_list);          
            f.write('    Children %d { \n'%triggers_count);            
            # Find all non-global 3d animation triggers attributes
            for one_trigger_node in trigger_node_list:
                triggers = maya.cmds.listAttr(one_trigger_node+'.triggers', multi=True, st=['triggers']);
                if triggers:
                    for one_trigger in triggers:
                        #print(one_trigger);
                        trigger_type = maya.cmds.getAttr(one_trigger_node+'.'+one_trigger+'.oneTrigger');
                        #print(trigger_type);
                        if trigger_type != 0:
                            apply_to_global_3danimation = maya.cmds.getAttr(one_trigger_node+'.'+one_trigger+'.applyToGlobal3DAnimation');
                            if not apply_to_global_3danimation:
                                f.write('      osg::TriggerParameterNode { \n');    
                                f.write('        UniqueID %d \n'%unique_id_count);
                                unique_id_count += 1;
                                f.write('        TriggerType "%s" \n'%triggers_dict[trigger_type]);
                                f.write('        NodeName "%s" \n'%one_trigger_node);
                                trigger_action = maya.cmds.getAttr(one_trigger_node+'.'+one_trigger+'.action');
                                f.write('        TriggerAction "%s" \n'%action_dict[trigger_action]);
                                timeout_secs = maya.cmds.getAttr(one_trigger_node+'.'+one_trigger+'.timeoutSecs');
                                f.write('        TriggerTimeoutSecs %f \n'%timeout_secs);
                                if maya.cmds.getAttr(one_trigger_node+'.'+one_trigger+'.autoTimeout'):
                                    auto_timeout = 'TRUE';
                                else:
                                    auto_timeout = 'FALSE';
                                f.write('        TriggerAutoTimeout %s \n'%auto_timeout);
                                if maya.cmds.getAttr(one_trigger_node+'.'+one_trigger+'.triggeredDefault'):
                                    triggered_default = 'TRUE';
                                else:
                                    triggered_default = 'FALSE';
                                f.write('        TriggeredDefault %s \n'%triggered_default);

                                # For general triggers control type is switch
                                f.write('        ControlType "switch" \n');
                                f.write('      } \n');

        
            print('add 3d animation triggers');    
            print(global_3danimation_trigger_list);          
            animationData = OSGAnimation.OSGAnimation.importData();  
            animationNameList = {};
            if animationData:
                animationNameList = animationData.keys();
                  
            # Find all global 3d animation triggers attributes
            for one_trigger_node in global_3danimation_trigger_list:
                triggers = maya.cmds.listAttr(one_trigger_node+'.triggers', multi=True, st=['triggers']);
                if triggers:
                    for one_trigger in triggers:
                        #print(one_trigger);
                        trigger_type = maya.cmds.getAttr(one_trigger_node+'.'+one_trigger+'.oneTrigger');
                        #print(trigger_type);
                        if trigger_type != 0:
                            apply_to_global_3danimation = maya.cmds.getAttr(one_trigger_node+'.'+one_trigger+'.applyToGlobal3DAnimation');
                            if apply_to_global_3danimation:
                                f.write('      osg::TriggerParameterNode { \n');    
                                f.write('        UniqueID %d \n'%unique_id_count);
                                unique_id_count += 1;
                                f.write('        TriggerType "%s" \n'%triggers_dict[trigger_type]);
                                # Global 3d animation using default animation clip name 'Take 001'
                                trigger_anim_name = animationNameList[maya.cmds.getAttr(one_trigger_node+'.'+one_trigger+'.animationName')];
                                f.write('        NodeName "%s" \n'%trigger_anim_name);
                                trigger_action = maya.cmds.getAttr(one_trigger_node+'.'+one_trigger+'.action');
                                f.write('        TriggerAction "%s" \n'%action_dict[trigger_action]);
                                timeout_secs = maya.cmds.getAttr(one_trigger_node+'.'+one_trigger+'.timeoutSecs');
                                f.write('        TriggerTimeoutSecs %f \n'%timeout_secs);
                                if maya.cmds.getAttr(one_trigger_node+'.'+one_trigger+'.autoTimeout'):
                                    auto_timeout = 'TRUE';
                                else:
                                    auto_timeout = 'FALSE';
                                f.write('        TriggerAutoTimeout %s \n'%auto_timeout);
                                if maya.cmds.getAttr(one_trigger_node+'.'+one_trigger+'.triggeredDefault'):
                                    triggered_default = 'TRUE';
                                else:
                                    triggered_default = 'FALSE';
                                f.write('        TriggeredDefault %s \n'%triggered_default);
                                # For 3d animation, control type is animationmanager
                                f.write('        ControlType "animationmanager" \n');
                                f.write('      } \n');             

            f.write('    } \n');
            return unique_id_count;
        except:
            print('catch exception in writing triggers ');
            return unique_id_count;
         


    # Write animations
    def write_animations(self, f, unique_id_count):
        try:
            print('enter write_animations');
            
            scene_fps = 24.0;
            scene_start_frame = maya.cmds.playbackOptions(query = True, minTime = True);
            scene_end_frame = maya.cmds.playbackOptions(query = True, maxTime = True);
                        
            animationData = OSGAnimation.OSGAnimation.importData();  
            if animationData:
                animationNameList = animationData.keys()
    
                           
                f.write('    Children %d { \n'%len(animationNameList));            
                # Find all non-global 3d animation triggers attributes
                for oneName in animationNameList:                    
                    oneAnimation = animationData[oneName];
     
                    start_time = (float(oneAnimation['startFrame']) - scene_start_frame + 1) / scene_fps;
                    end_time = (float(oneAnimation['endFrame']) - scene_start_frame + 1) / scene_fps;
                    duration = (float(oneAnimation['endFrame']) - float(oneAnimation['startFrame']) + 1)/float(oneAnimation['fps']);
                    
                    f.write('      osg::AnimationParameterNode { \n');    
                    f.write('        UniqueID %d \n'%unique_id_count);
                    unique_id_count += 1;
                    f.write('        AnimationName "%s" \n'%oneAnimation['animationName']);
                    f.write('        StartTime %f \n'%start_time);
                    f.write('        EndTime %f \n'%end_time);
                    f.write('        Duration %f \n'%duration);
                    f.write('        OriginalFPS %f \n'%scene_fps);
                    f.write('      } \n');
             
                f.write('    } \n');
            return unique_id_count;
        except:
            print('catch exception in writing animations ');
            return unique_id_count;

    
    # Write geode property
    def write_geode_property(self, f, unique_id_count):
        try:
            print('enter write_geode_property');
            staticNodes = maya.cmds.listRelatives('static', children=True, noIntermediate=True, fullPath=False);
            print(staticNodes);
            if staticNodes:
                f.write('    Children %d { \n'%len(staticNodes));            
                for oneNode in staticNodes:                    
                    f.write('      osg::GeodePropertyParameterNode { \n');    
                    f.write('        UniqueID %d \n'%unique_id_count);
                    unique_id_count += 1;
                    f.write('        NodeName "%s" \n'%oneNode);
                    f.write('        IsStaticNode TRUE \n');
                    f.write('      } \n');
                f.write('    } \n');
            return unique_id_count;
        except:
            print('catch exception in writing geode property');
            return unique_id_count;
            
            
    def createTransparentMaterialForOcclusionMesh(self):
        try:
            OcclusionMeshShape = maya.cmds.listRelatives('OcclusionMesh', shapes=True);
    
            material = maya.cmds.listConnections(OcclusionMeshShape[0], s=True, type='shadingEngine');
            print(material);  
            # already set some material
            if material:
                maya.cmds.sets(material, rm='initialShadingGroup');
            newSG = maya.cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name='occlusion_rgbaSG1');
            maya.cmds.connectAttr(('occlusion_rgba.outColor'),(newSG+'.surfaceShader'),f=True);
            maya.cmds.sets('OcclusionMesh', e=True, forceElement=newSG);
        except:
            print("createTransparentMaterialForOcclusionMesh error");
        
        print("set occlusion scale");
        try:
            maya.cmds.setAttr("occlusion.scaleX", 0.99);
            maya.cmds.setAttr("occlusion.scaleY", 0.99);
            maya.cmds.setAttr("occlusion.scaleZ", 0.99);    
        except:
            print('set occlusionMesh scale error');
        return;
            
    # Get shaders connect to SG node
    #    shaders = maya.cmds.listNodeTypes('shader'); 
    #    if shaders:
    #        for oneShader in shaders:
    #            obj = maya.cmds.listConnections(material[0], s=True, d=False, exactType=True, type='%s'%oneShader);
    #            print obj


    # Return the dictionary holding nodes using custom shader
    def getCustomShaders(self, custom_mat_list):
        # create dictionary 
        for one_shader in CUSTOM_SHADERS:
            custom_mat_list[one_shader] = set();

        # list all transform nodes
        transforms = maya.cmds.ls(transforms=True);
        for one_transform in transforms:
            # if transform nodes also connect to a shape node
            print(one_transform);
            shapes = maya.cmds.listRelatives(one_transform, shapes=True);
            if shapes:
                for one_shape in shapes:
                    materials = maya.cmds.listConnections(one_shape, s=True, type='shadingEngine');
                    if materials:
                        for one_material in materials:
                            for one_shader in CUSTOM_SHADERS:
                                if one_shader in one_material:
                                    custom_mat_list[one_shader].add((one_transform, one_material));



    def write_custom_shader_textures(self, key, one_node, f):
   
        # Get GLSL shader
        shader = maya.cmds.listConnections(one_node[1], s=True, d=False, exactType=True, type='GLSLShader')[0];
        f.write('        Type "%s" \n'%key);

        # Write shader name
        f.write('        Name "%s" \n'%shader);
    
        # Handle texture_rgba shader (simple texture mapping)
        if key == 'texture_rgba':
            texture_file = maya.cmds.listConnections('%s.gTexture'%shader, destination=True)[0];
            print(texture_file);
            # Check whether it is an image sequence since we will handle image sequence separately
            if maya.cmds.getAttr('%s.useFrameExtension'%texture_file):
                return ;
            else:
                fileName = maya.cmds.getAttr(texture_file + ".fileTextureName");
                print(fileName);
                f.write('        DiffuseTextureFileName "%s" \n'%fileName);
        # Handle mojo_rgba shader
        elif key == 'mojo_rgba':
            #write some weights parameters
            f.write('        MojoLightingWeight %f \n'%maya.cmds.getAttr(shader+".lightingWeight"));
            f.write('        MojoPaintReferenceIntensity %f \n'%maya.cmds.getAttr(shader+".paintReferenceIntensity"));
            f.write('        MojoImageReferenceIntensity %f \n'%maya.cmds.getAttr(shader+".imageReferenceIntensity"));

            texture_file = maya.cmds.listConnections('%s.gTexture'%shader, destination=True)[0];
            print(texture_file);
            # Check whether it is an image sequence since we will handle image sequence separately
            if maya.cmds.getAttr('%s.useFrameExtension'%texture_file):
                return ;
            else:
                fileName = maya.cmds.getAttr(texture_file + ".fileTextureName");
                print(fileName);
                f.write('        DiffuseTextureFileName "%s" \n'%fileName);

 
    def __init__(self):               
        print('\n\n==========================');
        print('--------  Start --------');
        select_nodes = maya.cmds.ls(selection=True);
        
        try:
            OSGAddShaders.OSGAddShaders();
        except:
            print("Add shaders error");
        
        # Get current scene name
        scene_full_path = maya.cmds.file(q=True, sceneName=True);
        scene_file_folder = os.path.abspath(os.path.join(scene_full_path, os.pardir));

        # We save temporary osg export related files into CURRENT_SCENE/osg_export folder
        osg_export_folder = os.path.join(scene_file_folder, 'osg_export');
        scene_file_name = os.path.splitext(os.path.basename(scene_full_path))[0];
        if not scene_file_name:
            maya.cmds.confirmDialog( title='Error', message='Scene is not saved: \nPlease save your scene first', button=['OK'], defaultButton='OK', cancelButton='OK', dismissString='OK' );
            return;
        mat_list = set();      
        texture_nodes = [
            i for i in maya.cmds.ls(type='file')
            if self.is_valid_texture_sequence(i, mat_list, osg_export_folder)
        ];

        print('--------  find texture sequences: --------');
        print(texture_nodes);
        print('--------  find material including texture sequence:  --------');
        print(mat_list);

        # Check and create transparent material to occlusion
        print('--------  create material for occlusion mesh  --------');
        self.createTransparentMaterialForOcclusionMesh();

        print('--------  find custom shaders: --------');
        custom_mat_dict = {};
        self.getCustomShaders(custom_mat_dict);
        print(custom_mat_dict);

        custom_shader_len = 0;
        for key in custom_mat_dict:
            custom_shader_len += len(custom_mat_dict[key]);
        print('number of custom shader nodes: %d'%custom_shader_len);



        osg_param_file_name = os.path.join(osg_export_folder, scene_file_name+'_param.osgt');
        print('osg extra param file: '+osg_param_file_name);

        # Delete old osg export folder and create new osg_export folder
        if os.path.exists(osg_export_folder):
            shutil.rmtree(osg_export_folder)
        os.makedirs(osg_export_folder)
    
        # Get animation playback start/end frame
        print('--------  save parameter file  --------');

        start_frame = int(maya.cmds.playbackOptions(query = True, minTime = True));
        end_frame = int(maya.cmds.playbackOptions(query = True, maxTime = True));

        unique_id_count = 1;
        try:
            f = open(osg_param_file_name, 'w+');
            f.write('#Ascii Scene \n');
            f.write('#Version 141 \n');
            f.write('#Generator OpenSceneGraph 3.5.2 \n\n');
            f.write('osg::Group {\n');
            f.write('  UniqueID %d \n'%unique_id_count);
            unique_id_count += 1;
            f.write('  Children 4 { \n');
            f.write('  osg::Group { \n');
            f.write('    UniqueID %d \n'%unique_id_count);
            unique_id_count += 1;
            f.write('    Name "material" \n');
            f.write('    Children %d { \n'%(len(mat_list)+custom_shader_len));
            # Write custom shader nodes
            for key in custom_mat_dict:
                for one_node in custom_mat_dict[key]:
                    f.write('      osg::MaterialParameterNode { \n');
                    f.write('        UniqueID %d \n'%unique_id_count);
                    unique_id_count += 1;
                    # We need to write the custom shader textures since the fbx exporter will not
                    # export textures
                    self.write_custom_shader_textures(key, one_node, f);
                    f.write('      } \n');
            
            
            # Write image sequence nodes
            for one_mat in mat_list:
                f.write('      osg::MaterialParameterNode { \n');
                f.write('        UniqueID %d \n'%unique_id_count);
                unique_id_count += 1;
                f.write('        Name "%s" \n'%one_mat);
                self.write_texture_sequence(one_mat, f, unique_id_count, start_frame, end_frame, osg_export_folder);
                f.write('      } \n');
            f.write('    } \n');
            f.write('  } \n');
    
            # Write geode property parameters
            f.write('  osg::Group { \n');
            f.write('    UniqueID %d \n'%unique_id_count);
            unique_id_count += 1;
            f.write('    Name "geodeproperty" \n');
    
            # Check and write geode property
            unique_id_count = self.write_geode_property(f, unique_id_count);
            f.write('  } \n');

            
            # Write trigger parameters
            f.write('  osg::Group { \n');
            f.write('    UniqueID %d \n'%unique_id_count);
            unique_id_count += 1;
            f.write('    Name "trigger" \n');
    
            # Check and write triggers
            unique_id_count = self.write_triggers(f, unique_id_count);
            f.write('  } \n');
            
            
            
            # Write animation parameters
            f.write('  osg::Group { \n');
            f.write('    UniqueID %d \n'%unique_id_count);
            unique_id_count += 1;
            f.write('    Name "animation" \n');
    
            # Check and write animations
            unique_id_count = self.write_animations(f, unique_id_count);
            f.write('  } \n');
            
            
            f.write('} \n'); #for root group

            f.close();
        except:
            print('write file error: make sure you have the permission to write file at %s'%osg_param_file_name);



 
        maya.cmds.select(select_nodes);
        #export fbx file
        if select_nodes: 
            print('--------  export fbx file  --------');
            maya.cmds.loadPlugin("fbxmaya");
            fbx_file_name = os.path.join(osg_export_folder, scene_file_name+'.fbx');
            fbx_file_name = fbx_file_name.replace('\\', '/')
            print('fbx file: '+fbx_file_name);
            
            maya.mel.eval('FBXExportInAscii -v true'); 
            maya.mel.eval('FBXExport -f "%s" -s' % fbx_file_name); 

            print('--------  convert to osg file  --------');
    
            # Check wheter OSG distribute is put into correct path
            if not os.path.exists(osg_execute_path):
                maya.cmds.confirmDialog( title='Error', message='Incorrect OSG path: \nPlease put IMTools into %s'%IMTOOLS_PLUGIN_FOLDER, button=['OK'], defaultButton='OK', cancelButton='OK', dismissString='OK' );
            else:
                #save current path
                try:
                    cur_path = os.getcwd();
                    print('current path:'+os.getcwd());
                except:
                    print('Error happened while exporting, please reopen this maya file');
                    return;

                #change to current scene folder
                os.chdir(osg_export_folder);
                print('scene file path:'+os.getcwd());
    
                os.environ['OSG_LIBRARY_PATH']=osg_library_path;
    
                osgt_file_name = os.path.join(osg_export_folder, scene_file_name+'.osgt'); 
    
                print('osgt file: '+osgt_file_name);
                osgconvCommand = os.path.join(osg_execute_path, 'osgconv');
                
                print('%s "%s" "%s" --param "%s"'%(osgconvCommand, fbx_file_name, osgt_file_name, osg_param_file_name));
                os.system('%s "%s" "%s" --param "%s"'%(osgconvCommand, fbx_file_name, osgt_file_name, osg_param_file_name));
        
                #display osgt file in osgviewer
                shouldLoadViewer = maya.cmds.confirmDialog( title='Confirm', message='Check OSG file in viewer?', button=['Yes','No'], defaultButton='Yes', cancelButton='No', dismissString='No' );
                if shouldLoadViewer == 'Yes':
                    print('--------  check OSG file in viewer  --------');
                    osgviewerCommand = os.path.join(osg_execute_path, 'osgviewer');
                    os.system('%s "%s"'%(osgviewerCommand, osgt_file_name));
        
                shouldCreateDeviceFile = maya.cmds.confirmDialog( title='Confirm', message='Create a deployment OSG file for device?', button=['Yes','No'], defaultButton='Yes', cancelButton='No', dismissString='No' );
                if shouldCreateDeviceFile == 'Yes':
                   print('--------  create device deployment file  --------');
                   # export device file as binary format
                   osgt_device_file_name = os.path.join(osg_export_folder, scene_file_name+'_device.osgt'); 
                   print('device osgb file: '+osgt_device_file_name);
                   print('%s "%s" "%s" --param "%s" --device --package'%(osgconvCommand, fbx_file_name, osgt_device_file_name, osg_param_file_name));
                   os.system('%s "%s" "%s" --param "%s" --device --package'%(osgconvCommand, fbx_file_name, osgt_device_file_name, osg_param_file_name));
           
                   # package osg file and images into a *_deployment.zip file
                   package_zip_file_name = os.path.join(osg_export_folder, scene_file_name+'_deployment'); 
                   package_folder = os.path.join(osg_export_folder, 'osg_package');
                   shutil.make_archive(package_zip_file_name, 'zip', package_folder);
                   # delete osg_package folder
                   shutil.rmtree(package_folder);
           
           
                #restore current path
                os.chdir(cur_path);
        else:
            maya.cmds.confirmDialog( title='Error', message='No object selected!', button=['OK'], defaultButton='OK', cancelButton='OK', dismissString='OK' );

        print('--------  done  --------');

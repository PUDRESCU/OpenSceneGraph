import maya.mel;
import maya.cmds;
import functools;
import json;
import os;

DEFAULT_ANIMATION_NAME = 'Take 001';
scene_full_path = maya.cmds.file(q=True, sceneName=True);
scene_file_folder = os.path.abspath(os.path.join(scene_full_path, os.pardir));
scene_file_name = os.path.splitext(os.path.basename(scene_full_path))[0];
animation_def_file_path = os.path.join(scene_file_folder, scene_file_name+'_animationdef.json');
                   
class OSGAnimation:
    def exportData(self, filePath):
        animationsDict = {};
        
        rowColumnLayouts = maya.cmds.scrollLayout(self.scrollLayout, query=True, ca=True);
        
        if rowColumnLayouts:
            for oneRow in rowColumnLayouts:
                oneAnimationDict = {};
                
                ui_elements = maya.cmds.rowColumnLayout(oneRow, query=True, ca=True);
                animation_name = maya.cmds.textField(ui_elements[1], text=True, query=True);
                animation_start_frame = maya.cmds.intField(ui_elements[3], value=True, query=True);
                animation_end_frame = maya.cmds.intField(ui_elements[5], value=True, query=True);
                animation_fps = maya.cmds.intField(ui_elements[7], value=True, query=True);

                print('name:%s, start_frame:%d, end_frame:%d, fps:%d'%(animation_name, animation_start_frame, animation_end_frame, animation_fps));
                oneAnimationDict['animationName']=animation_name;
                oneAnimationDict['startFrame']=str(animation_start_frame);
                oneAnimationDict['endFrame']=str(animation_end_frame);
                oneAnimationDict['fps']=str(animation_fps);
                animationsDict[animation_name] = oneAnimationDict;
 
        f=open(filePath, 'w');
        f.write(json.dumps(animationsDict));
        f.close();                
    
    @staticmethod                    
    def importData(filePath):
        f=open(filePath, 'r')
        animationData= json.loads(f.readline())
        f.close()
        
        return animationData;
        
        
            
    def OnValueChanged(self, cur_layout, *args):
        ui_elements = maya.cmds.rowColumnLayout(cur_layout, query=True, ca=True);
        animation_name = maya.cmds.textField(ui_elements[1], text=True, query=True);
        animation_start_frame = maya.cmds.intField(ui_elements[3], value=True, query=True);
        animation_end_frame = maya.cmds.intField(ui_elements[5], value=True, query=True);
        animation_fps = maya.cmds.intField(ui_elements[7], value=True, query=True);

        print('name:%s, start_frame:%d, end_frame:%d, fps:%d'%(animation_name, animation_start_frame, animation_end_frame, animation_fps));

    def AddButtonClick(self, *args):
        maya.cmds.setParent(self.scrollLayout);
        scene_fps = 24;
        scene_start_frame = int(maya.cmds.playbackOptions(query = True, minTime = True));
        scene_end_frame = int(maya.cmds.playbackOptions(query = True, maxTime = True));
        
        cur_layout = maya.cmds.rowColumnLayout( numberOfColumns=2, columnAttach=(1, 'right', 0), columnWidth=[(1, 100), (2, 250)] );
        maya.cmds.text( label='Animation Name' );
        animation_name = maya.cmds.textField(text=DEFAULT_ANIMATION_NAME, changeCommand=functools.partial(self.OnValueChanged, cur_layout), enterCommand=functools.partial(self.OnValueChanged, cur_layout));
        maya.cmds.text( label='Start Frame' );
        animation_start_frame = maya.cmds.intField(value = scene_start_frame, changeCommand=functools.partial(self.OnValueChanged, cur_layout), enterCommand=functools.partial(self.OnValueChanged, cur_layout));
        maya.cmds.text( label='End Frame' );
        animation_end_frame = maya.cmds.intField(value = scene_end_frame, changeCommand=functools.partial(self.OnValueChanged, cur_layout), enterCommand=functools.partial(self.OnValueChanged, cur_layout));
        maya.cmds.text( label='FPS' );
        animation_fps = maya.cmds.intField(value=scene_fps, changeCommand=functools.partial(self.OnValueChanged, cur_layout), enterCommand=functools.partial(self.OnValueChanged, cur_layout));
        maya.cmds.text( label='' );
        deleteButton = maya.cmds.button(label="Delete", command=functools.partial(self.DeleteButtonClick, cur_layout));

        maya.cmds.separator( height=10, style='singleDash');
   
        #cmds.optionVar( iv=('defaultTriangles', 4), sv=('defaultFileName', 'buffalo.maya') )    
        
    def DeleteButtonClick(self, cur_layout, *args):
        maya.cmds.deleteUI(cur_layout);
             
    def deleteUI(self, *args):
        try:
            if maya.cmds.window(self.winName, exists=True):
                maya.cmds.deleteUI(self.winName);
            if maya.cmds.windowPref(self.winName, exists=True):
                maya.cmds.windowPref(self.winName, remove=True);
        except:
            print();
            
    def SaveButtonClick(self, filePath, *args):
        self.exportData(filePath);
        
    def LoadButtonClick(self, filePath, *args):
        animationData = OSGAnimation.importData(filePath);
        # delete old animation data
        rowColumnLayouts = maya.cmds.scrollLayout(self.scrollLayout, query=True, ca=True);
        if rowColumnLayouts:
            for oneRow in rowColumnLayouts:
                maya.cmds.deleteUI(oneRow);
                
        #list of keys(objects)
        animationNameList = animationData.keys()
        for oneName in animationNameList:
            oneAnimation = animationData[oneName];
            
            maya.cmds.setParent(self.scrollLayout);
 
            cur_layout = maya.cmds.rowColumnLayout( numberOfColumns=2, columnAttach=(1, 'right', 0), columnWidth=[(1, 100), (2, 250)] );
            maya.cmds.text( label='Animation Name' );
            animation_name = maya.cmds.textField(text=oneAnimation['animationName'], changeCommand=functools.partial(self.OnValueChanged, cur_layout), enterCommand=functools.partial(self.OnValueChanged, cur_layout));
            maya.cmds.text( label='Start Frame' );
            animation_start_frame = maya.cmds.intField(value =int(oneAnimation['startFrame']), changeCommand=functools.partial(self.OnValueChanged, cur_layout), enterCommand=functools.partial(self.OnValueChanged, cur_layout));
            maya.cmds.text( label='End Frame' );
            animation_end_frame = maya.cmds.intField(value =int(oneAnimation['endFrame']), changeCommand=functools.partial(self.OnValueChanged, cur_layout), enterCommand=functools.partial(self.OnValueChanged, cur_layout));
            maya.cmds.text( label='FPS' );
            animation_fps = maya.cmds.intField(value =int(oneAnimation['fps']), changeCommand=functools.partial(self.OnValueChanged, cur_layout), enterCommand=functools.partial(self.OnValueChanged, cur_layout));
            maya.cmds.text( label='' );
            deleteButton = maya.cmds.button(label="Delete", command=functools.partial(self.DeleteButtonClick, cur_layout));

            maya.cmds.separator( height=10, style='singleDash');
       
    def CloseButtonClick(self, filePath):
        shouldSave = maya.cmds.confirmDialog( title='Confirm', message='Save current animation definition data?', button=['Yes','No'], defaultButton='Yes', cancelButton='No', dismissString='No' );
        if shouldSave == 'Yes':
            self.SaveButtonClick(filePath);
            
    def createUI(self):
        
        ## Create Window
        #scene_full_path = maya.cmds.file(q=True, sceneName=True);
        #scene_file_folder = os.path.abspath(os.path.join(scene_full_path, os.pardir));
        #scene_file_name = os.path.splitext(os.path.basename(scene_full_path))[0];
        
        #self.animation_def_file_path = os.path.join(scene_file_folder, scene_file_name+'_animationdef.json');
        self.animationList = {};

        createWin = maya.cmds.window(self.winName, title=self.winTitle, widthHeight=(400, 600));
        
        mainLayout = maya.cmds.formLayout();

        self.scrollLayout = maya.cmds.scrollLayout(childResizable = True);
        
        maya.cmds.setParent("..");
        
        # create buttons layout 
        buttonsLayout = maya.cmds.rowColumnLayout( numberOfColumns=3, columnWidth=[(1, 120), (2, 120), (3, 120)], columnSpacing=[(1, 0), (2, 20), (3, 20)]);

        addButton = maya.cmds.button(label="Add", command=self.AddButtonClick);
        saveButton = maya.cmds.button(label="Save", command=functools.partial(self.SaveButtonClick, animation_def_file_path)); 
        loadButton = maya.cmds.button(label="Load", command=functools.partial(self.LoadButtonClick, animation_def_file_path));
        maya.cmds.setParent("..");
        
        maya.cmds.setParent("..");

        #maya.cmds.scriptJob(uid=[self.winName, functools.partial(self.CloseButtonClick, self.animation_def_file_path)]);
        maya.cmds.formLayout(mainLayout, edit=True, attachForm=[(self.scrollLayout, "top", 0), (self.scrollLayout, "right", 0), (self.scrollLayout, "left", 0), (buttonsLayout, "bottom", 0), (buttonsLayout, "left", 0), (buttonsLayout, "right", 0)], attachControl=[(self.scrollLayout, "bottom", 5, buttonsLayout)]);
        maya.cmds.showWindow(self.winName);
                 
            
    def __init__(self):
        self.winTitle = "Animation Define";
        self.winName = "IMAnimationWin";
        
        self.deleteUI();
        self.createUI();
        

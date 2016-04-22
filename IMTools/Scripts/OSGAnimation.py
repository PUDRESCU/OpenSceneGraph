import maya.mel;
import maya.cmds;
import functools;

global_animation_list = [];
           
class OSGAnimation:
    def AddButtonClick(self, *args):
        maya.cmds.setParent(self.scrollLayout);
        scene_fps = int(maya.cmds.playbackOptions(query = True, framesPerSecond = True));
        scene_start_frame = int(maya.cmds.playbackOptions(query = True, maxTime = True));
        scene_end_frame = int(maya.cmds.playbackOptions(query = True, minTime = True));
        
        cur_layout = maya.cmds.rowColumnLayout( numberOfColumns=2, columnAttach=(1, 'right', 0), columnWidth=[(1, 100), (2, 250)] );
        maya.cmds.text( label='Animation Name' );
        animation_name = maya.cmds.textField(text='Take 001');
        maya.cmds.text( label='Start Frame' );
        animation_start_frame = maya.cmds.intField(value = scene_start_frame);
        maya.cmds.text( label='End Frame' );
        animation_end_frame = maya.cmds.intField(value = scene_end_frame);
        maya.cmds.text( label='FPS' );
        animation_fps = maya.cmds.intField(value=scene_fps);
        maya.cmds.text( label='' );
        deleteButton = maya.cmds.button(label="Delete", command=functools.partial(self.DeleteButtonClick, cur_layout));

        maya.cmds.separator( height=10, style='singleDash');
   
   cmds.optionVar( iv=('defaultTriangles', 4), sv=('defaultFileName', 'buffalo.maya') )
    def DeleteButtonClick(self, cur_layout, *args):
        maya.cmds.deleteUI(cur_layout);
             
    def deleteUI(self, *args):
        if maya.cmds.window(self.winName, exists=True):
            maya.cmds.deleteUI(self.winName);
        if maya.cmds.windowPref(self.winName, exists=True):
            maya.cmds.windowPref(self.winName, remove=True);
           
    def createUI(self):
        
        ## Create Window
        createWin = maya.cmds.window(self.winName, title=self.winTitle, widthHeight=(400, 600));
        
        mainLayout = maya.cmds.formLayout();

        self.scrollLayout = maya.cmds.scrollLayout(childResizable = True);
        
        maya.cmds.setParent("..");

        addButton = maya.cmds.button(label="Add", command=self.AddButtonClick);
        maya.cmds.setParent("..");
        
        
        maya.cmds.formLayout(mainLayout, edit=True, attachForm=[(self.scrollLayout, "top", 0), (self.scrollLayout, "right", 0), (self.scrollLayout, "left", 0), (addButton, "bottom", 0), (addButton, "left", 0), (addButton, "right", 0)], attachControl=[(self.scrollLayout, "bottom", 5, addButton)]);
        maya.cmds.showWindow(self.winName);
                 
            
    def __init__(self):
        self.winTitle = "Animation Define";
        self.winName = "IMAnimationWin";
        
        self.deleteUI();
        self.createUI();

OSGAnimation();
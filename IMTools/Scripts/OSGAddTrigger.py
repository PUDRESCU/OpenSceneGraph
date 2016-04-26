import maya.mel;
import maya.cmds;
import functools;
import OSGAnimation;

TRIGGERS_DEF = 'none:mouthopen start:mouthopen end:eyesclosed start:eyesclosed end:browsraised start:browsraised end:headup start:headup end:headdown start:headdown end:headleft start:headleft end:headright start:headright end';
ACTION_DEF = 'run:pause:hide:show';
   
class OSGAddTrigger:
    def updateUI(self, name_prefix):
        is_apply_to_3danimation = maya.cmds.getAttr(name_prefix+'.applyToGlobal3DAnimation');
        action = maya.cmds.getAttr(name_prefix+'.action');
    
        if is_apply_to_3danimation and action > 1:
            maya.cmds.warning( "3D animation only supports run/pause actions!" );
            maya.cmds.setAttr('%s.action'%name_prefix, 0);
            return;
            
            
    def __init__(self):
        select_nodes = maya.cmds.ls(selection=True);
        
        ANIMATION_NAME_DEF = '';
        animationData = OSGAnimation.OSGAnimation.importData();  
        if animationData:
            index = 0;
            animationNameList = animationData.keys()
            for oneAnimation in animationNameList:
                if index == 0:
                    ANIMATION_NAME_DEF = oneAnimation;
                else:
                    ANIMATION_NAME_DEF = ANIMATION_NAME_DEF+':'+oneAnimation;
                index += 1;
            
        if select_nodes:
            for one_select in select_nodes:
                try:
                    maya.cmds.deleteAttr(one_select, at='triggers');
                except:
                    print();
                
                    
                maya.cmds.addAttr(one_select, ln = "triggers", at="compound", nc = 7, m=True);
                maya.cmds.addAttr(one_select, at = 'enum', keyable=True, en =TRIGGERS_DEF, ln='oneTrigger', p='triggers');
                maya.cmds.addAttr(one_select, at = 'enum', keyable=True, en =ACTION_DEF, ln='action', p='triggers');
                maya.cmds.addAttr(one_select, at = 'float', ln='timeoutSecs', p='triggers', defaultValue=-1.0);
                maya.cmds.addAttr(one_select, at = 'bool', ln='autoTimeout', p='triggers', defaultValue=False);
                maya.cmds.addAttr(one_select, at = 'bool', ln='triggeredDefault', p='triggers', defaultValue=False);
                maya.cmds.addAttr(one_select, at = 'bool', ln='applyToGlobal3DAnimation', p='triggers', defaultValue=False);
                maya.cmds.addAttr(one_select, at = 'enum', keyable=True, en =ANIMATION_NAME_DEF, ln='animationName', p='triggers');

                for i in range(100):
                    maya.cmds.scriptJob(runOnce=False, attributeChange=[one_select+'.triggers[%d].action'%i, functools.partial(self.updateUI, '%s.triggers[%d]'%(one_select, i))] );
                    maya.cmds.scriptJob(runOnce=False, attributeChange=[one_select+'.triggers[%d].applyToGlobal3DAnimation'%i, functools.partial(self.updateUI, '%s.triggers[%d]'%(one_select, i))] );


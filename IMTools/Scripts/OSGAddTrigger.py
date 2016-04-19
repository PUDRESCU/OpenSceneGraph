import maya.mel;
import maya.cmds;

TRIGGERS_DEF = 'none:mouthopen:mouthclose';
ACTION_DEF = 'run:pause:hide:show';

def updateUI(name_prefix):
    is_apply_to_3danimation = maya.cmds.getAttr(name_prefix+'.applyToGlobal3DAnimation');
    action = maya.cmds.getAttr(name_prefix+'.action');

    # For global 3D animation, only run/pause actions support
    if is_apply_to_3danimation and action > 1:
        maya.cmds.warning( "3D animation only supports run/pause actions!" );
        maya.cmds.setAttr('%s.action'%name_prefix, 0);
            
class OSGAddTrigger:
    
    def __init__(self):
        select_nodes = maya.cmds.ls(selection=True);
        if select_nodes:
            for one_select in select_nodes:
                attrExist = maya.cmds.attributeQuery('triggers', node=one_select, exists=True);
                if attrExist:
                    maya.cmds.deleteAttr(one_select, at='triggers');
                maya.cmds.addAttr(one_select, ln = "triggers", at="compound", nc = 6, m=True);
                maya.cmds.addAttr(one_select, at = 'enum', keyable=True, en =TRIGGERS_DEF, ln='oneTrigger', p='triggers');
                maya.cmds.addAttr(one_select, at = 'enum', keyable=True, en =ACTION_DEF, ln='action', p='triggers');
                maya.cmds.addAttr(one_select, at = 'float', ln='timeoutSecs', p='triggers', defaultValue=-1.0);
                maya.cmds.addAttr(one_select, at = 'bool', ln='autoTimeout', p='triggers', defaultValue=False);
                maya.cmds.addAttr(one_select, at = 'bool', ln='triggeredDefault', p='triggers', defaultValue=False);
                maya.cmds.addAttr(one_select, at = 'bool', ln='applyToGlobal3DAnimation', p='triggers', defaultValue=False);
                for i in range(100):
                    maya.cmds.scriptJob(runOnce=False, attributeChange=[one_select+'.triggers[%d].action'%i, "updateUI('%s.triggers[%d]')"%(one_select, i)] );










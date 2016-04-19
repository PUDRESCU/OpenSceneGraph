try:
    import sys
except ImportError: 
    print "sys import failed"
    
try:
    import os
except ImportError: 
    print "os import failed"

try:
    import maya.mel
except ImportError: 
    print "maya.mel import failed"
    
try:
    import maya.cmds as cmds
except ImportError: 
    print "maya.cmds import failed"
    
try:
    from xml.dom import minidom
except ImportError: 
    print "minidom import failed"


    
pluginPath = os.environ.get('IMAGEMETRICS_TOOLS_PLUGIN_PATH', None)
iconsPath = os.path.join(pluginPath, 'Icons');
scriptsPath = os.path.join(pluginPath, 'Scripts');
configsPath = os.path.join(pluginPath, 'Configs');
prjConfFiles = configsPath;

sys.path.append(scriptsPath)


def initializePlugin(mobject):
    
    shelfConfFile = os.path.join(prjConfFiles, 'IMToolsShelfConf.xml')
    
    if os.path.exists(shelfConfFile) == True :
        maya.mel.eval('if (`shelfLayout -exists "IMTools" `) deleteUI "IMTools";')
        shelfTab = maya.mel.eval('global string $gShelfTopLevel;')
        maya.mel.eval('global string $imtoolsShelf;')
        maya.mel.eval('$imtoolsShelf = `shelfLayout -cellWidth 33 -cellHeight 33 -p $gShelfTopLevel "IMTools"`;')

        xmlMenuDoc = minidom.parse(shelfConfFile)
        
        for eachShelfItem in xmlMenuDoc.getElementsByTagName("shelfItem") :
            getIcon = eachShelfItem.attributes['icon'].value
            shelfBtnIcon = os.path.join(iconsPath, getIcon)
            getAnnotation = eachShelfItem.attributes['ann'].value
            getCommand = eachShelfItem.attributes['cmds'].value

            cmds.shelfButton(command=getCommand, annotation=getAnnotation, image=shelfBtnIcon)

        print('//-- IMTools shelf successfully loaded --//');
    
    
def uninitializePlugin(mobject):
    maya.mel.eval('if (`shelfLayout -exists "IMTools" `) deleteUI "IMTools";')

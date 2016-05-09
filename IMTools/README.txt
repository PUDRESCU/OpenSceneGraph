* How to install
modify the maya.env file IMAGEMETRICS_TOOLS_PLUGIN_PATH to your install path, 
then copy maya.env to your maya folder (usually it is located at /Users/username/Library/Preferences/Autodesk/maya/version for mac,
and drive:\Documents and Settings\username\My Documents\maya\version for windows)
run maya and select Windows->Settings/Preferences->Plug-in Manager 
You should see there is IMToolsShelf.py plugin check it as load and autoload.

* Add GLSL shaders
click “Shader” button inside IMTools shelf. It will automatically add three shaders: mojo_rgba, texture_rgba, 
occlusion_rgba. mojo_rgba is usually used for facemask mesh. texture_rgba is used for simple texture mapping.
And occlusion_rgba is used for OcclusionMesh. If you need to create another copy of material,
 you can copy/paste in hypershade UI. You can rename the material as you want but should keep the “mojo_rgba”,
“texture_rgba”, or “occlusion_rgba” inside the name. Because we use these key words to find correct shader type.

* Define Animation data
click “Anim” button inside IMTools shelf. There is an Animation Define UI shown up. There are three buttons
at the bottom: “Add” to add a new animation definition. “Save” to save your animation definition. “Load” to
load previous definition. If you continue a previous work, you can click “Load” button to load old data. If
you just start a new job, then click “Add” button. After clicking “Add” button, a default animation clip will
be added. You can modify the values: animation name, start frame index, end frame index, and desired FPS. 
You can click the “Delete” button to delete an animation clip. And click “Add” button to add more animations.
After done, don’t forget click “Save” button to save your work.

* Add trigger
select the objects which you want to add trigger, then click “Trigger” button inside IMTools shelf. you will 
find there is a new triggers attribute added inside attribute editor/Extra Attributes. Now you can click 
"Add New Item" button to add a new trigger. You can configure the trigger type:none means there is no trigger, mouthopen start, mouthopen end etc. trigger action: run/pause animation, show/hide this object etc. 
Timeout Secs: means the seconds after that the triggered action will be stopped (running animation will paused, 
shown object will be hidden or vice visa). if timeout seconds < 0 then it will be never timeout. If you check 
"Auto Timeout" then the timeout value will be computed as animation duration automatically. Triggered Default means
whether the trigger should be executed while the scene loaded.  "Apply to Global 3D Animation" means
this trigger should apply to global 3D animation, we can add global 3D animation trigger at any
node. But for these triggers only run/pause actions will be accepted. If "Apply to Global 3D Animation" is checked,
you can select the animation name from you defined animation list. You can delete a trigger by click the trashcan 
icon. You can add multiple triggers for an object: e.g. one “mouthopen start” trigger to show and one “mouthopen end”
trigger to hide.

* Export scene
selec the objects you want to export (usually should be the “assets” node): notice the hidden objects will be exported if they are inside the selected objects. Click “Export” button inside IMTools shelf. This script will first add a occlusion_rgba shader to occlusionMesh if there is no material found. Then it will create an extra parameter file for image sequences and triggers. After that it will export a .fbx file and convert it to a .osgt file. There will be a popup dialog to ask you to view the osgt file. If you click "Yes", then the OSG viewer will appear and load
the .osgt file. You can press Esc button to quit the viewer. Finally, there will be a popup dialog to ask you to 
create a device deployment package. If you click "Yes", then there is a *_deployment.zip file will be created inside your "maya scene file folder/osg_export" which can be sent to run on device. In order to test the asset on an iOS device, you 
can use iExplorer(https://www.macroplant.com/iexplorer/) to open your device, find Apps->FacePaintInternalDemo->Library->Caches, then can uncompress the *_deployment.zip file inside osg_export folder and upload the files into your device’s Caches folder. Exit FacePaintInternalDemo and restart.

Hints:
* occlusion mesh should be named as “OcclusionMesh" (just keep it unchanged)
* image sequence should named as fileNamePrefix.#####.png (start index from 0) e.g. imagesequence01.00000.png

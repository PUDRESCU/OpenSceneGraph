SET(TARGET_SRC osgviewer.cpp
  ../osgconv/ShaderConstants.cpp
  ../osgconv/Texture2DImageSequence.cpp
  ../osgconv/TriggerParameterNode.cpp
  UpdateNodeVisibility.cpp
)

SET(TARGET_H
  ../osgconv/ShaderConstants.h
  ../osgconv/NodeVisitorHelper.h
  ../osgconv/Texture2DImageSequence.h
  ../osgconv/TriggerParameterNode.h
   UpdateNodeVisibility.h
)

SET(TARGET_ADDED_LIBRARIES osgAnimation)

SETUP_APPLICATION(osgviewer)

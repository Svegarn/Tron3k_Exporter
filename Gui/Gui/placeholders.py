import maya.OpenMaya as OM
import pymel.core as pm

def replacePHs(input):
    transPath = OM.MDagPath()

    nrOfMeshes = 0  
    nrOfNewMeshes = 0
    nrOfNewMeshes = 0
    translationList = []
    placeHolderNameList = []
    transformList = []

    transIt = OM.MItDag(OM.MItDag.kBreadthFirst, OM.MFn.kTransform)
    
    selectedObject = pm.ls(sl=True, tr=True)
    
    if not pm.attributeQuery("Placeholder", node=selectedObject[0], exists=True):
        while not transIt.isDone():
            if not(transIt.getPath(transPath)):
                transform = OM.MFnTransform(transPath)    
    
                #Names + Positions-----------------------------
                try:
                    #get parent node (transform node to our shape node
                    attribute = transform.name() + ".Placeholder"
                    placeHolderName = pm.getAttr(attribute)

                    transformList.append(transform.name())

                    #get the translationValue of said node
                
                    allTranslations = pm.xform(transform.name(), query = True, worldSpace = True, matrix = True)
                    translationList.append(allTranslations)
                    placeHolderNameList.append(placeHolderName)

                    nrOfMeshes+=1
                except:
                    pass
                                                    
            transIt.next()
    
    
        for i in range(nrOfMeshes):
            if (placeHolderNameList[i] == input):
                if(nrOfNewMeshes == 0):
                    this = pm.ls(sl=True)
                    room = (pm.listRelatives(transformList[i], parent=True, fullPath=True))
                    try:
                        pm.parent(this, room[0])
                    except:
                        pass
                    pm.xform(worldSpace = True, matrix = translationList[i])
    
                else:
                    this = pm.duplicate()
                    room = (pm.listRelatives(transformList[i], parent=True, fullPath=True))
                    try:
                        pm.parent(this, room[0])
                    except:
                        pass

                    pm.xform(worldSpace = True, matrix = translationList[i])
    
                nrOfNewMeshes+=1
                pm.delete(transformList[i]) 
    else:
        print "Plese select something that is not a placeholder"
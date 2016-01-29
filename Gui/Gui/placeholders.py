import maya.OpenMaya as OM
import pymel.core as pm

def replacePHs(input):
    print "AH!"
    print input    
    meshPath = OM.MDagPath()

    nrOfTotalMeshes = 0
    nrOfNewMeshes = 0
    nrOfMeshes = 0  
    meshList = []
    meshNameList = []
    allMeshNameList = []
    translationList = []
    placeHolderIDList = []
    placeHolderNameList = []

    placeHolderExists = True

    meshIt = OM.MItDag(OM.MItDag.kBreadthFirst, OM.MFn.kMesh)
    while not meshIt.isDone():
        if not(meshIt.getPath(meshPath)):
            thisMesh = OM.MFnMesh(meshPath)    

            #Names + Positions-----------------------------
            allMeshNameList.append(thisMesh.name())


            meshList = pm.listRelatives(allMeshNameList, parent=True, fullPath=True)

            try:
                placeHolderName = pm.getAttr(meshList[nrOfTotalMeshes].PlaceHoldername)
                placeHolderExists = True
            except:
                placeHolderExists = False                                        

            if(placeHolderExists == True):

                #Names + Positions-----------------------------
                meshNameList.append(thisMesh.name())                

                #get parent node (transform node to our shape node
                transformList = pm.listRelatives(meshNameList, parent=True, fullPath=True)
                placeHolderName = pm.getAttr(transformList[nrOfMeshes].PlaceHoldername)                

                #get the translationValue of said node

                allTranslations = pm.xform(transformList[nrOfMeshes], query = True, worldSpace = True, matrix = True)
                translationList.append(allTranslations)
                placeHolderNameList.append(placeHolderName)

                nrOfMeshes+=1

            nrOfTotalMeshes+=1

        meshIt.next()


    for i in range(nrOfMeshes):
        if (placeHolderNameList[i] == input):
            if(nrOfNewMeshes == 0):
                this = pm.ls(sl=True)
                room = (pm.listRelatives(transformList[i], parent=True, fullPath=True))
                try:
                    pm.parent(this, room[0])
                except:
                    print transformList[i] + " has no parent"
                pm.xform(worldSpace = True, matrix = translationList[i])
                print "new mesh made"

            else:
                this = pm.duplicate()
                room = (pm.listRelatives(transformList[i], parent=True, fullPath=True))
                try:
                    pm.parent(this, room[0])
                except:
                    print transformList[i] + " has no parent"
                pm.xform(worldSpace = True, matrix = translationList[i])
                print "duplicate made"

            nrOfNewMeshes+=1
            pm.delete(transformList[i]) 
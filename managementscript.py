import maya.cmds as cmds
import maya.OpenMaya as OM
import operator

def gatherSceneData():
	roomDict = {}
	portalDict = {}
	
	transformPath = OM.MDagPath()
	dagIt = OM.MItDag(OM.MItDag.kBreadthFirst, OM.MFn.kTransform)
	while not dagIt.isDone():
		if not(dagIt.getPath(transformPath)):
			transform = OM.MFnTransform(transformPath)

			if transform.hasAttribute("Object_Type"):
				objType = transform.findPlug("Object_Type").asInt()
				
				if (objType == 1):
					roomDict[transform.name()] = transform.findPlug("Object_Id").asInt()
				
				elif (objType == 2):
					portalDict[transform.name()] = (transform.findPlug("ROOM_A").asInt(), transform.findPlug("ROOM_B").asInt())
				
		dagIt.next()

	sorted_roomDict = sorted(roomDict.items(), key=operator.itemgetter(1))
	sorted_portalDict = sorted(portalDict.items(), key=operator.itemgetter(1))
	
	return sorted_roomDict, sorted_portalDict
    
sceneData = gatherSceneData()

for item in sceneData[0]:
	print "%s: %d" % (item[0], item[1])

for item in sceneData[1]:
	print "%s: A(%d) B(%d)" % (item[0], item[1][0], item[1][1])


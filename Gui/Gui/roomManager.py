import maya.OpenMaya as OM
import operator

def gatherRPData():
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
					portalDict[transform.name()] = (transform.findPlug("Object_Id").asInt(), transform.findPlug("ROOM_A").asInt(), transform.findPlug("ROOM_B").asInt())
				
		dagIt.next()

	sorted_roomDict = sorted(roomDict.items(), key=operator.itemgetter(1))
	sorted_portalDict = sorted(portalDict.items(), key=operator.itemgetter(1))
	
	return sorted_roomDict, sorted_portalDict


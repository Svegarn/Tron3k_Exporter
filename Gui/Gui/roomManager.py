import maya.OpenMaya as OM
import maya.cmds as cmds
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

def hideChildrenText(itemList):
	visibility = True
	first = True	

	for item in itemList:
		node = OM.MObject()
		nodeList = OM.MSelectionList()
		nodeList.add(item)
		nodeList.getDependNode(0, node)
		
		dagIt = OM.MItDag(OM.MItDag.kBreadthFirst)
		dagIt.reset(node, OM.MItDag.kBreadthFirst)
		dagIt.next()
	
		while not dagIt.isDone():
			if(dagIt.currentItem().hasFn(OM.MFn.kTransform)):
				nodeName = OM.MFnTransform(dagIt.currentItem()).name()
	
				if cmds.attributeQuery("Object_Type", node=nodeName, exists=True):
					attribute = nodeName + ".visibility"
	
					if first:
						if cmds.getAttr(attribute):
							visibility = False
						first = False			    
	
					cmds.setAttr(attribute, visibility)
	
			elif(dagIt.currentItem().hasFn(OM.MFn.kSpotLight)):
				nodeName = OM.MFnTransform(OM.MFnSpotLight(dagIt.currentItem()).parent(0)).name()
				attribute = nodeName + ".visibility"
	
				if first:
					if cmds.getAttr(attribute):
						visibility = False
					first = False			    
	
				cmds.setAttr(attribute, visibility)	
	
			elif(dagIt.currentItem().hasFn(OM.MFn.kVolumeLight)):
				nodeName = OM.MFnTransform(OM.MFnVolumeLight(dagIt.currentItem()).parent(0)).name()
				attribute = nodeName + ".visibility"
	
				if first:
					if cmds.getAttr(attribute):
						visibility = False
					first = False			    
	
				cmds.setAttr(attribute, visibility)	
	
			dagIt.next()
			
def hideChildrenObject(itemList):
	visibility = True
	first = True	

	for item in itemList:
		node = OM.MObject()
		nodeList = OM.MSelectionList()
		nodeList.add(item.text())
		nodeList.getDependNode(0, node)
		
		dagIt = OM.MItDag(OM.MItDag.kBreadthFirst)
		dagIt.reset(node, OM.MItDag.kBreadthFirst)
		dagIt.next()
	
		while not dagIt.isDone():
			if(dagIt.currentItem().hasFn(OM.MFn.kTransform)):
				nodeName = OM.MFnTransform(dagIt.currentItem()).name()
	
				if cmds.attributeQuery("Object_Type", node=nodeName, exists=True):
					attribute = nodeName + ".visibility"
	
					if first:
						if cmds.getAttr(attribute):
							visibility = False
						first = False			    
	
					cmds.setAttr(attribute, visibility)
	
			elif(dagIt.currentItem().hasFn(OM.MFn.kSpotLight)):
				nodeName = OM.MFnTransform(OM.MFnSpotLight(dagIt.currentItem()).parent(0)).name()
				attribute = nodeName + ".visibility"
	
				if first:
					if cmds.getAttr(attribute):
						visibility = False
					first = False			    
	
				cmds.setAttr(attribute, visibility)	
	
			elif(dagIt.currentItem().hasFn(OM.MFn.kVolumeLight)):
				nodeName = OM.MFnTransform(OM.MFnVolumeLight(dagIt.currentItem()).parent(0)).name()
				attribute = nodeName + ".visibility"
	
				if first:
					if cmds.getAttr(attribute):
						visibility = False
					first = False			    
	
				cmds.setAttr(attribute, visibility)	
	
			dagIt.next()
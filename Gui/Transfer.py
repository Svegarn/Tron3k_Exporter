import pymel.core as pm
import pymel.core.datatypes as dt

sourceList = []
targetList = []

def setSourceList(node):
    children = node.getChildren()
    for child in children:
        sourceList.append(child)
        setSourceList(child)

def setTargetList(node):
    children = node.getChildren()
    for child in children:
        targetList.append(child)
        setTargetList(child)

def getSourceList(node):
    del sourceList[:]
    sourceList.append(node)
    setSourceList(node)

def getTargetList(node):
    del targetList[:]
    targetList.append(node)
    setTargetList(node)

def moveNodeUp(node, sourceList):
    sourceList.insert(sourceList.index(node)+1, sourceList.pop(sourceList.index(node)))

def moveNodeDown(node, nodeList):
    nodeList.insert(nodeList.index(node)-1, nodeList.pop(nodeIndex))
    
def transferRotation(): 
    for element in range(len(targetList)-len(sourceList)):
        try:
            pm.rotate(targetList[len(sourceList)+element], pm.listRelatives(targetList[len(sourceList)+element], p=True)[0].getRotation()) 
            print 'Excessive joints rotated!' + targetList[len(sourceList)+element]    
        except:
            print 'No excessive joints!'+ targetList[len(sourceList)+element]  
            
    for element in range(len(sourceList)):
        try:
            pm.rotate(targetList[element], sourceList[element].getRotation(), pcp = True) 
            print 'All done sire!'
        except:
            print 'Could not transfer data yao!'

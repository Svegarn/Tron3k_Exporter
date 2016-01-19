from maya import OpenMayaUI as omui
from PySide.QtCore import *
from PySide.QtGui import *
from PySide.QtUiTools import *
from shiboken import wrapInstance
from sys import path as pythonPath
from Keyframe import *
from Transfer import *
    
def getMayaWin():
    #obtain a reference to the maya window
    mayaWinPtr = omui.MQtUtil.mainWindow()
    mayaWin    = wrapInstance(long(mayaWinPtr), QWidget)

def loadUI(uiName):
    """Returns QWidget with the UI"""
    # object to load ui files
    loader = QUiLoader()
    # file name of the ui created in Qt Designer
    # directory name (we will update this until we find the file)
    dirIconShapes = ""
    # buffer to hold the XML we are going to load
    buff = None
    # search in each path of the interpreter
    for p in pythonPath:
        fname = p + '/' + uiName
        uiFile = QFile(fname)
        # if we find the "ui" file
        if uiFile.exists():
            # the directory where the UI file is
            dirIconShapes = p
            uiFile.open(QFile.ReadOnly)
            # create a temporary array so we can tweak the XML file
            buff = QByteArray( uiFile.readAll() )
            uiFile.close()
            # the filepath where the ui file is: p + uiname
            break
    else:
        print 'UI file not found'
    # fix XML
    fixXML(buff, p)
    qbuff = QBuffer()
    qbuff.open(QBuffer.ReadOnly|QBuffer.WriteOnly)
    qbuff.write(buff)
    qbuff.seek(0)
    ui = loader.load(qbuff, parentWidget = getMayaWin())
    ui.path = p
    return ui


def fixXML(qbyteArray, path):
    # first replace forward slashes for backslashes
    if path[-1] != '/':
        path = path + '/'
    path = path.replace("/","\\")
    
    # construct whole new path with <pixmap> at the begining
    tempArr = QByteArray( "<pixmap>" + path + "\\")
    
    # search for the word <pixmap>
    lastPos = qbyteArray.indexOf("<pixmap>", 0)
    while ( lastPos != -1 ):
        qbyteArray.replace(lastPos,len("<pixmap>"), tempArr)
        lastPos = qbyteArray.indexOf("<pixmap>", lastPos+1)
    return 

class UIController(QObject):
    def __init__(self, ui):
        QObject.__init__(self)
        
        # connect each signal to it's slot (handler) one by one
        ui.cancel.clicked.connect(self.buttonCancelClicked)
        ui.getNodesSource.clicked.connect(self.buttonGetNodesSourceClicked)
        ui.getNodesTarget.clicked.connect(self.buttonGetNodesTargetClicked)
        
        ui.moveUpSource.clicked.connect(self.buttonMoveUpSourceClicked)
        ui.moveDownSource.clicked.connect(self.buttonMoveDownSourceClicked)
        ui.moveUpTarget.clicked.connect(self.buttonMoveUpTargetClicked)
        ui.moveDownTarget.clicked.connect(self.buttonMoveDownTargetClicked)  
        
        ui.reorientTarget.clicked.connect(self.reorientTargetClicked)
        ui.transferAnimation.clicked.connect(self.transferAnimationClicked)
        
        ui.sourceListWidget.clicked.connect(self.clearTargetSel)
        ui.targetListWidget.clicked.connect(self.clearSourceSel)
        
        self.ui = ui
        self.ui.show()
    def clearTargetSel(self):
        self.ui.targetListWidget.clearSelection()
        
    def clearSourceSel(self):
        self.ui.sourceListWidget.clearSelection()   

    def showUI(self):
        self.ui.show()
        
    def hideUI(self):
        self.ui.hide()
    
    def buttonCancelClicked(self):
        self.ui.close()
    def buttonGetNodesSourceClicked(self):
        try:
            getSourceList(pm.PyNode(self.ui.sourceRootView.toPlainText())) 
            self.ui.sourceListWidget.clear()
            
            for element in sourceList:
                item = QListWidgetItem(element.name())
                self.ui.sourceListWidget.addItem(item)
            
            self.ui.sourceCount.setText('%i' % self.ui.sourceListWidget.count())
        except:
            print 'Invalid source node selected: ' + self.ui.sourceRootView.toPlainText()
    
    def buttonGetNodesTargetClicked(self):
        try:
            getTargetList(pm.PyNode(self.ui.targetRootView.toPlainText()))
            self.ui.targetListWidget.clear()
            
            for element in targetList:
                item = QListWidgetItem(element.name())
                self.ui.targetListWidget.addItem(item)
                
            self.ui.targetCount.setText('%i' % self.ui.targetListWidget.count())
        except:
            print 'Invalid target node selected: ' + self.ui.targetRootView.toPlainText()
    
    def buttonMoveUpSourceClicked(self):
        try:
            moveNodeUp(pm.PyNode(self.ui.sourceListWidget.currentItem().text()), sourceList)
            tempRow = self.ui.sourceListWidget.currentRow()
            item = QListWidgetItem(self.ui.sourceListWidget.takeItem(tempRow))
            self.ui.sourceListWidget.insertItem(tempRow-1, item)
            self.ui.sourceListWidget.setCurrentRow(tempRow-1)
            pass
        except:
            print 'No item selected...'
            pass
        
    def buttonMoveDownSourceClicked(self):
        try:
            moveNodeUp(pm.PyNode(self.ui.sourceListWidget.currentItem().text()), sourceList)
            tempRow = self.ui.sourceListWidget.currentRow()
            item = QListWidgetItem(self.ui.sourceListWidget.takeItem(tempRow))
            self.ui.sourceListWidget.insertItem(tempRow+1, item)
            self.ui.sourceListWidget.setCurrentRow(tempRow+1)
            pass
        except:
            print 'No item selected...'
            pass        
        
    def buttonMoveUpTargetClicked(self):
        try:
            moveNodeUp(pm.PyNode(self.ui.targetListWidget.currentItem().text()), targetList)
            tempRow = self.ui.targetListWidget.currentRow()
            item = QListWidgetItem(self.ui.targetListWidget.takeItem(tempRow))
            self.ui.targetListWidget.insertItem(tempRow-1, item)
            self.ui.targetListWidget.setCurrentRow(tempRow-1)
            pass
        except:
            print 'No item selected...'
            pass        

    def buttonMoveDownTargetClicked(self):
        try:
            moveNodeUp(pm.PyNode(self.ui.targetListWidget.currentItem().text()), targetList) 
            tempRow = self.ui.targetListWidget.currentRow()
            item = QListWidgetItem(self.ui.targetListWidget.takeItem(tempRow))
            self.ui.targetListWidget.insertItem(tempRow+1, item)
            self.ui.targetListWidget.setCurrentRow(tempRow+1)
        except:
            print 'No item selected...'     
        
    def reorientTargetClicked(self):           
        transferRotation()  

    def transferAnimationClicked(self):
        try:
            copyKeyframes()
        except:
            print 'Couldnt complete the animation transfer, sire!'

##reload(loadXMLUI)
##from loadXMLUI import *
##UIController is loadXMLUI.UIController
##cont = UIController(loadUI('iconshapes.ui'))
from maya import OpenMayaUI as omui
import os
import maya.cmds as cmds
from PySide.QtCore import *
from PySide.QtGui import *
from PySide.QtUiTools import *
from shiboken import wrapInstance
from sys import path as pythonPath

a = 0

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

    qbuff = QBuffer()
    qbuff.open(QBuffer.ReadOnly|QBuffer.WriteOnly)
    qbuff.write(buff)
    qbuff.seek(0)
    ui = loader.load(qbuff, parentWidget = getMayaWin())
    ui.path = p
    return ui

class UIController(QObject):
    def __init__(self, ui):
        QObject.__init__(self)
        
        # connect each signal to it's slot (handler) one by one
        ui.buttonExit.clicked.connect(self.buttonExitClicked)
        ui.exportAll.clicked.connect(self.exportAllChecked)
        ui.exportCharacter.clicked.connect(self.uncheckAllBox)
        ui.exportAnimation.clicked.connect(self.uncheckAllBox)
        ui.buttonExportAnimated.clicked.connect(self.buttonExportAnimatedClicked)
        ui.buttonExportStatic.clicked.connect(self.buttonExportStaticClicked)
        
        self.ui = ui
        self.ui.show()

    def showUI(self):
        self.ui.show()
        
    def hideUI(self):
        self.ui.hide()
    
    def buttonExitClicked(self):
        self.ui.close()
        
    def exportAllChecked(self):
        if self.ui.exportAll.isChecked():
            self.ui.exportCharacter.setChecked(1)
            self.ui.exportAnimation.setChecked(1)
        else:
            self.ui.exportCharacter.setChecked(0)
            self.ui.exportAnimation.setChecked(0)  
    
    def uncheckAllBox(self):
        if not self.ui.exportCharacter.isChecked() or not self.ui.exportAnimation.isChecked():
            self.ui.exportAll.setChecked(0)  
        
    def buttonExportAnimatedClicked(self):
        if self.ui.exportCharacter.isChecked() or self.ui.exportAnimation.isChecked():
            cmds.loadPlugin(os.getenv('MAYA_SCRIPT_PATH').split(';')[2] + "/Exporter.mll")
            
            path = cmds.fileDialog2(fm=2, startingDirectory="../../Tron3k/Tron3k/Debug/GameFiles/CharacterFiles/")
            if path:
                perspective = 0;
                if self.ui.exportThird.isChecked():
                    perspective = 1;

                cmds.DataHandler(1, path[0], self.ui.exportCharacter.isChecked(), self.ui.exportAnimation.isChecked(), self.ui.classList.currentRow(), perspective)
                print "Export complete!"
            else:
                print "Path not found..."
                
            cmds.unloadPlugin("Exporter.mll");
        else:
            print "Select at least one item to export..."

    def buttonExportStaticClicked(self):
        if self.ui.exportMap.isChecked():
            cmds.loadPlugin(os.getenv('MAYA_SCRIPT_PATH').split(';')[2] + "/Exporter.mll")
        
            path = cmds.fileDialog2(fm=2, startingDirectory="../../Tron3k/Tron3k/Debug/GameFiles/CharacterFiles/")
            if path:
                cmds.DataHandler(0, path[0])
                print "Export complete!"
            else:
                print "Path not found..."
        else:
            cmds.loadPlugin(os.getenv('MAYA_SCRIPT_PATH').split(';')[2] + "/Exporter.mll")
                    
            path = cmds.fileDialog2(fm=0, startingDirectory="../../Tron3k/Tron3k/Debug/GameFiles/CharacterFiles/", ff="*.bin")
            if path:
                cmds.DataHandler(2, path[0])
                print "Export complete!"
            else:
                print "Path not found..."            
        
            cmds.unloadPlugin("Exporter.mll");
                        
##reload(loadXMLUI)
##from loadXMLUI import *
##UIController is loadXMLUI.UIController
##cont = UIController(loadUI('iconshapes.ui'))
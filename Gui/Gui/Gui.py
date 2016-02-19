from maya import OpenMayaUI as omui
import maya.OpenMaya as OM
import os
import pymel.core as pm
import maya.cmds as cmds
from placeholders import replacePHs
from roomManager import *
from PySide.QtCore import *
from PySide.QtGui import *
from PySide.QtUiTools import *
from shiboken import wrapInstance
from sys import path as pythonPath

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
	
	cmds.loadPlugin(os.getenv('MAYA_SCRIPT_PATH').split(';')[2] + "/OnImportSettings.mll")
	
        # Assembler
        ui.buttonCreateObject.clicked.connect(self.buttonCreateObjectClicked)
        ui.buttonAddObject.clicked.connect(self.buttonAddObjectClicked)
        ui.buttonCreatePh.clicked.connect(self.buttonCreatePhClicked)
        ui.buttonReplacePh.clicked.connect(self.buttonReplacePhClicked)
        ui.buttonCreateOBB.clicked.connect(self.buttonCreateOBBClicked)
        ui.buttonRemoveAttr.clicked.connect(self.buttonRemoveAttrClicked)
        ui.buttonExit2.clicked.connect(self.buttonExitClicked)
	
	# Room/Portal Management
	ui.buttonHide.clicked.connect(self.buttonHideClicked)
	ui.buttonHideAllRoom.clicked.connect(self.buttonHideAllRoomClicked)
	ui.buttonHideAllPortal.clicked.connect(self.buttonHideAllPortalClicked)
	ui.buttonHideContent.clicked.connect(self.buttonHideContentClicked)
	ui.buttonHideContentRoom.clicked.connect(self.buttonHideContentRoomClicked)
	ui.buttonHideContentPortal.clicked.connect(self.buttonHideContentPortalClicked)
	ui.buttonHideContentNonSelected.clicked.connect(self.buttonHideContentNonSelectedClicked)
	ui.buttonHideNonSelected.clicked.connect(self.buttonHideNonSelectedClicked)
	ui.buttonRefresh.clicked.connect(self.buttonRefreshClicked)
	ui.buttonExit3.clicked.connect(self.buttonExitClicked)
        
        # Exporter
        ui.buttonExit.clicked.connect(self.buttonExitClicked)
        ui.exportAll.clicked.connect(self.exportAllChecked)
        ui.exportCharacter.clicked.connect(self.uncheckAllBox)
        ui.exportAnimation.clicked.connect(self.uncheckAllBox)
        ui.buttonExportAnimated.clicked.connect(self.buttonExportAnimatedClicked)
        ui.buttonExportStatic.clicked.connect(self.buttonExportStaticClicked)
        
        # UI
        self.ui = ui
	self.ui.setWindowFlags(Qt.Window | Qt.WindowStaysOnTopHint);
        self.ui.show()
        
        transPath = OM.MDagPath()
        transIt = OM.MItDag(OM.MItDag.kBreadthFirst, OM.MFn.kTransform)
        self.ui.placeholderList.setSortingEnabled(True)
        duplicateCount = 0
        while not transIt.isDone():
            if not(transIt.getPath(transPath)):
                transform = OM.MFnTransform(transPath)          
                
                if pm.attributeQuery("Placeholder", node=transform.name(), exists=True):
                    attribute = transform.name() + ".Placeholder"
                    placeholderType = pm.getAttr(attribute)
                    
                    placeholderExists = self.ui.placeholderList.findItems(placeholderType, Qt.MatchCaseSensitive)
                    if len(placeholderExists) == 0:
                        self.ui.placeholderList.addItem(placeholderType)
    
            transIt.next()        
    # Assembler
    def buttonCreateObjectClicked(self):
	selection = cmds.ls(sl=True, tr=True)
	for item in selection:
	    if not cmds.namespace(exists=(":GENERAL")):
		cmds.namespace(add="GENERAL")
		
	    cmds.namespace(set="GENERAL")
	    
        cmds.ImportHandler(0, self.ui.addObjectGrp.checkedId())
	cmds.namespace(set=":")
        
    def buttonAddObjectClicked(self):
        cmds.ImportHandler(1, self.ui.addObjectGrp.checkedId())
        
    def buttonCreatePhClicked(self):
        itemList = self.ui.placeholderList.findItems(self.ui.lineEdit.text(), Qt.MatchCaseSensitive)
        
        if len(itemList) == 0:
            cmds.ImportHandler(2, str(self.ui.lineEdit.text()))
            self.ui.placeholderList.addItem(self.ui.lineEdit.text())
        else:
            cmds.warning("This placeholder already exists.")
        
    def buttonReplacePhClicked(self):
        itemList = self.ui.placeholderList.selectedItems()
        selection = cmds.ls(sl=True, tr=True)
        if len(itemList) > 0 and len(selection) > 0:
            replacePHs(str(itemList[0].text()))
        else:
            cmds.warning("Please select a placeholder from the list.")
        
    def buttonCreateOBBClicked(self):
        selection = cmds.ls(sl=True, tr=True)
        if len(selection) != 0:
            translate = cmds.xform(selection[0], q=True, t=True)
            
            cmds.polyCube( sx=1, sy=1, sz=1, h=1 )
            cmds.xform(t=translate)
            cmds.select(selection[0], add=True)
            cmds.parent()
        else:
            cmds.warning("Select an object in order to add a boundingbox.")
        
    def buttonRemoveAttrClicked(self):
        selection = cmds.ls(sl=True, tr=True)
        for item in selection:
            if cmds.attributeQuery("Object_Type", node=item, exists=True):
                cmds.deleteAttr(item, at="Object_Type")
            if cmds.attributeQuery("Object_Id", node=item, exists=True):
                cmds.deleteAttr(item, at="Object_Id")
            if cmds.attributeQuery("ROOM_A", node=item, exists=True):
                cmds.deleteAttr(item, at="ROOM_A")   
            if cmds.attributeQuery("ROOM_B", node=item, exists=True):
                cmds.deleteAttr(item, at="ROOM_B")    
            if cmds.attributeQuery("Placeholder", node=item, exists=True):
                cmds.deleteAttr(item, at="Placeholder")
            if cmds.attributeQuery("Ancestor", node=item, exists=True):
                cmds.deleteAttr(item, at="Ancestor") 
        
    

    # Room/Portal Management
    def portalEdited(self):
	value = int(self.ui.PortalTable.currentItem().text())
	
	if(self.ui.PortalTable.currentColumn() == 0):
	    attribute = self.ui.PortalTable.item(self.ui.PortalTable.currentRow(), 3).text() + ".Object_Id"
	    cmds.setAttr(attribute, value)
	elif(self.ui.PortalTable.currentColumn() == 1):
	    attribute = self.ui.PortalTable.item(self.ui.PortalTable.currentRow(), 3).text() + ".ROOM_A"
	    cmds.setAttr(attribute, value)
	elif(self.ui.PortalTable.currentColumn() == 2):
	    attribute = self.ui.PortalTable.item(self.ui.PortalTable.currentRow(), 3).text() + ".ROOM_B"
	    cmds.setAttr(attribute, value)	
	
    def portalSelected(self):
	cmds.select(clear=True)
	self.ui.RoomTable.clearSelection()
	
	selected = self.ui.PortalTable.selectedItems()

	for item in selected:
	    cmds.select(self.ui.PortalTable.item(item.row(), 3).text(), add=True)
    
    def roomEdited(self):
	value = int(self.ui.RoomTable.currentItem().text())
	attribute = self.ui.RoomTable.item(self.ui.RoomTable.currentRow(), 1).text() + ".Object_Id"
	cmds.setAttr(attribute, value)
    
    def roomSelected(self):
	cmds.select(clear=True)
	self.ui.PortalTable.clearSelection()
	
	selected = self.ui.RoomTable.selectedItems()
	
	for item in selected:
	    cmds.select(self.ui.RoomTable.item(item.row(), 1).text(), add=True)
    
    def buttonHideClicked(self):
	selection = cmds.ls(sl=True, tr=True)
	visibility = True;
	
	if cmds.getAttr(selection[0] + ".visibility"):
	    visibility = False	
	
	for item in selection:
	    attribute = item + ".visibility"
	    cmds.setAttr(attribute, visibility)
    
    def buttonHideContentClicked(self):
	tableItems = self.ui.PortalTable.selectedItems()
	if not (len(tableItems) > 0):
	    tableItems = self.ui.RoomTable.selectedItems()
	    
	for item in tableItems:	    
	    visibility = True
	    first = True
	    children = cmds.listRelatives(item.text(), ad=True)
	    
	    for child in children:
		if cmds.attributeQuery("Object_Type", node=child, exists=True):
		    if first:
			if cmds.getAttr(child + ".visibility"):
			    visibility = False
			first = False
			
		    attribute = child + ".visibility"
		    cmds.setAttr(attribute, visibility)	

    def buttonHideNonSelectedClicked(self):
	oldSelection = cmds.ls(sl=True, tr=True)
	newSelection = []
	
	tableItems = self.ui.PortalTable.selectedItems()
	if (len(tableItems) > 0):
	    for i in range(self.ui.PortalTable.rowCount()):
		newSelection.append(self.ui.PortalTable.item(i, 3).text())
	else:
	    for i in range(self.ui.RoomTable.rowCount()):
		newSelection.append(self.ui.RoomTable.item(i, 1).text())	    
	
	for item in oldSelection:
	    newSelection.remove(item)
	    
	visibility = True;
	if cmds.getAttr(newSelection[0] + ".visibility"):
	    visibility = False	
	
	for item in newSelection:
	    attribute = item + ".visibility"
	    cmds.setAttr(attribute, visibility)
    
    def buttonHideContentNonSelectedClicked(self):
	portalItems = self.ui.PortalTable.selectedItems()
	roomItems = self.ui.RoomTable.selectedItems()
	if (len(tableItems) > 0) and (len(tableItems > 0)):
	    nonSelectedItems = []
	    
	    if(len(tableItems) > 0): ################################# START HERE! FILL LIST AND REMOVE SELECTION ->>>>>>>>>>>> FIX SHIT
		visibility = True;
		
		if cmds.getAttr(self.ui.PortalTable.item(0, 3).text() + ".visibility"):
		    visibility = False		
		    
		for i in range(self.ui.PortalTable.rowCount()):
		    attribute = self.ui.PortalTable.item(i, 3).text() + ".visibility"
		    cmds.setAttr(attribute, visibility)  	
		
	    for item in tableItems:	    
		visibility = True
		first = True
		children = cmds.listRelatives(item.text(), ad=True)
		
		for child in children:
		    if cmds.attributeQuery("Object_Type", node=child, exists=True):
			if first:
			    if cmds.getAttr(child + ".visibility"):
				visibility = False
			    first = False
			    
			attribute = child + ".visibility"
			cmds.setAttr(attribute, visibility)	
	
    def buttonHideAllRoomClicked(self):
	if(self.ui.RoomTable.rowCount() > 0):
	    visibility = True;
	    
	    if cmds.getAttr(self.ui.RoomTable.item(0, 1).text() + ".visibility"):
		visibility = False		
		
	    for i in range(self.ui.RoomTable.rowCount()):
		attribute = self.ui.RoomTable.item(i, 1).text() + ".visibility"
		cmds.setAttr(attribute, visibility)      	      
	
    def buttonHideContentRoomClicked(self):
	print "hej"    
    
    def buttonHideAllPortalClicked(self):
	if(self.ui.PortalTable.rowCount() > 0):
	    visibility = True;
	    
	    if cmds.getAttr(self.ui.PortalTable.item(0, 3).text() + ".visibility"):
		visibility = False		
		
	    for i in range(self.ui.PortalTable.rowCount()):
		attribute = self.ui.PortalTable.item(i, 3).text() + ".visibility"
		cmds.setAttr(attribute, visibility)     
	
    def buttonHideContentPortalClicked(self):
	print "hej"
    
    def buttonRefreshClicked(self):
	try:
	    self.ui.PortalTable.itemChanged.disconnect()
	    self.ui.PortalTable.itemClicked.disconnect()
		    
	    self.ui.RoomTable.itemChanged.disconnect()
	    self.ui.RoomTable.itemClicked.disconnect()
	except:
	    pass
	
	self.ui.RoomTable.setRowCount(0)
	self.ui.PortalTable.setRowCount(0)
	
	rpData = gatherRPData()
	
	for item in rpData[0]:
	    rowCount = self.ui.RoomTable.rowCount()
	    self.ui.RoomTable.setRowCount(rowCount + 1)	  
	    
	    room_id = QTableWidgetItem()
	    room_id.setData(Qt.DisplayRole, item[1])
	    room_id.setTextAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
	    room_id.setForeground(QColor.fromRgb(255,255,255))
	    room_id.setBackground(QColor.fromRgb(120,120,120))
	    self.ui.RoomTable.setItem(rowCount, 0, room_id)
	    
	    room = QTableWidgetItem()
	    room.setData(Qt.DisplayRole, item[0])
	    room.setFlags(Qt.ItemIsEnabled | Qt.ItemIsSelectable)
	    room.setForeground(QColor.fromRgb(255,255,255))
	    self.ui.RoomTable.setItem(rowCount, 1, room)
	
	for item in rpData[1]:
	    rowCount = self.ui.PortalTable.rowCount()
	    self.ui.PortalTable.setRowCount(rowCount + 1)
	    
	    portal_id = QTableWidgetItem()
	    portal_id.setData(Qt.DisplayRole, item[1][0])
	    portal_id.setTextAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
	    portal_id.setForeground(QColor.fromRgb(255,255,255))
	    portal_id.setBackground(QColor.fromRgb(120,120,120))
	    self.ui.PortalTable.setItem(rowCount, 0, portal_id)	
	    	    
	    portal_A = QTableWidgetItem()
	    portal_A.setData(Qt.DisplayRole, item[1][1])
	    portal_A.setTextAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
	    self.ui.PortalTable.setItem(rowCount, 1, portal_A)	
	    
	    portal_B = QTableWidgetItem()
	    portal_B.setData(Qt.DisplayRole, item[1][2])
	    portal_B.setTextAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
	    self.ui.PortalTable.setItem(rowCount, 2, portal_B)	
	    
	    portal = QTableWidgetItem()
	    portal.setData(Qt.DisplayRole, item[0])
	    portal.setFlags(Qt.ItemIsEnabled | Qt.ItemIsSelectable)
	    portal.setForeground(QColor.fromRgb(255,255,255))
	    self.ui.PortalTable.setItem(rowCount, 3, portal)
	
	self.ui.PortalTable.itemChanged.connect(self.portalEdited)
	self.ui.PortalTable.itemClicked.connect(self.portalSelected)
	
	self.ui.RoomTable.itemChanged.connect(self.roomEdited)
	self.ui.RoomTable.itemClicked.connect(self.roomSelected)	
    

    # Exporter
    def buttonExitClicked(self):
	cmds.unloadPlugin("OnImportSettings.mll")
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
            else:
                cmds.confirmDialog(title="Exporter", message="Path not found...       ", button="Ok", defaultButton="Ok", ma="Center")
                
            cmds.unloadPlugin("Exporter.mll");
        else:
            cmds.confirmDialog(title="Exporter", message="Select at least one item to export...       ", button="Ok", defaultButton="Ok", ma="Center")

    def buttonExportStaticClicked(self):
        if self.ui.exportMap.isChecked():
            cmds.loadPlugin(os.getenv('MAYA_SCRIPT_PATH').split(';')[2] + "/Exporter.mll")
        
            path = cmds.fileDialog2(fm=2, startingDirectory="../../Tron3k/Tron3k/Debug/GameFiles/CharacterFiles/")
            if path:
                cmds.DataHandler(0, path[0])
            else:
                cmds.confirmDialog(title="Exporter", message="Path not found...       ", button="Ok", defaultButton="Ok", ma="Center")
            
            cmds.unloadPlugin("Exporter.mll");
            
        else:
            cmds.loadPlugin(os.getenv('MAYA_SCRIPT_PATH').split(';')[2] + "/Exporter.mll")
                    
            path = cmds.fileDialog2(fm=0, startingDirectory="../../Tron3k/Tron3k/Debug/GameFiles/CharacterFiles/", ff="*.bin")
            if path:
                cmds.DataHandler(2, path[0])
            else:
                cmds.confirmDialog(title="Exporter", message="Path not found...       ", button="Ok", defaultButton="Ok", ma="Center")           
        
            cmds.unloadPlugin("Exporter.mll");
            
    def showUI(self):
        self.ui.show()
        
    def hideUI(self):
        self.ui.hide()
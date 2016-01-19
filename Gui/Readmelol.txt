#1 Copy contents of Gui to documents/maya/scripts
#2 Execute runGui from Maya script editor

---------------------------------------------------

To hook WingIDE to Maya, copy wingdpstub.py to documents/maya/scripts and execute the following from Maya script editor:
import wingdbstub
wingdbstub.Ensure()
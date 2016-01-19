# Animation code
from Transfer import *
import pymel.core as pm
import time

def copyKeyframes():
    first = pm.findKeyframe(sourceList[0], which='first')
    last = pm.findKeyframe(sourceList[0], which='last')
    
    print first, last
    curr = first
    
    while curr <= last:
        curr = pm.findKeyframe(sourceList[0], time=curr, which='next')
        pm.setCurrentTime(curr)
        
        for index in range(len(sourceList)):
            print index
            if(index == 0):
                pm.copyKey(sourceList[index], time=curr)
                pm.pasteKey(targetList[index], time=curr)
            else:
                try:
                    pm.copyKey(sourceList[index], time=curr, attribute=['rotateX', 'rotateY', 'rotateZ'])
                    pm.pasteKey(targetList[index], time=curr, attribute=['rotateX', 'rotateY', 'rotateZ'])
                except:
                    pm.setKeyframe(targetList[index], time=curr, attribute=['rotateX', 'rotateY', 'rotateZ'])
        
        if curr==last:
            break
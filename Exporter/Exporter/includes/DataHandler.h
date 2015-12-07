#ifndef DATAHANDLER_H
#define DATAHANDLER_H

#include <algorithm>

#include "Enumerations.h"
#include "maya_includes.h"

class DataHandler : public MPxCommand
{
public:
	map<unsigned int, Prop> propList;
	map<unsigned int, Light> pointLightList;
	map<unsigned int, Light> spotLightList;
	map<string, Material> materialList;
	map<string, unsigned int> textureList;
	map<unsigned int, Portal> portalList;
	vector<unsigned int> capturePoints;
	vector<SpawnPoint> spawnTeamA;
	vector<SpawnPoint> spawnTeamB;
	vector<SpawnPoint> spawnTeamFFA;
	map<unsigned int, ABBox> roomBoxes;

	unsigned int roomCount = 0;
	AnimCharacter character;
	MStatus res;

	DataHandler();
	~DataHandler();

	virtual MStatus doIt(const MArgList&);
	static void* creator();

	void CreateMaterial(MObjectArray materials);
	void CreateProp(MObject object);
	void CreatePointLight(MObject object);
	void CreateSpotLight(MObject object);
	void CreatePortal(MObject object);
	void CreateSpawnPoint(MObject object, unsigned int team);
	void GatherSceneData();
	void CalculateKeyframe(MFnTransform &jointTransform, MMatrix toRoot, vector<MMatrix> &inverseBindpose, vector<Transform> &keyframeData);
	void GatherCharacterData();
	void ExportStatic();
	void ExportCharacter();
};

#endif
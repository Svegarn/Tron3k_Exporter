#ifndef DATAHANDLER_H
#define DATAHANDLER_H

#include <algorithm>

#include "Enumerations.h"
#include "maya_includes.h"

class DataHandler : public MPxCommand
{
public:
	// Statics
	StaticAsset staticAsset;
	map<unsigned int, Prop> propList;
	map<unsigned int, Light> pointLightList;
	map<unsigned int, Light> spotLightList;
	map<string, Material> materialList;
	vector<string> textureList;
	map<unsigned int, Portal> portalList;
	vector<unsigned int> capturePoints;
	vector<SpawnPoint> spawnTeamA;
	vector<SpawnPoint> spawnTeamB;
	vector<SpawnPoint> spawnTeamFFA;
	map<unsigned int, ABBox> roomBoxes;

	// Animated
	AnimAsset character;
	map<string, Animation> animationList;

	unsigned int roomCount = 0;
	MStatus res;
	MStatus noError = MStatus::kSuccess;

	DataHandler();
	~DataHandler();

	virtual MStatus doIt(const MArgList&);
	static void* creator();

	void CreateMaterial(MObjectArray materials);
	void CreateMaterial(MObjectArray materials, map<string, Material> &materialList, vector<string> &textureList);
	void CreateProp(MObject object);
	void CreatePointLight(MObject object);
	void CreateSpotLight(MObject object);
	void CreatePortal(MObject object);
	void CreateSpawnPoint(MObject object, unsigned int team);

	void CalculateKeyframe(MFnIkJoint &joint, MMatrix toRoot, vector<int> &parents, vector<MMatrix> &bindPose, vector<MMatrix> &relativePose, vector<Transform> &keyframeData);

	void GatherMapData();
	void GatherStaticData();
	void GatherCharacterData(bool exportMesh, bool exportAnimations);

	void ExportMap(MString path);
	void ExportStatic(MString path);
	void ExportCharacter(MString path, unsigned int charType, unsigned int perspType);
};

#endif
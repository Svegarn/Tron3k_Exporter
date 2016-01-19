#ifndef DATAHANDLER_H
#define DATAHANDLER_H

#include <algorithm>

#include "Enumerations.h"
#include "maya_includes.h"

class DataHandler : public MPxCommand
{
public:
	// Statics
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
	map<string, AnimAsset> animAssetList;
	map<string, Animation> animationList;

	unsigned int roomCount = 0;
	MStatus res;

	DataHandler();
	~DataHandler();

	virtual MStatus doIt(const MArgList&);
	static void* creator();

	void CreateMaterial(MObjectArray materials);
	void CreateMaterial(MObjectArray materials, AnimAsset& asset);
	void CreateProp(MObject object);
	void CreatePointLight(MObject object);
	void CreateSpotLight(MObject object);
	void CreatePortal(MObject object);
	void CreateSpawnPoint(MObject object, unsigned int team);
	void GatherSceneData();
	void CalculateKeyframe(MFnIkJoint &joint, MMatrix toRoot, vector<int> &parents, vector<MMatrix> &bindPose, vector<MMatrix> &relativePose, vector<Transform> &keyframeData);
	void GatherCharacterData(bool exportMesh, bool exportWeapons, bool exportAnimations);
	void ExportStatic(MString path);
	void ExportCharacter(MString path);
};

#endif
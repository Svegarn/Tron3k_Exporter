#ifndef ENUMERATIONS_H
#define ENUMERATIONS_H

#include <vector>
#include <fstream>
#include <string>
#include <math.h>
#include <map>

using namespace std;

struct Texture {
	string path;
};

struct Vertex {
	float px, py, pz;
	float u, v;
	float nx, ny, nz;
	float tx, ty, tz;
};

struct AnimVertex {
	float pos[3];
	float uv[2];
	float normal[3];
	float tangent[3];
	unsigned int boneIndices[4];
	float skinWeights[4];
};

struct BBox {
	float positions[8][4] = { 0.0f };
};

struct ABBox {
	float abbPositions[3][4] = { 0.0f };
};

struct Transform {
	float matrix[4][4] = { 0.0f };
};

struct Light {
	unsigned int roomId = 0;
	float color[3] = { 0.0f, 0.0f, 0.0f };
	float intensity = 0.0f;
	float position[3] = { 0.0, 0.0, 0.0 };
	float ambientIntensity = 0.0f;
	float direction[3] = { 0.0f, 0.0f, 0.0f };
	float coneAngle = 0.0f;
	float attenuation[4] = { 1.0f, 0.045f, 0.0075f, 0.0f };
};

struct Material {
	int materialId = -1;
	int textureIds[3] = { -1, -1, -1 };
};

struct Portal {
	unsigned int portalId;
	unsigned int bridgedRooms[2];
	float positions[4][4];
};

struct CapturePointHeader {
	vector<unsigned int> AABBCounts;
	vector<unsigned int> WallCounts;
};

struct CapturePointWall {
	Transform transform;
	vector<unsigned int> offsetIndices;
	vector<Vertex> vertices;
};

struct CapturePoint {
	unsigned int roomID;
	ABBox mainAABB;
	vector<ABBox> AABBs;

	vector<unsigned int> indicesCounts;
	vector<unsigned int> vertexCounts;
	vector<CapturePointWall> walls;
};

struct SpawnPoint {
	int roomId;
	float transform[4][4];
	float direction[3];
};

// #### ANIMATED ####
struct AnimAssetHeader {
	unsigned int materialCount = 0;
	unsigned int textureCount = 0;
	unsigned int indexCount = 0;
	unsigned int vertexCount = 0;
};

struct AnimAsset {
	AnimAssetHeader header;

	vector<unsigned int> materialOffsets;

	vector<vector<unsigned int>> offsetIndices; // Per material
	vector<AnimVertex> vertices;

	map<string, Material> materialList;
	vector<string> textureList;
};

struct Animation {
	unsigned int jointCount = 0;
	unsigned int keyCount = 0;

	vector<vector<Transform>> animationMatrices; // Per keyframe, per joint
};
// ##################

// #### STATIC ####
struct FileHeader {
	unsigned int roomCount = 0;
	unsigned int propCount = 0;
	unsigned int pointLightCount = 0;
	unsigned int spotLightCount = 0;
	unsigned int materialCount = 0;
	unsigned int textureCount = 0;
	unsigned int portalCount = 0;
	unsigned int capturePointcount = 0;
	unsigned int SPCountTeamA = 0;
	unsigned int SPCountTeamB = 0;
	unsigned int SPCountTeamFFA = 0;
};

struct PropHeader {
	unsigned int objectType = 0;
	unsigned int instanceCount = 0;
	unsigned int materialCount = 0;
	unsigned int indicesCount = 0;
	unsigned int vertexCount = 0;
	unsigned int bbCount = 0;
};

struct Prop {
	PropHeader header;
	
	vector<unsigned int> roomId; // Per instance
	vector<Transform> transform; // Per instance

	vector<unsigned int> materialIndices;
	vector<unsigned int> materialOffsets;

	vector<vector<unsigned int>> offsetIndices; // Per material
	vector<Vertex> vertices;

	vector<ABBox> abbExtensions;
	vector<BBox> bbPositions; // Per boundingbox
};

struct StaticAssetHeader {
	unsigned int materialCount = 0;
	unsigned int textureCount = 0;
	unsigned int indexCount = 0;
	unsigned int vertexCount = 0;
};

struct StaticAsset {
	StaticAssetHeader header;

	vector<unsigned int> materialOffsets;

	vector<vector<unsigned int>> offsetIndices; // Per material
	vector<Vertex> vertices;

	map<string, Material> materialList;
	vector<string> textureList;
};
// ################

enum ObjectTypes
{
	OBJECT_TYPE_PROP,
	OBJECT_TYPE_ROOM,
	OBJECT_TYPE_PORTAL,
	OBJECT_TYPE_CAPTURE,
	OBJECT_TYPE_SPAWN_A,
	OBJECT_TYPE_SPAWN_B,
	OBJECT_TYPE_SPAWN_FFA,
	OBJECT_TYPE_COUNT
};

static char* characterType[]{
	"Brute",
	"Manipulator",
	"Shanker",
	"Destroyer",
	"Trapper"
};

static char* perspectiveType[]{
	"First",
	"Third"
};

#endif
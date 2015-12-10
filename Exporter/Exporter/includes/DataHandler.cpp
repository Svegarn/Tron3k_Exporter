#include "DataHandler.h"

DataHandler::DataHandler(){
}

DataHandler::~DataHandler(){
	propList.clear();
	pointLightList.clear();
	spotLightList.clear();
	materialList.clear();
	textureList.clear();
	portalList.clear();
}

MStatus DataHandler::doIt(const MArgList& args) {
	if (args.asInt(0) == 0) {
		GatherSceneData();
		ExportStatic();
	}
	else if (args.asInt(0) == 1) {
		GatherCharacterData();
		ExportCharacter();
	}

	setResult("DataHandler Called\n");
	return MS::kSuccess;
}

void* DataHandler::creator() {
	return new DataHandler;
}

void DataHandler::CreateMaterial(MObjectArray materials) {
	for (unsigned int i = 0; i < materials.length(); i++) {
		MPlugArray connections;
		MFnDependencyNode(materials[i]).findPlug("surfaceShader").connectedTo(connections, true, false, &res);
		if (res) {
			MFnLambertShader lambert(connections[0].node());

			// Check if material is already in the list
			map<string, Material>::iterator matIt = materialList.find(lambert.name().asChar());
			if (matIt == materialList.end()) {
				string path;
				Material material;
				material.materialId = (int)materialList.size();

				// Diffuse
				if (lambert.findPlug("color").isConnected()) {
					lambert.findPlug("color").connectedTo(connections, true, false, &res);
					MFnDependencyNode textureNode(connections[0].node());

					// Check if texture is already in the list
					path = textureNode.findPlug("fileTextureName").asString().asChar();
					size_t slash = path.find_last_of("/");
					path = path.substr(slash + 1);
					map<string, unsigned int>::iterator it = textureList.find(path);
					if (it == textureList.end()) {
						textureList[path] = (unsigned int)textureList.size();
					}

					if (path.length() > 0)
						material.textureIds[0] = textureList[path];
				}

				// Normal + Dynamic Glow
				if (lambert.findPlug("normalCamera").isConnected()) {
					lambert.findPlug("normalCamera").connectedTo(connections, true, false, &res);
					MFnDependencyNode bumpNode(connections[0].node());

					if (bumpNode.findPlug("bumpValue").isConnected()) {
						bumpNode.findPlug("bumpValue").connectedTo(connections, true, false, &res);
						MFnDependencyNode textureNode(connections[0].node());

						// Check if texture is already in the list
						path = textureNode.findPlug("fileTextureName").asString().asChar();
						size_t slash = path.find_last_of("/");
						path = path.substr(slash + 1);
						map<string, unsigned int>::iterator it = textureList.find(path);
						if (it == textureList.end()) {
							textureList[path] = (unsigned int)textureList.size();
						}

						if (path.length() > 0)
							material.textureIds[1] = textureList[path];
					}
				}

				// Static Glow + Specular
				if (lambert.findPlug("glowIntensity").isConnected()) {
					lambert.findPlug("glowIntensity").connectedTo(connections, true, false, &res);
					MFnDependencyNode textureNode(connections[0].node());

					// Check if texture is already in the list
					path = textureNode.findPlug("fileTextureName").asString().asChar();
					size_t slash = path.find_last_of("/");
					path = path.substr(slash + 1);
					map<string, unsigned int>::iterator it = textureList.find(path);
					if (it == textureList.end()) {
						textureList[path] = (unsigned int)textureList.size();
					}

					if (path.length() > 0)
						material.textureIds[2] = textureList[path];
				}

				materialList[lambert.name().asChar()] = material;
			}
		}	
	}
}

void DataHandler::CreatePortal(MObject object) {
	MFnDagNode node(object);
	MDagPath path;
	node.getPath(path);
	MFnMesh portal(path);
	MFnTransform portalTransform(portal.parent(0));
	Portal portalData;

	// ID
	portalData.portalId = portalTransform.findPlug("Object_Id", &res).asInt();

	// BridgedRooms
	portalData.bridgedRooms[0] = portalTransform.findPlug("ROOM_A", &res).asInt();
	portalData.bridgedRooms[1] = portalTransform.findPlug("ROOM_B", &res).asInt();

	// Positions
	MPointArray points;
	portal.getPoints(points, MSpace::kWorld);
	for (unsigned int i = 0; i < 4; i++)
		points[i].get(portalData.positions[i]);

	portalList[portalData.portalId] = portalData;
}

void DataHandler::CreateProp(MObject object) {
	MFnMesh mesh(object);
	MFnTransform meshTransform(mesh.parent(0));
	unsigned int objectId = meshTransform.findPlug("Object_Id", &res).asInt();

	if (res) {
		map<unsigned int, Prop>::iterator it = this->propList.find(objectId);
		if (it != this->propList.end()) {
			// Header
			this->propList[objectId].header.instanceCount++;

			// Instances
			MFnTransform roomTransform(meshTransform.parent(0));
			this->propList[objectId].roomId.push_back(roomTransform.findPlug("Object_Id", &res).asInt());

			Transform transform;
			MFnMatrixData data(meshTransform.findPlug("parentMatrix").elementByLogicalIndex(0).asMObject());
			MMatrix ctm = meshTransform.transformationMatrix() * data.matrix(&res);
			ctm.get(transform.matrix);
			this->propList[objectId].transform.push_back(transform);

			// BoundingBoxes
			ABBox meshBox;
			MBoundingBox box = mesh.boundingBox();
			box.transformUsing(ctm);

			box.center().get(meshBox.abbPositions[0]);
			box.max().get(meshBox.abbPositions[1]);
			box.min().get(meshBox.abbPositions[2]);

			this->propList[objectId].abbExtensions.push_back(meshBox);

			MItDag it;
			it.reset(meshTransform.object(), MItDag::kBreadthFirst, MFn::kTransform);
			while (!it.isDone()) {
				if (it.item() != meshTransform.object()) {
					MFnMesh temp(MFnTransform(it.item()).child(0));
					MDagPath childPath;
					temp.getPath(childPath);
					MFnMesh childMesh(childPath);
					MPointArray bbPositions;
					BBox box;

					res = childMesh.getPoints(bbPositions, MSpace::kWorld);
					bbPositions.get(box.positions);
					this->propList[objectId].bbPositions.push_back(box);
				}
				it.next();
			}
		}
		else {
			Prop prop;

			MIntArray vertexCount, posIndices, uvPerPolygonCount, uvIndices, normalPerPolygonArray, normalIndices, materialPerFace, trianglesPerFace, offsetIndices;
			MFloatArray uList, vList;
			MFloatVectorArray tangents;
			MObjectArray connectedShaders;

			float* positions = (float*)mesh.getRawPoints(&res);
			float* normals = (float*)mesh.getRawNormals(&res);

			mesh.getVertices(vertexCount, posIndices);
			mesh.getUVs(uList, vList);
			mesh.getAssignedUVs(uvPerPolygonCount, uvIndices);
			mesh.getNormalIds(normalPerPolygonArray, normalIndices);
			mesh.getConnectedShaders(0, connectedShaders, materialPerFace);
			mesh.getTriangleOffsets(trianglesPerFace, offsetIndices);
			mesh.getTangents(tangents, MSpace::kObject);

			// Get materials
			CreateMaterial(connectedShaders);

			// Header
			prop.header.objectType = meshTransform.findPlug("Object_Type").asInt();
			prop.header.instanceCount = 1;
			prop.header.materialCount = connectedShaders.length();
			prop.header.indicesCount = offsetIndices.length();
			prop.header.vertexCount = posIndices.length();

			// Instances
			MFnTransform roomTransform(meshTransform.parent(0));
			prop.roomId.push_back(roomTransform.findPlug("Object_Id").asInt());

			Transform transform;
			MFnMatrixData data(meshTransform.findPlug("parentMatrix").elementByLogicalIndex(0).asMObject());
			MMatrix ctm = meshTransform.transformationMatrix() * data.matrix(&res);
			ctm.get(transform.matrix);
			prop.transform.push_back(transform);

			// Vertices & Materials
			prop.offsetIndices.resize(connectedShaders.length());
			unsigned int vertCount = 0;
			for (unsigned int i = 0; i < materialPerFace.length(); i++)
				for (unsigned int x = 0; x < (unsigned int)trianglesPerFace[i]; x++)
					for (unsigned int y = 0; y < 3; y++) {
						prop.offsetIndices[materialPerFace[i]].push_back(offsetIndices[vertCount]);
						vertCount++;
					}

			for (unsigned int i = 0; i < connectedShaders.length(); i++) {
				MPlugArray connections;
				MFnDependencyNode(connectedShaders[i]).findPlug("surfaceShader").connectedTo(connections, true, false, &res);
				prop.materialIndices.push_back(this->materialList[MFnLambertShader(connections[0].node()).name().asChar()].materialId);
				prop.materialOffsets.push_back((unsigned int)prop.offsetIndices[i].size());
			}

			for (unsigned int i = 0; i < posIndices.length(); i++) {
				Vertex vertex = {
					positions[posIndices[i] * 3],
					positions[posIndices[i] * 3 + 1],
					positions[posIndices[i] * 3 + 2],

					uList[uvIndices[i]],
					vList[uvIndices[i]],

					normals[normalIndices[i] * 3],
					normals[normalIndices[i] * 3 + 1],
					normals[normalIndices[i] * 3 + 2],

					tangents[normalIndices[i]].x,
					tangents[normalIndices[i]].y,
					tangents[normalIndices[i]].z

				};

				prop.vertices.push_back(vertex);
			}

			// BoundingBoxes
			ABBox meshBox;
			MBoundingBox box = mesh.boundingBox();
			box.transformUsing(ctm);

			box.center().get(meshBox.abbPositions[0]);
			box.max().get(meshBox.abbPositions[1]);
			box.min().get(meshBox.abbPositions[2]);

			prop.abbExtensions.push_back(meshBox);

			MItDag it;
			it.reset(meshTransform.object(), MItDag::kBreadthFirst, MFn::kTransform);
			while (!it.isDone()) {
				if (it.item() != meshTransform.object()) {
					MFnMesh temp(MFnTransform(it.item()).child(0));
					MDagPath childPath;
					temp.getPath(childPath);
					MFnMesh childMesh(childPath);
					MPointArray bbPositions;
					BBox box;

					res = childMesh.getPoints(bbPositions, MSpace::kWorld);
					bbPositions.get(box.positions);
					prop.bbPositions.push_back(box);

					prop.header.bbCount++; // Header
				}
				it.next();
			}

			this->propList[objectId] = prop;
		}
	}
	else {
		MString command = "confirmDialog -title ^1s -message ^2s -defaultButton ^3s -cancelButton ^4s -dismissString ^5s";
		command.format(command, "Confirm", "Are you sure?", "Yes", "No", "No");
		MGlobal::executeCommand(command);
	}
}

void DataHandler::CreatePointLight(MObject object) {
	unsigned int pLightCount = (unsigned int)pointLightList.size();
	MFnDagNode node(object);
	MDagPath path;
	node.getPath(path);
	MFnPointLight light(path);
	MFnTransform lightTransform(light.parent(0));
	
	// Room
	pointLightList[pLightCount].roomId = MFnTransform(lightTransform.parent(0)).findPlug("Object_Id", &res).asInt();

	// Color
	light.color().get(pointLightList[pLightCount].color);

	// Intensity
	pointLightList[pLightCount].intensity = light.intensity();

	// Position
	double position[4];
	lightTransform.getTranslation(MSpace::kObject).get(position);
	pointLightList[pLightCount].position[0] = (float)position[0];
	pointLightList[pLightCount].position[1] = (float)position[1];
	pointLightList[pLightCount].position[2] = (float)position[2];

	// Ambient
	pointLightList[pLightCount].ambientIntensity = 0.0f;

	// Direction
	pointLightList[pLightCount].direction[0] = 0.0f;
	pointLightList[pLightCount].direction[1] = 0.0f;
	pointLightList[pLightCount].direction[2] = 0.0f;

	// ConeAngle (cutoff)
	pointLightList[pLightCount].coneAngle = 3.14f;
}

void DataHandler::CreateSpotLight(MObject object) {
	unsigned int sLightCount = (unsigned int)spotLightList.size();
	MFnSpotLight light(object);
	MFnTransform lightTransform(light.parent(0));

	// Room
	spotLightList[sLightCount].roomId = MFnTransform(lightTransform.parent(0)).findPlug("Object_Id", &res).asInt();

	// Color
	light.color().get(spotLightList[sLightCount].color);

	// Intensity
	spotLightList[sLightCount].intensity = light.intensity();

	// Position
	double position[4];
	lightTransform.getTranslation(MSpace::kObject).get(position);
	spotLightList[sLightCount].position[0] = (float)position[0];
	spotLightList[sLightCount].position[1] = (float)position[1];
	spotLightList[sLightCount].position[2] = (float)position[2];

	// Ambient
	spotLightList[sLightCount].ambientIntensity = 0.0f;

	// Direction
	light.lightDirection(0, MSpace::kWorld).get(spotLightList[sLightCount].direction);

	// ConeAngle (cutoff)
	spotLightList[sLightCount].coneAngle = (float)light.coneAngle();
}

void DataHandler::CreateSpawnPoint(MObject object, unsigned int team) {
	SpawnPoint spawn;
	MFnSpotLight light(object);
	MFnTransform lightTransform(light.parent(0));

	spawn.roomId = MFnTransform(lightTransform.parent(0)).findPlug("Object_Id").asInt();

	MFnMatrixData data(lightTransform.findPlug("parentMatrix").elementByLogicalIndex(0).asMObject());
	MMatrix ctm = lightTransform.transformationMatrix() * data.matrix(&res);
	ctm.get(spawn.transform);

	light.lightDirection(0, MSpace::kWorld).get(spawn.direction);

	if (team == OBJECT_TYPE_SPAWN_A)
		spawnTeamA.push_back(spawn);
	else if (team == OBJECT_TYPE_SPAWN_B)
		spawnTeamB.push_back(spawn);
	else
		spawnTeamFFA.push_back(spawn);
}

void DataHandler::CalculateKeyframe(MFnIkJoint &joint, MMatrix toRoot, vector<int> &parents, vector<MMatrix> &bindPose, vector<MMatrix> &relativePose, vector<Transform> &keyframeData) {

}

//void DataHandler::CalculateKeyframe(MFnIkJoint &joint, MMatrix toRoot, vector<int> &parents, vector<MMatrix> &bindPose, vector<Transform> &keyframeData) {
//	Transform transform;
//	MMatrix newToRoot;
//
//	if (parents[keyframeData.size()] == -1)
//		newToRoot = joint.transformationMatrix();
//	else
//		newToRoot = bindPose[parents[keyframeData.size()]] * joint.transformationMatrix();
//
//
//	MMatrix final = newToRoot * bindPose[keyframeData.size()].inverse();
//
//	final.get(transform.matrix);
//	keyframeData.push_back(transform);
//
//	MItDag dagIt;
//	dagIt.reset(joint.dagPath(), MItDag::kDepthFirst, MFn::kJoint);
//	while (!dagIt.isDone()) {
//		if (dagIt.item() != joint.object())
//			CalculateKeyframe(MFnIkJoint(dagIt.item()), newToRoot, parents, bindPose, keyframeData);
//
//		dagIt.next();
//	}
//}

//void DataHandler::CalculateKeyframe(MFnIkJoint &joint, MMatrix toRoot, MMatrix bpToRoot, vector<MMatrix> &bindPose, vector<Transform> &keyframeData) {
//	Transform transform;
//
//	MTransformationMatrix matrix;
//	double rot[4];
//	MQuaternion quat;
//	MQuaternion jointQuat;
//	joint.getRotationQuaternion(rot[0], rot[1], rot[2], rot[3], MSpace::kTransform);
//
//	matrix.setRotationQuaternion(rot[0], rot[1], rot[2], rot[3], MSpace::kTransform);
//	matrix.setTranslation(joint.getTranslation(MSpace::kTransform), MSpace::kTransform);
//
//	MMatrix newToRoot = matrix.asMatrix() * toRoot;
//	MMatrix(bindPose[keyframeData.size()].inverse() * newToRoot).get(transform.matrix);
//
//	keyframeData.push_back(transform);
//
//	MItDag dagIt;
//	dagIt.reset(joint.dagPath(), MItDag::kDepthFirst, MFn::kJoint);
//	while (!dagIt.isDone()) {
//		if (dagIt.item() != joint.object())
//			CalculateKeyframe(MFnIkJoint(dagIt.item()), newToRoot, bpToRoot, bindPose, keyframeData);
//
//		dagIt.next();
//	}
//}

void DataHandler::GatherSceneData() {
	MItDag dagIt;
	while (dagIt.isDone() != true) {
		if (dagIt.item().hasFn(MFn::kMesh)) {
			MFnMesh mesh(dagIt.item());
			MFnTransform meshTransform(mesh.parent(0));
			if (meshTransform.hasAttribute("Object_Type")) {
				unsigned int objectType = meshTransform.findPlug("Object_Type", &res).asInt();
				if (res) {
					if (objectType == OBJECT_TYPE_PROP)
						CreateProp(dagIt.item());
					else if (objectType == OBJECT_TYPE_PORTAL)
						CreatePortal(dagIt.item());
					else if (objectType == OBJECT_TYPE_CAPTURE)
						capturePoints.push_back(MFnTransform(meshTransform.parent(0)).findPlug("Object_Id", &res).asInt());
					else if (objectType == OBJECT_TYPE_ROOM) {
						unsigned int objectId = MFnTransform(mesh.parent(0)).findPlug("Object_Id", &res).asInt();
						if (objectId != 0) {
							ABBox roomBox;
							meshTransform.boundingBox().center().get(roomBox.abbPositions[0]);
							meshTransform.boundingBox().max().get(roomBox.abbPositions[1]);
							meshTransform.boundingBox().min().get(roomBox.abbPositions[2]);
							roomBoxes[objectId] = roomBox;
						}
						roomCount++;
					}
				}
			}
		}
		else if (dagIt.item().hasFn(MFn::kPointLight))
			CreatePointLight(dagIt.item());
		else if (dagIt.item().hasFn(MFn::kSpotLight)) {
			if (MFnTransform(MFnSpotLight(dagIt.item()).parent(0)).hasAttribute("Object_Type")) {
				CreateSpawnPoint(dagIt.item(), MFnTransform(MFnSpotLight(dagIt.item()).parent(0)).findPlug("Object_Type", &res).asInt());
			}
			else
				CreateSpotLight(dagIt.item());
		}

		dagIt.next();
	}
}

void DataHandler::GatherCharacterData() {
	MDagPath meshPath;

	MItDag dagIt(MItDag::kBreadthFirst, MFn::kMesh);
	while (dagIt.isDone() != true) {
		dagIt.getPath(meshPath);

		if (!MFnMesh(meshPath).isIntermediateObject()) {
			MItDependencyNodes depIt(MFn::kSkinClusterFilter);
			while (depIt.isDone() != true) {
				MFnSkinCluster skinCluster(depIt.item());

				unsigned int meshIndex = skinCluster.indexForOutputConnection(0);
				MDagPath meshPathViaCluster;
				skinCluster.getPathAtIndex(meshIndex, meshPathViaCluster);

				// Make sure the current skincluster is attached to this mesh
				if (meshPath == meshPathViaCluster) {
					vector<MMatrix> jointBindPose;
					vector<int> parentIndices;

					MDagPathArray jointPaths;
					character.header.jointCount = skinCluster.influenceObjects(jointPaths);

					// Iterate through joints
					for (unsigned int i = 0; i < jointPaths.length(); i++) {
						MFnDependencyNode joint(jointPaths[i].node());
						MPlugArray members;
						joint.findPlug("message").connectedTo(members, false, true);

						// Get inverseBindpose
						for (unsigned int x = 0; x < members.length(); x++) {
							if (members[x].node().hasFn(MFn::kDagPose)) {
								MFnDependencyNode bindPose(members[x].node());
								MFnMatrixData matrix(bindPose.findPlug("worldMatrix").elementByPhysicalIndex(i).asMObject(), &res);

								if (res)
									jointBindPose.push_back(matrix.matrix());

								MPlugArray parentPlugs;
								bindPose.findPlug("parents").elementByPhysicalIndex(i).connectedTo(parentPlugs, 1, 0, &res);

								if (res)
									parentIndices.push_back((int)parentPlugs[0].logicalIndex());
							}
						}
					}

					// Step through plugs to find used animationLayers
					MPlugArray layerPlugs;
					MFnDependencyNode(jointPaths[0].node()).findPlug("translateX").connectedTo(layerPlugs, false, true);

					for (unsigned int i = 0; i < layerPlugs.length(); i++) {
						if (layerPlugs[i].node().hasFn(MFn::kAnimLayer)) {
							MPlugArray blendPlugs;
							MFnDependencyNode(layerPlugs[i].node()).findPlug("foregroundWeight").connectedTo(blendPlugs, false, true);
							
							bool layerFound = false;
							for (unsigned int x = 0;x < blendPlugs.length(); x++) {
								if (blendPlugs[x].node().hasFn(MFn::kBlendNodeDoubleLinear)) {
									MPlugArray curvePlugs;
									MFnDependencyNode(blendPlugs[x].node()).findPlug("inputB").connectedTo(curvePlugs, true, false);

									// Set this layer to solo and calculate data for each keyframe, for each joint
									for (unsigned int y = 0; y < curvePlugs.length(); y++) {
										if (curvePlugs[y].node().hasFn(MFn::kAnimCurve)) {
											MString myCommand = "animLayer -e -solo 1 " + MFnDependencyNode(layerPlugs[i].node()).name() + ";";
											MGlobal::executeCommandOnIdle(myCommand);
											
											MAnimControl animControl;
											MTime time;
											vector<vector<Transform>> layerData;
											
											character.header.animationCount++;
											character.animationTypes.push_back(0); // TODO
											character.animationLayerKeyCount.push_back(MFnAnimCurve(curvePlugs[y].node()).numKeys());

											for (unsigned int z = 1; z <= MFnAnimCurve(curvePlugs[y].node()).numKeys(); z++) {
												time.setValue(z);
												animControl.setCurrentTime(time);

												vector<Transform> keyframeData;
												vector<MMatrix> relativePose;

												// Gather joint-data
												for (unsigned int n = 0; n < character.header.jointCount; n++) {
													MFnIkJoint joint(jointPaths[n]);
													Transform transform;
													
													if (n == 0)
														relativePose.push_back(joint.transformationMatrix());
													else
														relativePose.push_back(relativePose[parentIndices[n]] * joint.transformationMatrix());

													MMatrix final = relativePose[n] * jointBindPose[n].inverse();

													final.transpose().get(transform.matrix);
													keyframeData.push_back(transform);
												}

												layerData.push_back(keyframeData);
											}

											character.animationMatrices.push_back(layerData);

											time.setValue(1);
											animControl.setCurrentTime(time);

											// Use break since BNDL nodes with animCurves occur more than once
											layerFound = true;
										}
									}
								}
								if (layerFound == true)
									break;
							}
						}
					}

					MFnMesh mesh(meshPath); // TODO set all layers to mute (to get mesh in "kind of bind pose"...

					// Store Transform
					MFnTransform(mesh.parent(0)).transformationMatrix().transpose().get(character.transform.matrix);

					// Get mesh data
					MIntArray vertexCount, posIndices, uvPerPolygonCount, uvIndices, normalPerPolygonArray, normalIndices, materialPerFace, trianglesPerFace, offsetIndices;
					MFloatArray uList, vList;
					MFloatVectorArray tangents;
					MObjectArray connectedShaders;

					float* positions = (float*)mesh.getRawPoints(&res);
					float* normals = (float*)mesh.getRawNormals(&res);

					mesh.getVertices(vertexCount, posIndices);
					mesh.getUVs(uList, vList);
					mesh.getAssignedUVs(uvPerPolygonCount, uvIndices);
					mesh.getNormalIds(normalPerPolygonArray, normalIndices);
					mesh.getConnectedShaders(0, connectedShaders, materialPerFace);
					mesh.getTriangleOffsets(trianglesPerFace, offsetIndices);
					mesh.getTangents(tangents, MSpace::kObject);

					// Get materials
					CreateMaterial(connectedShaders);

					// Header
					character.header.materialCount = connectedShaders.length();
					character.header.textureCount = (unsigned int)textureList.size();
					character.header.indexCount = offsetIndices.length();
					character.header.vertexCount = posIndices.length();

					// Materials & indices
					character.offsetIndices.resize(connectedShaders.length());
					unsigned int vertCount = 0;
					for (unsigned int i = 0; i < materialPerFace.length(); i++)
						for (unsigned int x = 0; x < (unsigned int)trianglesPerFace[i]; x++)
							for (unsigned int y = 0; y < 3; y++) {
								character.offsetIndices[materialPerFace[i]].push_back(offsetIndices[vertCount]);
								vertCount++;
							}

					for (unsigned int i = 0; i < connectedShaders.length(); i++)
						character.materialOffsets.push_back((unsigned int)character.offsetIndices[i].size());

					// Store/sort 4 weights and joints per vertex
					vector<vector<pair<float, unsigned int>>> weights;
					weights.resize(mesh.numVertices());
					MItGeometry geomIter(meshPath);
					while (!geomIter.isDone()) {

						for (unsigned int i = 0; i < character.header.jointCount; i++) {
							MDoubleArray jointWeight;
							skinCluster.getWeights(meshPath, geomIter.currentItem(), skinCluster.indexForInfluenceObject(jointPaths[i]), jointWeight);

							weights[geomIter.index()].push_back({ (float)jointWeight[0], skinCluster.indexForInfluenceObject(jointPaths[i]) });
						}

						sort(weights[geomIter.index()].begin(), weights[geomIter.index()].end());

						geomIter.next();
					}

					// Build vertices
					for (unsigned int i = 0; i < posIndices.length(); i++) {
						AnimVertex vertex = {
							positions[posIndices[i] * 3],
							positions[posIndices[i] * 3 + 1],
							positions[posIndices[i] * 3 + 2],

							uList[uvIndices[i]],
							vList[uvIndices[i]],

							normals[normalIndices[i] * 3],
							normals[normalIndices[i] * 3 + 1],
							normals[normalIndices[i] * 3 + 2],

							tangents[normalIndices[i]].x,
							tangents[normalIndices[i]].y,
							tangents[normalIndices[i]].z,

							weights[posIndices[i]][character.header.jointCount - 1].second,
							weights[posIndices[i]][character.header.jointCount - 2].second,
							weights[posIndices[i]][character.header.jointCount - 3].second,
							weights[posIndices[i]][character.header.jointCount - 4].second,

							weights[posIndices[i]][character.header.jointCount - 1].first,
							weights[posIndices[i]][character.header.jointCount - 2].first,
							weights[posIndices[i]][character.header.jointCount - 3].first,
							weights[posIndices[i]][character.header.jointCount - 4].first,
						};

						character.vertices.push_back(vertex);
					}
				}

				depIt.next();
			}
		}

		dagIt.next();
	}
}

void readMap() {
	ifstream openFile;
	openFile.open("../Assets/Tron3k_map_0.bin", ios::in | ios::binary);

	// File Header
	FileHeader rFileHeader;
	openFile.read(reinterpret_cast<char*>(&rFileHeader), sizeof(FileHeader));

	cerr << "\n\n\n\n\n\n\n\n\n### FILE HEADER ###";
	cerr << "\nRoomCount: " << rFileHeader.roomCount;
	cerr << "\nPropCount: " << rFileHeader.propCount;
	cerr << "\nPointLightCount: " << rFileHeader.pointLightCount;
	cerr << "\nSpotLightCount: " << rFileHeader.spotLightCount;
	cerr << "\nMaterialCount: " << rFileHeader.materialCount;
	cerr << "\nTextureCount: " << rFileHeader.textureCount;
	cerr << "\nPortalCount: " << rFileHeader.portalCount;

	for (unsigned int i = 0; i < rFileHeader.propCount; i++) {
		// Prop Header
		PropHeader rHeader;
		openFile.read(reinterpret_cast<char*>(&rHeader), sizeof(PropHeader));

		// Prop Data
		unsigned int* roomIDs = new unsigned int[rHeader.instanceCount];
		Transform* transforms = new Transform[rHeader.instanceCount];
		openFile.read(reinterpret_cast<char*>(roomIDs), sizeof(unsigned int) * rHeader.instanceCount);
		openFile.read(reinterpret_cast<char*>(transforms), sizeof(Transform) * rHeader.instanceCount);

		unsigned int* materialIndices = new unsigned int[rHeader.materialCount];
		unsigned int* materialOffsets = new unsigned int[rHeader.materialCount];
		openFile.read(reinterpret_cast<char*>(materialIndices), sizeof(unsigned int) * rHeader.materialCount);
		openFile.read(reinterpret_cast<char*>(materialOffsets), sizeof(unsigned int) * rHeader.materialCount);

		unsigned int* indices = new unsigned int[rHeader.indicesCount];
		Vertex* vertices = new Vertex[rHeader.vertexCount];
		openFile.read(reinterpret_cast<char*>(indices), sizeof(unsigned int) * rHeader.indicesCount);
		openFile.read(reinterpret_cast<char*>(vertices), sizeof(Vertex) * rHeader.vertexCount);

		ABBox* abBoxes = new ABBox[rHeader.instanceCount];
		BBox* bBoxes = new BBox[rHeader.bbCount * rHeader.instanceCount];
		Transform* bbTransforms = new Transform[rHeader.instanceCount * rHeader.bbCount];
		openFile.read(reinterpret_cast<char*>(abBoxes), sizeof(ABBox) * rHeader.instanceCount);
		openFile.read(reinterpret_cast<char*>(bBoxes), sizeof(BBox) * rHeader.bbCount * rHeader.instanceCount);

		cerr << "\n\n### PROP HEADER ###";
		cerr << "\nObject Type: " << rHeader.objectType;
		cerr << "\nInstance Count: " << rHeader.instanceCount;
		cerr << "\nMaterial Count: " << rHeader.materialCount;
		cerr << "\nIndices Count: " << rHeader.indicesCount;
		cerr << "\nVertex Count: " << rHeader.vertexCount;
		cerr << "\nBox Count: " << rHeader.bbCount;

		cerr << "\n\n### PROP DATA ###";
		cerr << "\n### Room IDs: ";
		for (unsigned int x = 0; x < rHeader.instanceCount; x++)
			cerr << "[" << roomIDs[x] << "]";

		for (unsigned int x = 0; x < rHeader.instanceCount; x++) {
			cerr << "\nTransform" << x << ": ";
			for (unsigned int y = 0; y < 4; y++)
				cerr << "[" << transforms[x].matrix[y][0] << ", " << transforms[x].matrix[y][1] << ", " << transforms[x].matrix[y][2] << ", " << transforms[x].matrix[y][3] << "] ";
		}

		cerr << "\n### MaterialIndices: ";
		for (unsigned int x = 0; x < rHeader.materialCount; x++)
			cerr << "[" << materialIndices[x] << "]";

		cerr << "\n### MaterialOffsets: ";
		for (unsigned int x = 0; x < rHeader.materialCount; x++)
			cerr << "[" << materialOffsets[x] << "]";

		cerr << "\n### Indices: ";
		for (unsigned int x = 0; x < rHeader.indicesCount; x++)
			cerr << "[" << indices[x] << "]";

		cerr << "\n### Vertices:";
		for (unsigned int x = 0; x < rHeader.vertexCount; x++) {
			cerr << "\n" << x << "[" << vertices[x].px << ", " << vertices[x].py << ", " << vertices[x].pz << "] ";
			cerr << "[" << vertices[x].u << ", " << vertices[x].v << "] ";
			cerr << "[" << vertices[x].nx << ", " << vertices[x].ny << ", " << vertices[x].nz << "] ";
			cerr << "[" << vertices[x].tx << ", " << vertices[x].ty << ", " << vertices[x].tz << "] ";
		}

		cerr << "\n### ABB:";
		for (unsigned int x = 0; x < rHeader.instanceCount; x++) {
			cerr << "\nBox" << x << ": ";
			for (unsigned int y = 0; y < 3; y++)
				cerr << "[" << abBoxes[x].abbPositions[y][0] << "]" << "[" << abBoxes[x].abbPositions[y][1] << "]" << "[" << abBoxes[x].abbPositions[y][2] << "]" << "[" << abBoxes[x].abbPositions[y][3] << "]";
		}

		cerr << "\n### OBB:";
		for (unsigned int x = 0; x < (rHeader.bbCount * rHeader.instanceCount); x++) {
			cerr << "\nBox" << x << ":";
			for (unsigned int y = 0; y < 8; y++)
				cerr << "\n" << y << "[" << bBoxes[x].positions[y][0] << ", " << bBoxes[x].positions[y][1] << ", " << bBoxes[x].positions[y][2] << ", " << bBoxes[x].positions[y][3] << "]";
		}
	}

	Material* materials = new Material[rFileHeader.materialCount];
	unsigned int* textureSizes = new unsigned int[rFileHeader.textureCount];
	openFile.read(reinterpret_cast<char*>(materials), sizeof(Material) * rFileHeader.materialCount);
	openFile.read(reinterpret_cast<char*>(textureSizes), sizeof(unsigned int) * rFileHeader.textureCount);

	cerr << "\n\n### Materials:";
	for (unsigned int i = 0; i < rFileHeader.materialCount; i++) {
		cerr << "\nMaterial" << materials[i].materialId << ": " << "[" << materials[i].textureIds[0] << "]" << "[" << materials[i].textureIds[1] << "]" << "[" << materials[i].textureIds[2] << "]";
	}

	cerr << "\n### TextureHeader: \nSizes: ";
	for (unsigned int i = 0; i < rFileHeader.textureCount; i++)
		cerr << "[" << textureSizes[i] << "]";

	for (unsigned int i = 0; i < rFileHeader.textureCount; i++) {
		char* texture = new char[textureSizes[i] + 1];
		openFile.read(texture, sizeof(char) * textureSizes[i]);
		texture[textureSizes[i]] = 0;
		cerr << "\nTexture" << i << ": " << texture;
	}

	Light* pointLights = new Light[rFileHeader.pointLightCount];
	openFile.read(reinterpret_cast<char*>(pointLights), sizeof(Light) * rFileHeader.pointLightCount);

	cerr << "\n### PointLights:";
	for (unsigned int i = 0; i < rFileHeader.pointLightCount; i++) {
		cerr << "\n Light" << i << ":";
		cerr << "\nRoomId: " << pointLights[i].roomId;
		cerr << "\nColor: " << pointLights[i].color[0] << ", " << pointLights[i].color[1] << ", " << pointLights[i].color[2];
		cerr << "\nIntensity: " << pointLights[i].intensity;
		cerr << "\nPosition: " << pointLights[i].position[0] << ", " << pointLights[i].position[1] << ", " << pointLights[i].position[2];
		cerr << "\nAmbientIntensity: " << pointLights[i].ambientIntensity;
		cerr << "\nDirection: " << pointLights[i].direction[0] << ", " << pointLights[i].direction[1] << ", " << pointLights[i].direction[2];
		cerr << "\nConeAngle(Cutoff): " << pointLights[i].coneAngle;
		cerr << "\nAttenuation: " << pointLights[i].attenuation[0] << ", " << pointLights[i].attenuation[1] << ", " << pointLights[i].attenuation[2] << "\n";
	}

	Light* spotLights = new Light[rFileHeader.spotLightCount];
	openFile.read(reinterpret_cast<char*>(spotLights), sizeof(Light) * rFileHeader.spotLightCount);

	cerr << "\n\n### SpotLights:";
	for (unsigned int i = 0; i < rFileHeader.spotLightCount; i++) {
		cerr << "\n Light" << i << ":";
		cerr << "\nRoomId: " << spotLights[i].roomId;
		cerr << "\nColor: " << spotLights[i].color[0] << ", " << spotLights[i].color[1] << ", " << spotLights[i].color[2];
		cerr << "\nIntensity: " << spotLights[i].intensity;
		cerr << "\nPosition: " << spotLights[i].position[0] << ", " << spotLights[i].position[1] << ", " << spotLights[i].position[2];
		cerr << "\nAmbientIntensity: " << spotLights[i].ambientIntensity;
		cerr << "\nDirection: " << spotLights[i].direction[0] << ", " << spotLights[i].direction[1] << ", " << spotLights[i].direction[2];
		cerr << "\nConeAngle(Cutoff): " << spotLights[i].coneAngle;
		cerr << "\nAttenuation: " << spotLights[i].attenuation[0] << ", " << spotLights[i].attenuation[1] << ", " << spotLights[i].attenuation[2] << "\n";
	}

	Portal* portals = new Portal[rFileHeader.portalCount];
	openFile.read(reinterpret_cast<char*>(portals), sizeof(Portal) * rFileHeader.portalCount);

	cerr << "\n\n### Portals:";
	for (unsigned int i = 0; i < rFileHeader.portalCount; i++) {
		cerr << "\nID: " << portals[i].portalId;
		cerr << "\nBridgedRooms: " << portals[i].bridgedRooms[0] << ", " << portals[i].bridgedRooms[1];

		cerr << "\nPositions:";
		for (unsigned int x = 0; x < 4; x++)
			cerr << "[" << portals[i].positions[x][0] << ", " << portals[i].positions[x][1] << ", " << portals[i].positions[x][2] << ", " << portals[i].positions[x][3] << "]";

	}

	unsigned int* capturePoints = new unsigned int[rFileHeader.capturePointcount];
	openFile.read(reinterpret_cast<char*>(capturePoints), sizeof(unsigned int) * rFileHeader.capturePointcount);

	cerr << "\n\n### CapturePoints: ";
	for (unsigned int i = 0; i < rFileHeader.capturePointcount; i++) {
		cerr << "[" << capturePoints[i] << "]";
	}

	SpawnPoint* rSpawnTeamA = new SpawnPoint[rFileHeader.SPCountTeamA];
	openFile.read(reinterpret_cast<char*>(rSpawnTeamA), sizeof(SpawnPoint) * rFileHeader.SPCountTeamA);

	cerr << "\n### SpawnTeamA: ";
	for (unsigned int i = 0; i < rFileHeader.SPCountTeamA; i++) {
		cerr << "\nSpawn" << i << ":";
		cerr << "\n[" << rSpawnTeamA[i].roomId << "]";

		for (unsigned int x = 0; x < 4; x++)
			cerr << "\n[" << rSpawnTeamA[i].transform[x][0] << ", " << rSpawnTeamA[i].transform[x][1] << ", " << rSpawnTeamA[i].transform[x][2] << ", " << rSpawnTeamA[i].transform[x][3] << "]";

		cerr << "\n[" << rSpawnTeamA[i].direction[0] << ", " << rSpawnTeamA[i].direction[1] << ", " << rSpawnTeamA[i].direction[2] << "]";
	}

	SpawnPoint* rSpawnTeamB = new SpawnPoint[rFileHeader.SPCountTeamB];
	openFile.read(reinterpret_cast<char*>(rSpawnTeamB), sizeof(SpawnPoint) * rFileHeader.SPCountTeamB);
	cerr << "\n### SpawnTeamB: ";
	for (unsigned int i = 0; i < rFileHeader.SPCountTeamB; i++) {
		cerr << "\nSpawn" << i << ":";
		cerr << "\n[" << rSpawnTeamB[i].roomId << "]";

		for (unsigned int x = 0; x < 4; x++)
			cerr << "\n[" << rSpawnTeamB[i].transform[x][0] << ", " << rSpawnTeamB[i].transform[x][1] << ", " << rSpawnTeamB[i].transform[x][2] << ", " << rSpawnTeamB[i].transform[x][3] << "]";

		cerr << "\n[" << rSpawnTeamB[i].direction[0] << ", " << rSpawnTeamB[i].direction[1] << ", " << rSpawnTeamB[i].direction[2] << "]";
	}

	SpawnPoint* rSpawnTeamFFA = new SpawnPoint[rFileHeader.SPCountTeamFFA];
	openFile.read(reinterpret_cast<char*>(rSpawnTeamFFA), sizeof(SpawnPoint) * rFileHeader.SPCountTeamFFA);
	cerr << "\n### SpawnTeamFFA: ";
	for (unsigned int i = 0; i < rFileHeader.SPCountTeamFFA; i++) {
		cerr << "\nSpawn" << i << ":";
		cerr << "\n[" << rSpawnTeamFFA[i].roomId << "]";

		for (unsigned int x = 0; x < 4; x++)
			cerr << "\n[" << rSpawnTeamFFA[i].transform[x][0] << ", " << rSpawnTeamFFA[i].transform[x][1] << ", " << rSpawnTeamFFA[i].transform[x][2] << ", " << rSpawnTeamFFA[i].transform[x][3] << "]";

		cerr << "\n[" << rSpawnTeamFFA[i].direction[0] << ", " << rSpawnTeamFFA[i].direction[1] << ", " << rSpawnTeamFFA[i].direction[2] << "]";
	}

	ABBox* roomBoxes = new ABBox[rFileHeader.roomCount];
	openFile.read(reinterpret_cast<char*>(roomBoxes), sizeof(ABBox) * (rFileHeader.roomCount - 1));
	cerr << "\n### Room AABBs:";
	for (unsigned int i = 0; i < rFileHeader.roomCount - 1; i++) {
		cerr << "\nRoom" << i << ": ";
		for (unsigned int x = 0; x < 3; x++)
			cerr << "[" << roomBoxes[i].abbPositions[i][0] << ", " << roomBoxes[i].abbPositions[i][1] << ", " << roomBoxes[i].abbPositions[i][2] << ", " << roomBoxes[i].abbPositions[i][3] << "]";
	}

	openFile.close();
}

void DataHandler::ExportStatic() {
	ofstream file;
	file.open("../Assets/Tron3k_map_0.bin", ios::out | ios::binary);

	// File Header
	FileHeader fHeader;
	fHeader.roomCount = this->roomCount;
	fHeader.propCount = (unsigned int)this->propList.size();
	fHeader.pointLightCount = (unsigned int)this->pointLightList.size();
	fHeader.spotLightCount = (unsigned int)this->spotLightList.size();
	fHeader.materialCount = (unsigned int)this->materialList.size();
	fHeader.textureCount = (unsigned int)this->textureList.size();
	fHeader.portalCount = (unsigned int)this->portalList.size();
	fHeader.capturePointcount = (unsigned int)capturePoints.size();
	fHeader.SPCountTeamA = (unsigned int)spawnTeamA.size();
	fHeader.SPCountTeamB = (unsigned int)spawnTeamB.size();
	fHeader.SPCountTeamFFA = (unsigned int)spawnTeamFFA.size();

	file.write(reinterpret_cast<char*>(&fHeader), sizeof(FileHeader));

	for (map<unsigned int, Prop>::iterator propIt = this->propList.begin(); propIt != this->propList.end(); ++propIt) {
		// ### Prop Header ###
		file.write(reinterpret_cast<char*>(&propIt->second.header), sizeof(PropHeader));

		// ### Prop Data ###
		// Instance
		file.write(reinterpret_cast<char*>(propIt->second.roomId.data()), sizeof(unsigned int) * propIt->second.roomId.size());
		file.write(reinterpret_cast<char*>(propIt->second.transform.data()), sizeof(Transform) * propIt->second.transform.size());

		// Material
		file.write(reinterpret_cast<char*>(propIt->second.materialIndices.data()), sizeof(unsigned int) * propIt->second.materialIndices.size());
		file.write(reinterpret_cast<char*>(propIt->second.materialOffsets.data()), sizeof(unsigned int) * propIt->second.materialOffsets.size());

		// Vertices
		for (unsigned int i = 0; i < propIt->second.header.materialCount; i++)
			file.write(reinterpret_cast<char*>(propIt->second.offsetIndices[i].data()), sizeof(unsigned int) * propIt->second.materialOffsets[i]);
		file.write(reinterpret_cast<char*>(propIt->second.vertices.data()), sizeof(Vertex) * propIt->second.vertices.size());
		
		// BoundingBoxes
		file.write(reinterpret_cast<char*>(propIt->second.abbExtensions.data()), sizeof(ABBox) * propIt->second.header.instanceCount);
		file.write(reinterpret_cast<char*>(propIt->second.bbPositions.data()), sizeof(BBox) * (propIt->second.header.bbCount * propIt->second.header.instanceCount));
	}

	for (map<string, Material>::iterator it = materialList.begin(); it != materialList.end(); ++it) {
		// ### Material Data ###
		file.write(reinterpret_cast<char*>(&it->second), sizeof(Material));
	}

	for (map<string, unsigned int>::iterator it = textureList.begin(); it != textureList.end(); ++it) {
		// ### Texture Header ###
		unsigned int pathSize = (unsigned int)it->first.length();
		file.write(reinterpret_cast<char*>(&pathSize), sizeof(unsigned int));
	}

	for (map<string, unsigned int>::iterator it = textureList.begin(); it != textureList.end(); ++it) {
		// ### Texture Data ###
		file.write(it->first.c_str(), sizeof(char) * it->first.length());
	}

	for (map<unsigned int, Light>::iterator it = pointLightList.begin(); it != pointLightList.end(); ++it) {
		// ### PointLight Data ###
		file.write(reinterpret_cast<char*>(&it->second), sizeof(Light));
	}

	for (map<unsigned int, Light>::iterator it = spotLightList.begin(); it != spotLightList.end(); ++it) {
		// ### SpotLight Data ###
		file.write(reinterpret_cast<char*>(&it->second), sizeof(Light));
	}

	for (map<unsigned int, Portal>::iterator it = portalList.begin(); it != portalList.end(); ++it) {
		// ### Portal Data ###
		file.write(reinterpret_cast<char*>(&it->second), sizeof(Portal));
	}

	// ### Capture Points Data ###
	file.write(reinterpret_cast<char*>(capturePoints.data()), sizeof(unsigned int) * capturePoints.size());

	// ### SpawnPoints Team A ###
	file.write(reinterpret_cast<char*>(spawnTeamA.data()), sizeof(SpawnPoint) * spawnTeamA.size());

	// ### SpawnPoints Team B ###
	file.write(reinterpret_cast<char*>(spawnTeamB.data()), sizeof(SpawnPoint) * spawnTeamB.size());

	// ### SpawnPoints Team FFA ###
	file.write(reinterpret_cast<char*>(spawnTeamFFA.data()), sizeof(SpawnPoint) * spawnTeamFFA.size());

	for (map<unsigned int, ABBox>::iterator it = roomBoxes.begin(); it != roomBoxes.end(); ++it) {
		// ### Room AABB ###
		file.write(reinterpret_cast<char*>(&it->second), sizeof(ABBox));
	}
	
	file.close();

	readMap();
}

void DataHandler::ExportCharacter() {
	cerr << "\nEXPORT STARTED";
	ofstream file;
	file.open("C:/Users/Svegarn_/Documents/GitHub/Tron3k/Tron3k/Debug/GameFiles/CharacterFiles/Tron3k_animTest_2.bin", ios::out | ios::binary);

	// Header
	file.write(reinterpret_cast<char*>(&character.header), sizeof(AnimHeader));

	// Transform
	file.write(reinterpret_cast<char*>(&character.transform), sizeof(Transform));

	// MaterialOffsets (number of indices)
	file.write(reinterpret_cast<char*>(character.materialOffsets.data()), sizeof(unsigned int) * character.header.materialCount);

	// Indices
	for (unsigned int i = 0; i < character.header.materialCount; i++)
		file.write(reinterpret_cast<char*>(character.offsetIndices[i].data()), sizeof(unsigned int) * character.materialOffsets[i]);
	
	// Vertices
	file.write(reinterpret_cast<char*>(character.vertices.data()), sizeof(AnimVertex) * character.header.vertexCount);

	// AnimationKeyCounts
	file.write(reinterpret_cast<char*>(character.animationLayerKeyCount.data()), sizeof(unsigned int) * character.header.animationCount);
	cerr << character.animationLayerKeyCount[0];
	// AnimationTypes
	file.write(reinterpret_cast<char*>(character.animationTypes.data()), sizeof(unsigned int) * character.header.animationCount);

	// Animations
	for (unsigned int i = 0; i < character.header.animationCount; i++)
		for (unsigned int x = 0; x < character.animationLayerKeyCount[i]; x++)
			file.write(reinterpret_cast<char*>(character.animationMatrices[i][x].data()), sizeof(Transform) * character.header.jointCount);

	for (map<string, Material>::iterator it = materialList.begin(); it != materialList.end(); ++it) {
		// ### Material Data ###
		file.write(reinterpret_cast<char*>(&it->second), sizeof(Material));
	}

	for (map<string, unsigned int>::iterator it = textureList.begin(); it != textureList.end(); ++it) {
		// ### Texture Header ###
		unsigned int pathSize = (unsigned int)it->first.length();
		file.write(reinterpret_cast<char*>(&pathSize), sizeof(unsigned int));
	}

	for (map<string, unsigned int>::iterator it = textureList.begin(); it != textureList.end(); ++it) {
		// ### Texture Data ###
		file.write(it->first.c_str(), sizeof(char) * it->first.length());
	}

	file.close();

	ifstream openFile;
	openFile.open("C:/Users/Svegarn_/Documents/GitHub/Tron3k/Tron3k/Debug/GameFiles/CharacterFiles/Tron3k_animTest_2.bin", ios::in | ios::binary);

	//// File Header
	AnimHeader rHeader;
	openFile.read(reinterpret_cast<char*>(&rHeader), sizeof(AnimHeader));

	cerr << "\n\n###### HEADER ######";
	cerr << "\nMaterialCount: " << rHeader.materialCount;
	cerr << "\nTextureCount: " << rHeader.textureCount;
	cerr << "\nIndexCount: " << rHeader.indexCount;
	cerr << "\nVertexCount: " << rHeader.vertexCount;
	cerr << "\nAnimationCount: " << rHeader.animationCount;
	cerr << "\nJointCount: " << rHeader.jointCount;

	Transform test;
	openFile.read(reinterpret_cast<char*>(&test), sizeof(Transform));

	unsigned int* offsets = new unsigned int[rHeader.materialCount];
	openFile.read(reinterpret_cast<char*>(offsets), sizeof(unsigned int) * rHeader.materialCount);
	//for (unsigned int i = 0; i < rHeader.materialCount; i++)
	//	cerr << "\n\nOffset: " << offsets[i];

	unsigned int* indices = new unsigned int[rHeader.indexCount];
	openFile.read(reinterpret_cast<char*>(indices), sizeof(unsigned int) * rHeader.indexCount);
	//cerr << "\n\nIndices:";
	//for (unsigned int i = 0; i < rHeader.indexCount; i++)
	//	cerr << "\n" << indices[i];




	openFile.close();
}
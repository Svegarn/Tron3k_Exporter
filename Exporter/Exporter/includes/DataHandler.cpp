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
		GatherMapData();
		if (noError == MStatus::kSuccess) {
			ExportMap(args.asString(1));
			MGlobal::executeCommand("confirmDialog - title \"Exporter\" - message \"Export complete!       \" - button \"Ok\" - defaultButton \"Ok\" - ma \"Center\"");
		}
	}
	else if (args.asInt(0) == 1) {
		GatherCharacterData(args.asBool(2), args.asBool(3));
		if (noError == MStatus::kSuccess) {
			ExportCharacter(args.asString(1), args.asInt(4), args.asInt(5));
			MGlobal::executeCommand("confirmDialog - title \"Exporter\" - message \"Export complete!       \" - button \"Ok\" - defaultButton \"Ok\" - ma \"Center\"");
		}
	}
	else if (args.asInt(0) == 2) {
		GatherStaticData();
		if (noError == MStatus::kSuccess) {
			ExportStatic(args.asString(1));
			MGlobal::executeCommand("confirmDialog - title \"Exporter\" - message \"Export complete!       \" - button \"Ok\" - defaultButton \"Ok\" - ma \"Center\"");
		}
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
				//cerr << "\nMATERIAL ID: " << material.materialId << "(" << lambert.name().asChar() << ")";
				// Diffuse
				if (lambert.findPlug("color").isConnected()) {
					lambert.findPlug("color").connectedTo(connections, true, false, &res);
					MFnDependencyNode textureNode(connections[0].node());

					// Check if texture is already in the list
					path = textureNode.findPlug("fileTextureName").asString().asChar();
					size_t slash = path.find_last_of("/");
					path = path.substr(slash + 1);

					if (path.length() > 0) {
						bool exists = false;
						for (unsigned int x = 0; x < textureList.size(); x++) {
							if (path.compare(textureList[x]) == 0) {
								exists = true;
								material.textureIds[0] = x;
							}
						}
						if (exists == false) {
							textureList.push_back(path);
							material.textureIds[0] = (unsigned int)textureList.size() - 1;
						}
						//cerr << "\nDiffuse: " << material.textureIds[0] << "(" << path << ")";
					}
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

						if (path.length() > 0) {
							bool exists = false;
							for (unsigned int x = 0; x < textureList.size(); x++) {
								if (path.compare(textureList[x]) == 0) {
									exists = true;
									material.textureIds[1] = x;
								}
							}
							if (exists == false) {
								textureList.push_back(path);
								material.textureIds[1] = (unsigned int)textureList.size() - 1;
							}
							//cerr << "\nNormal: " << material.textureIds[1] << "(" << path << ")";
						}
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

					if (path.length() > 0) {
						bool exists = false;
						for (unsigned int x = 0; x < textureList.size(); x++) {
							if (path.compare(textureList[x]) == 0) {
								exists = true;
								material.textureIds[2] = x;
							}
						}
						if (exists == false) {
							textureList.push_back(path);
							material.textureIds[2] = (unsigned int)textureList.size() - 1;
						}
						//cerr << "\nGlow: " << material.textureIds[2] << "(" << path << ")";
					}
				}

				
				
				
				//cerr << "\n";
				materialList[lambert.name().asChar()] = material;
			}
		}	
	}
}

void DataHandler::CreateMaterial(MObjectArray materials, map<string, Material> &materialList, vector<string> &textureList) {
	for (unsigned int i = 0; i < materials.length(); i++) {
		MPlugArray connections;
		MFnDependencyNode(materials[i]).findPlug("surfaceShader").connectedTo(connections, true, false, &res);
		if (res) {
			MFnLambertShader lambert(connections[0].node(), &res);

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

					if (path.length() > 0) {
						bool exists = false;
						for (unsigned int x = 0; x < textureList.size(); x++) {
							if (path.compare(textureList[x]) == 0) {
								exists = true;
								material.textureIds[0] = x;
							}
						}
						if (exists == false) {
							textureList.push_back(path);
							material.textureIds[0] = (unsigned int)textureList.size() - 1;
						}
					}
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

						if (path.length() > 0) {
							bool exists = false;
							for (unsigned int x = 0; x < textureList.size(); x++) {
								if (path.compare(textureList[x]) == 0) {
									exists = true;
									material.textureIds[1] = x;
								}
							}
							if (exists == false) {
								textureList.push_back(path);
								material.textureIds[1] = (unsigned int)textureList.size() - 1;
							}
						}
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

					if (path.length() > 0) {
						bool exists = false;
						for (unsigned int x = 0; x < textureList.size(); x++) {
							if (path.compare(textureList[x]) == 0) {
								exists = true;
								material.textureIds[2] = x;
							}
						}
						if (exists == false) {
							textureList.push_back(path);
							material.textureIds[2] = (unsigned int)textureList.size() - 1;
						}
					}
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
	
	MStatus checkA = MStatus::kFailure;
	MStatus checkB = MStatus::kFailure;

	MItDag dagIt(MItDag::kDepthFirst, MFn::kTransform, &res);
	while (!dagIt.isDone()) {
		MFnTransform transform(dagIt.item());
		if (transform.hasAttribute("Object_Type")) {
			unsigned int objectType = transform.findPlug("Object_Type", &res).asInt();
			if (objectType == OBJECT_TYPE_ROOM) {
				unsigned int objectId = transform.findPlug("Object_Id", &res).asInt();
				if (objectId == portalData.bridgedRooms[0])
					checkA = MStatus::kSuccess;
				if (objectId == portalData.bridgedRooms[1])
					checkB = MStatus::kSuccess;
			}
		}

		dagIt.next();
	}

	if (!checkA || !checkB) {
		MGlobal::executeCommand(MString("error \"Portal assigned to a room that does not exist...\";"));
		MGlobal::executeCommand("confirmDialog - title \"Exporter\" - message \"" + portalTransform.name() + " is connected to a Room that does not exist.       \" - button \"Ok\" - defaultButton \"Ok\" - ma \"Center\"");
		noError = MStatus::kFailure;
	}
	else if (portalData.bridgedRooms[0] == portalData.bridgedRooms[1]) {
		MGlobal::executeCommand(MString("error \"" + portalTransform.name() + " has identical room ids...\";"));
		MGlobal::executeCommand("confirmDialog - title \"Exporter\" - message \"" + portalTransform.name() + " has identical room ids...       \" - button \"Ok\" - defaultButton \"Ok\" - ma \"Center\"");
		noError = MStatus::kFailure;
	}

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
	string meshTransformName = meshTransform.name().asChar();

	int colon = meshTransform.name().rindexW(":") + 1;
	if (colon != -1) {
		meshTransformName = meshTransform.name().substringW(colon, mesh.name().length()).asChar();
	}
	else {
		MGlobal::executeCommandOnIdle(MString("error \"" + meshTransform.name() + " has no namespace assigned.\";"));
		MGlobal::executeCommand("confirmDialog - title \"Exporter\" - message \"" + meshTransform.name() + " has no namespace assigned...       \" - button \"Ok\" - defaultButton \"Ok\" - ma \"Center\"");
		noError = MStatus::kFailure;
	}

	if (res) {
		map<string, Prop>::iterator it = this->propList.find(meshTransformName);
		if (it != this->propList.end()) {
			// Header
			this->propList[meshTransformName].header.instanceCount++;

			// Instances
			MFnTransform roomTransform(meshTransform.parent(0));
			this->propList[meshTransformName].roomId.push_back(roomTransform.findPlug("Object_Id", &res).asInt());

			Transform transform;
			MFnMatrixData data(meshTransform.findPlug("parentMatrix").elementByLogicalIndex(0).asMObject());
			MMatrix ctm = meshTransform.transformationMatrix() * data.matrix(&res);
			ctm.transpose().get(transform.matrix);
			this->propList[meshTransformName].transform.push_back(transform);

			// AABB
			MBoundingBox aabb = mesh.boundingBox();
			aabb.transformUsing(ctm);

			// OBB
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
					this->propList[meshTransformName].bbPositions.push_back(box);

					for (unsigned int i = 0; i < 8; i++) {
						aabb.expand(bbPositions[i]);
					}
				}
				it.next();
			}

			// AABB continued
			ABBox meshBox;
			aabb.center().get(meshBox.abbPositions[0]);
			aabb.max().get(meshBox.abbPositions[1]);
			aabb.min().get(meshBox.abbPositions[2]);

			this->propList[meshTransformName].abbExtensions.push_back(meshBox);

		}
		else {
			Prop prop;

			MIntArray vertexCount, posIndices, uvPerPolygonCount, uvIndices, normalPerPolygonArray, normalIndices, materialPerFace, trianglesPerFace, offsetIndices;
			MFloatArray uList, vList;
			MFloatVectorArray tangents;
			MObjectArray connectedShaders;
			MPoint max(-INFINITY, -INFINITY, -INFINITY);
			MPoint min(INFINITY, INFINITY, INFINITY);

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
			if(connectedShaders.length() > 0)
				CreateMaterial(connectedShaders);
			else {
				MGlobal::executeCommandOnIdle(MString("error \"" + meshTransform.name() + " has no material assigned to it.\";"));
				MGlobal::executeCommand("confirmDialog - title \"Exporter\" - message \"" + meshTransform.name() + " has no material assigned to it...       \" - button \"Ok\" - defaultButton \"Ok\" - ma \"Center\"");
				noError = MStatus::kFailure;
			}

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
			ctm.transpose().get(transform.matrix);
			prop.transform.push_back(transform);

			// Vertices & Materials
			if (connectedShaders.length() > 0) {
				prop.offsetIndices.resize(connectedShaders.length());
				unsigned int vertCount = 0;
				for (unsigned int i = 0; i < materialPerFace.length(); i++)
					for (unsigned int x = 0; x < (unsigned int)trianglesPerFace[i]; x++)
						for (unsigned int y = 0; y < 3; y++) {
							prop.offsetIndices[materialPerFace[i]].push_back(offsetIndices[vertCount]);
							vertCount++;
						}
			}

			for (unsigned int i = 0; i < connectedShaders.length(); i++) {
				MPlugArray connections;
				MFnDependencyNode(connectedShaders[i]).findPlug("surfaceShader").connectedTo(connections, true, false, &res);
				prop.materialIndices.push_back(this->materialList[MFnLambertShader(connections[0].node()).name().asChar()].materialId);
				prop.materialOffsets.push_back((unsigned int)prop.offsetIndices[i].size());
			}
			// Build vertices
			if (posIndices.length() == uvIndices.length() && posIndices.length() == normalIndices.length())		
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
			else {
				MGlobal::executeCommandOnIdle(MString("error \"Position-, uv- or normal-indices count do not match...\";"));
				MGlobal::executeCommand("confirmDialog - title \"Exporter\" - message \"Position-, uv- or normal-indices count do not match...       \" - button \"Ok\" - defaultButton \"Ok\" - ma \"Center\"");
				noError = MStatus::kFailure;
			}
			
			// AABB
			MBoundingBox aabb(mesh.boundingBox());
			aabb.transformUsing(ctm);

			// OBB
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

					for (unsigned int i = 0; i < 8; i++) {
						aabb.expand(bbPositions[i]);
					}
				}
				it.next();
			}

			// AABB continued
			ABBox meshBox;
			aabb.center().get(meshBox.abbPositions[0]);
			aabb.max().get(meshBox.abbPositions[1]);
			aabb.min().get(meshBox.abbPositions[2]);

			prop.abbExtensions.push_back(meshBox);

			this->propList[meshTransformName] = prop;
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
	MFnPointLight light(object);
	MFnDagNode node(light.parent(0));
	MDagPath path;
	node.getPath(path);
	MFnTransform lightTransform(path);
	
	// Room
	pointLightList[pLightCount].roomId = MFnTransform(lightTransform.parent(0)).findPlug("Object_Id", &res).asInt();

	// Color
	light.color().get(pointLightList[pLightCount].color);

	// Intensity
	pointLightList[pLightCount].intensity = light.intensity();

	// Position
	double position[4];
	lightTransform.getTranslation(MSpace::kWorld).get(position);
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
	MFnDagNode node(light.parent(0));
	MDagPath path;
	node.getPath(path);
	MFnTransform lightTransform(path);

	// Room
	spotLightList[sLightCount].roomId = MFnTransform(lightTransform.parent(0)).findPlug("Object_Id", &res).asInt();

	// Color
	light.color().get(spotLightList[sLightCount].color);

	// Intensity
	spotLightList[sLightCount].intensity = light.intensity();

	// Position
	double position[4];
	lightTransform.getTranslation(MSpace::kWorld).get(position);
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

void DataHandler::CreateCapturePoint(MObject object) {
	MFnMesh mesh(object);
	MFnTransform meshTransform(mesh.parent(0));

	CapturePoint cptPoint;
	cptPoint.roomID = MFnTransform(meshTransform.parent(0)).findPlug("Object_Id", &res).asInt();

	// Main AABB
	Transform transform;
	MFnMatrixData data(meshTransform.findPlug("parentMatrix").elementByLogicalIndex(0).asMObject());
	MMatrix ctm = meshTransform.transformationMatrix() * data.matrix(&res);
	ctm.transpose().get(transform.matrix);

	MBoundingBox aabb(mesh.boundingBox());
	aabb.transformUsing(ctm);

	// AABB + Walls
	MItDag it;
	it.reset(meshTransform.object(), MItDag::kBreadthFirst, MFn::kTransform);
	while (!it.isDone()) {
		if (it.item() != meshTransform.object()) {
			if (MFnTransform(it.item()).hasAttribute("Object_Type")) {
				unsigned int objectType = MFnTransform(it.item()).findPlug("Object_Type").asInt();
				if (objectType == OBJECT_TYPE_CAPTURE_WALL) {
					MFnMesh temp(MFnTransform(it.item()).child(0));
					MDagPath childPath;
					temp.getPath(childPath);
					
					MFnMesh wallMesh(childPath);
					MFnTransform wallTransform(wallMesh.parent(0));
					CapturePointWall wall;					

					MIntArray vertexCount, posIndices, uvPerPolygonCount, uvIndices, normalPerPolygonArray, normalIndices, materialPerFace, trianglesPerFace, offsetIndices;
					MFloatArray uList, vList;
					MFloatVectorArray tangents;
					MObjectArray connectedShaders;

					float* positions = (float*)wallMesh.getRawPoints(&res);
					float* normals = (float*)wallMesh.getRawNormals(&res);

					wallMesh.getVertices(vertexCount, posIndices);
					wallMesh.getUVs(uList, vList);
					wallMesh.getAssignedUVs(uvPerPolygonCount, uvIndices);
					wallMesh.getNormalIds(normalPerPolygonArray, normalIndices);
					wallMesh.getTriangleOffsets(trianglesPerFace, offsetIndices);
					wallMesh.getTangents(tangents, MSpace::kObject);

					cptPoint.indicesCounts.push_back(offsetIndices.length());
					cptPoint.vertexCounts.push_back(posIndices.length());

					// Transform
					Transform transform;
					MFnMatrixData wallData(wallTransform.findPlug("parentMatrix").elementByLogicalIndex(0).asMObject());
					MMatrix wallCtm = wallTransform.transformationMatrix() * wallData.matrix(&res);
					wallCtm.transpose().get(wall.transform.matrix);

					// Vertices & Materials
					for (unsigned int i = 0; i < offsetIndices.length(); i++) {
						wall.offsetIndices.push_back(offsetIndices[i]);
						cerr << "\nIndex: " << offsetIndices[i];
					}

					// Build vertices
					if (posIndices.length() == uvIndices.length() && posIndices.length() == normalIndices.length())
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

							wall.vertices.push_back(vertex);
						}
					else {
						MGlobal::executeCommandOnIdle(MString("error \"Position-, uv- or normal-indices count do not match...\";"));
						MGlobal::executeCommand("confirmDialog - title \"Exporter\" - message \"Position-, uv- or normal-indices count do not match...       \" - button \"Ok\" - defaultButton \"Ok\" - ma \"Center\"");
						noError = MStatus::kFailure;
					}

					cptPoint.walls.push_back(wall);
				}
			}
			else {
				MFnMesh temp(MFnTransform(it.item()).child(0));
				MDagPath childPath;
				temp.getPath(childPath);
				MFnMesh childMesh(childPath);
				MFnTransform childTransform(childMesh.parent(0));

				MFnMatrixData childData(childTransform.findPlug("parentMatrix").elementByLogicalIndex(0).asMObject());
				MBoundingBox childAABB(childMesh.boundingBox());
				childAABB.transformUsing(childTransform.transformationMatrix() * childData.matrix(&res));

				ABBox childBox;
				childAABB.center().get(childBox.abbPositions[0]);
				childAABB.max().get(childBox.abbPositions[1]);
				childAABB.min().get(childBox.abbPositions[2]);

				cptPoint.AABBs.push_back(childBox);

				aabb.expand(childAABB.max());
				aabb.expand(childAABB.min());
			}
		}
		it.next();
	}

	// Main AABB continued
	aabb.center().get(cptPoint.mainAABB.abbPositions[0]);
	aabb.max().get(cptPoint.mainAABB.abbPositions[1]);
	aabb.min().get(cptPoint.mainAABB.abbPositions[2]);

	capturePoints.push_back(cptPoint);
	capturePointHeader.AABBCounts.push_back(cptPoint.AABBs.size());
	capturePointHeader.WallCounts.push_back(cptPoint.walls.size());
}

void DataHandler::CreateSpawnPoint(MObject object, unsigned int team) {
	SpawnPoint spawn;
	MFnSpotLight light(object);
	MFnTransform lightTransform(light.parent(0));

	spawn.roomId = MFnTransform(lightTransform.parent(0)).findPlug("Object_Id").asInt();

	MFnMatrixData data(lightTransform.findPlug("parentMatrix").elementByLogicalIndex(0).asMObject());
	MMatrix ctm = lightTransform.transformationMatrix() * data.matrix(&res);
	ctm.transpose().get(spawn.transform);

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

void DataHandler::GatherMapData() {
	unsigned int highestRoomId = 0;

	MItDag dagIt;
	while (dagIt.isDone() != true) {
		if (dagIt.item().hasFn(MFn::kMesh) && noError) {
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
						CreateCapturePoint(dagIt.item());
					else if (objectType == OBJECT_TYPE_ROOM) {
						unsigned int objectId = MFnTransform(mesh.parent(0)).findPlug("Object_Id", &res).asInt();
						if (objectId != 0) {
							map<unsigned int, ABBox>::iterator roomIt = roomBoxes.find(objectId);
							if (roomIt != roomBoxes.end()) {
								MGlobal::executeCommandOnIdle(MString("error \"Room Object_Ids found in more than one object (Object: " + meshTransform.name() + ", ID: " + (MString() + objectId) + ")\";"));
								MGlobal::executeCommand("confirmDialog - title \"Exporter\" - message \"Room Object_Ids found in more than one object (Object: " + meshTransform.name() + ", ID: " + (MString() + objectId) + ")...       \" - button \"Ok\" - defaultButton \"Ok\" - ma \"Center\"");
								noError = MStatus::kFailure;
							}

							ABBox roomBox;
							meshTransform.boundingBox().center().get(roomBox.abbPositions[0]);
							meshTransform.boundingBox().max().get(roomBox.abbPositions[1]);
							meshTransform.boundingBox().min().get(roomBox.abbPositions[2]);
							roomBoxes[objectId] = roomBox;

							if (objectId > highestRoomId)
								highestRoomId = objectId;
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

	if (highestRoomId != roomCount-1) {
		MGlobal::executeCommandOnIdle(MString("error \"The number of rooms in the scene is lower than the highest Object_Id\";"));
		MGlobal::executeCommand("confirmDialog - title \"Exporter\" - message \"The number of rooms(" + (MString() + roomCount) + ") in the scene is lower than the highest Object_Id(" + (MString() + highestRoomId) + "). Room count should be ID + 1...       \" - button \"Ok\" - defaultButton \"Ok\" - ma \"Center\"");
		noError = MStatus::kFailure;
	}	
}

void DataHandler::GatherStaticData() {
	MDagPath meshPath;

	// get a list of the currently selected items 
	MSelectionList selected;
	MGlobal::getActiveSelectionList(selected);
	MDagPath path;
	res = selected.getDagPath(0, path);

	if (res) {
		MFnMesh mesh(path, &res);
		if (res) {
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
			CreateMaterial(connectedShaders, staticAsset.materialList, staticAsset.textureList);

			// Header
			staticAsset.header.materialCount = connectedShaders.length();
			staticAsset.header.textureCount = staticAsset.textureList.size();
			staticAsset.header.indexCount = offsetIndices.length();
			staticAsset.header.vertexCount = posIndices.length();

			// Materials & indices
			staticAsset.offsetIndices.resize(connectedShaders.length());
			unsigned int vertCount = 0;
			for (unsigned int i = 0; i < materialPerFace.length(); i++)
				for (unsigned int x = 0; x < (unsigned int)trianglesPerFace[i]; x++)
					for (unsigned int y = 0; y < 3; y++) {
						staticAsset.offsetIndices[materialPerFace[i]].push_back(offsetIndices[vertCount]);
						vertCount++;
					}

			for (unsigned int i = 0; i < connectedShaders.length(); i++)
				staticAsset.materialOffsets.push_back((unsigned int)staticAsset.offsetIndices[i].size());

			// Build vertices
			if (posIndices.length() == uvIndices.length() && posIndices.length() == normalIndices.length())
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
						tangents[normalIndices[i]].z,
					};

					staticAsset.vertices.push_back(vertex);
				}
			else {
				MGlobal::executeCommand(MString("error \"Position-, uv- or normal-indices count do not match...\";"));
				MGlobal::executeCommand("confirmDialog - title \"Exporter\" - message \"Position-, uv- or normal-indices count do not match...       \" - button \"Ok\" - defaultButton \"Ok\" - ma \"Center\"");
				noError = MStatus::kFailure;
			}
		}
		else {
			MGlobal::executeCommand(MString("error \"No mesh selected...\";"));
			MGlobal::executeCommand("confirmDialog - title \"Exporter\" - message \"No mesh selected...       \" - button \"Ok\" - defaultButton \"Ok\" - ma \"Center\"");
			noError = MStatus::kFailure;
		}
	}
}

void DataHandler::GatherCharacterData(bool exportCharacter, bool exportAnimations) {
	MDagPath meshPath;

	MItDag dagIt(MItDag::kBreadthFirst, MFn::kMesh);
	while (dagIt.isDone() != true) {
		dagIt.getPath(meshPath);

		MFnMesh mesh(meshPath);
		MFnTransform meshTransform(mesh.parent(0));
		if (!MFnMesh(meshPath).isIntermediateObject() && meshTransform.name().length() > 0) {

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
					skinCluster.influenceObjects(jointPaths);

					// Only retrieve joint data from character's skinCluster
					if (exportAnimations == true && animationList.size() == 0) {
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
									
									if (res) {
										if ((int)parentPlugs[0].logicalIndex() > (int)jointPaths.length()) {
											MGlobal::executeCommand(MString("error \"Parent/Member indices do not match. Make sure the bindPose plugs are valid...\";"));
											MGlobal::executeCommand("confirmDialog - title \"Exporter\" - message \"Parent/Member indices do not match. Make sure the bindPose plugs are valid...       \" - button \"Ok\" - defaultButton \"Ok\" - ma \"Center\"");
											noError = MStatus::kFailure;
										}
										else
											parentIndices.push_back((int)parentPlugs[0].logicalIndex());
									}
								}
							}
						}

						// Step through plugs to find used animationLayers
						if (noError) {
							MPlugArray layerPlugs;
							MFnDependencyNode(jointPaths[0].node()).findPlug("translateX").connectedTo(layerPlugs, false, true);

							for (unsigned int i = 0; i < layerPlugs.length(); i++) {
								if (layerPlugs[i].node().hasFn(MFn::kAnimLayer)) {
									MPlugArray blendPlugs;
									MFnDependencyNode(layerPlugs[i].node()).findPlug("foregroundWeight").connectedTo(blendPlugs, false, true);

									bool layerFound = false;
									for (unsigned int x = 0; x < blendPlugs.length(); x++) {
										if (blendPlugs[x].node().hasFn(MFn::kBlendNodeDoubleLinear)) {
											MPlugArray curvePlugs;
											MFnDependencyNode(blendPlugs[x].node()).findPlug("inputB").connectedTo(curvePlugs, true, false);

											// Set this layer to solo and calculate data for each keyframe, for each joint
											for (unsigned int y = 0; y < curvePlugs.length(); y++) {
												if (curvePlugs[y].node().hasFn(MFn::kAnimCurve)) {
													MString myCommand = "animLayer -e -solo 1 " + MFnDependencyNode(layerPlugs[i].node()).name() + ";";
													MGlobal::executeCommand(myCommand);

													MAnimControl animControl;
													MTime time;

													Animation animation;
													animation.jointCount = jointPaths.length();
													animation.keyCount = MFnAnimCurve(curvePlugs[y].node()).numKeys();

													for (unsigned int z = 0; z < MFnAnimCurve(curvePlugs[y].node()).numKeys(); z++) {
														animControl.setCurrentTime(MFnAnimCurve(curvePlugs[y].node()).time(z));

														vector<Transform> keyframeData;
														vector<MMatrix> relativePose;

														// Gather joint-data
														for (unsigned int n = 0; n < animation.jointCount; n++) {
															MFnIkJoint joint(jointPaths[n]);

															Transform transform;
															MMatrix final;

															if (n == 0)
																relativePose.push_back(joint.transformationMatrix());
															else
																relativePose.push_back(joint.transformationMatrix() * relativePose[parentIndices[n]]);

															final = jointBindPose[n].inverse() * relativePose[n];

															final.transpose().get(transform.matrix);
															keyframeData.push_back(transform);
														}

														animation.animationMatrices.push_back(keyframeData);
													}

													animationList[MFnDependencyNode(layerPlugs[i].node()).name().asChar()] = animation;

													// Use break since BNDL nodes with animCurves occur more than once
													layerFound = true;

													myCommand = "animLayer -e -solo 0 " + MFnDependencyNode(layerPlugs[i].node()).name() + ";";
													MGlobal::executeCommandOnIdle(myCommand);
												}
											}
										}
										if (layerFound == true)
											break;
									}
								}
							}
						}
					}

					// Get mesh data if current mesh has the prefix "mesh_"
					if (meshTransform.name().substringW(0, 4) == "mesh_" && exportCharacter == true && noError) {
						MString selectRoot = "select -r " + MFnIkJoint(jointPaths[0]).name() + "\;";
						MGlobal::executeCommandOnIdle(selectRoot);
						MGlobal::executeCommandOnIdle(MString("gotoBindPose\;"));

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
						CreateMaterial(connectedShaders, character.materialList, character.textureList);

						// Header
						character.header.materialCount = connectedShaders.length();
						character.header.textureCount = character.textureList.size();
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

							for (unsigned int i = 0; i < jointPaths.length(); i++) {
								MDoubleArray jointWeight;
								skinCluster.getWeights(meshPath, geomIter.currentItem(), skinCluster.indexForInfluenceObject(jointPaths[i]), jointWeight);

								weights[geomIter.index()].push_back({ (float)jointWeight[0], skinCluster.indexForInfluenceObject(jointPaths[i]) });
							}

							while (weights[geomIter.index()].size() < 4)
								weights[geomIter.index()].push_back({ 0.0f, 0 });

							sort(weights[geomIter.index()].begin(), weights[geomIter.index()].end());
							geomIter.next();
						}

						if (posIndices.length() == uvIndices.length() && posIndices.length() == normalIndices.length()) {
							// Build vertices (weights are sorted low to high by default, fetch weights starting from the back)
							unsigned int influenceCount = (unsigned int)weights[0].size();
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

									weights[posIndices[i]][influenceCount - 1].second, // Bone index
									weights[posIndices[i]][influenceCount - 2].second,
									weights[posIndices[i]][influenceCount - 3].second,
									weights[posIndices[i]][influenceCount - 4].second,

									weights[posIndices[i]][influenceCount - 1].first, // Weight
									weights[posIndices[i]][influenceCount - 2].first,
									weights[posIndices[i]][influenceCount - 3].first,
									weights[posIndices[i]][influenceCount - 4].first,
								};

								character.vertices.push_back(vertex);
							}
						}
						else {
							MGlobal::executeCommand(MString("error \"Position-, uv- or normal-indices count do not match...\";"));
							MGlobal::executeCommand("confirmDialog - title \"Exporter\" - message \"Position-, uv- or normal-indices count do not match...       \" - button \"Ok\" - defaultButton \"Ok\" - ma \"Center\"");
							noError = MStatus::kFailure;
						}
					}
					else if (meshTransform.name().substringW(0, 4) != "mesh_" && exportCharacter == true) {
						MGlobal::executeCommand(MString("error \"No character exported; make sure the character's name has the mesh_ prefix...\";"));
						MGlobal::executeCommand("confirmDialog - title \"Exporter\" - message \"No character exported; make sure the character's name has the mesh_ prefix...       \" - button \"Ok\" - defaultButton \"Ok\" - ma \"Center\"");
						noError = MStatus::kFailure;
					}
				}

				depIt.next();
			}
		}

		dagIt.next();
	}
}

void DataHandler::ExportMap(MString path) {
	ofstream file;
	MString mapPath = path + "/tron3k_map.bin";
	file.open(mapPath.asChar(), ios::out | ios::binary);

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

	for (map<string, Prop>::iterator propIt = this->propList.begin(); propIt != this->propList.end(); ++propIt) {
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

	// ### Material Data ###
	for (map<string, Material>::iterator it = materialList.begin(); it != materialList.end(); ++it) {
		file.write(reinterpret_cast<char*>(&it->second), sizeof(Material));
	}

	// ### Texture Header ###
	for (unsigned int x = 0; x < textureList.size(); x++) {
		unsigned int pathSize = (unsigned int)textureList[x].length();
		file.write(reinterpret_cast<char*>(&pathSize), sizeof(unsigned int));
	}

	// ### Texture Data ###
	for (unsigned int x = 0; x < textureList.size(); x++) {
		file.write(textureList[x].c_str(), sizeof(char) * textureList[x].length());
	}

	// ### PointLight Data ###
	for (map<unsigned int, Light>::iterator it = pointLightList.begin(); it != pointLightList.end(); ++it) {
		file.write(reinterpret_cast<char*>(&it->second), sizeof(Light));
	}

	// ### SpotLight Data ###
	for (map<unsigned int, Light>::iterator it = spotLightList.begin(); it != spotLightList.end(); ++it) {
		file.write(reinterpret_cast<char*>(&it->second), sizeof(Light));
	}

	// ### Portal Data ###
	for (map<unsigned int, Portal>::iterator it = portalList.begin(); it != portalList.end(); ++it) {
		file.write(reinterpret_cast<char*>(&it->second), sizeof(Portal));
	}

	// ### Capture Points Header ###
	if (capturePoints.size() > 0) {
		file.write(reinterpret_cast<char*>(capturePointHeader.AABBCounts.data()), sizeof(unsigned int) * capturePointHeader.AABBCounts.size());
		file.write(reinterpret_cast<char*>(capturePointHeader.WallCounts.data()), sizeof(unsigned int) * capturePointHeader.WallCounts.size());
	}

	// ### Capture Points Data ###
	for (unsigned int i = 0; i < capturePoints.size(); i++) {
		// CapturePoint
		file.write(reinterpret_cast<char*>(&capturePoints[i].roomID), sizeof(unsigned int));
		file.write(reinterpret_cast<char*>(&capturePoints[i].mainAABB), sizeof(ABBox));
		file.write(reinterpret_cast<char*>(capturePoints[i].AABBs.data()), sizeof(ABBox) * capturePointHeader.AABBCounts[i]);
		file.write(reinterpret_cast<char*>(capturePoints[i].indicesCounts.data()), sizeof(unsigned int) * capturePointHeader.WallCounts[i]);
		file.write(reinterpret_cast<char*>(capturePoints[i].vertexCounts.data()), sizeof(unsigned int) * capturePointHeader.WallCounts[i]);

		// Walls
		for (unsigned int x = 0; x < capturePointHeader.WallCounts[i]; x++) {
			file.write(reinterpret_cast<char*>(&capturePoints[i].walls[x].transform), sizeof(Transform));
			file.write(reinterpret_cast<char*>(capturePoints[i].walls[x].offsetIndices.data()), sizeof(unsigned int) * capturePoints[i].indicesCounts[x]);
			file.write(reinterpret_cast<char*>(capturePoints[i].walls[x].vertices.data()), sizeof(Vertex) * capturePoints[i].vertexCounts[x]);
		}
	}

	// ### SpawnPoints Team A ###
	if (spawnTeamA.size() > 0)
		file.write(reinterpret_cast<char*>(spawnTeamA.data()), sizeof(SpawnPoint) * spawnTeamA.size());

	// ### SpawnPoints Team B ###
	if (spawnTeamB.size() > 0)
		file.write(reinterpret_cast<char*>(spawnTeamB.data()), sizeof(SpawnPoint) * spawnTeamB.size());

	// ### SpawnPoints Team FFA ###
	if (spawnTeamFFA.size() > 0)
		file.write(reinterpret_cast<char*>(spawnTeamFFA.data()), sizeof(SpawnPoint) * spawnTeamFFA.size());

	// ### Room AABB ###
	for (map<unsigned int, ABBox>::iterator it = roomBoxes.begin(); it != roomBoxes.end(); ++it) {
		file.write(reinterpret_cast<char*>(&it->second), sizeof(ABBox));
	}
	
	file.close();
}

void DataHandler::ExportStatic(MString path) {
	ofstream static_file;
	static_file.open(path.asChar(), ios::out | ios::binary);

	// Header
	static_file.write(reinterpret_cast<char*>(&staticAsset.header), sizeof(StaticAssetHeader));

	// Material Indices/Offsets
	static_file.write(reinterpret_cast<char*>(staticAsset.materialOffsets.data()), sizeof(unsigned int) * staticAsset.header.materialCount);

	// Indices
	for (unsigned int i = 0; i < staticAsset.header.materialCount; i++)
		static_file.write(reinterpret_cast<char*>(staticAsset.offsetIndices[i].data()), sizeof(unsigned int) * staticAsset.materialOffsets[i]);

	// Vertices
	static_file.write(reinterpret_cast<char*>(staticAsset.vertices.data()), sizeof(Vertex) * staticAsset.header.vertexCount);

	// Materials
	for (map<string, Material>::iterator matIt = staticAsset.materialList.begin(); matIt != staticAsset.materialList.end(); ++matIt)
		static_file.write(reinterpret_cast<char*>(&matIt->second), sizeof(Material));

	// Texture Header
	for (unsigned int x = 0; x < staticAsset.textureList.size(); x++) {
		unsigned int pathSize = (unsigned int)staticAsset.textureList[x].length();
		static_file.write(reinterpret_cast<char*>(&pathSize), sizeof(unsigned int));
	}

	// Texture Data
	for (unsigned int x = 0; x < staticAsset.textureList.size(); x++) {
		static_file.write(staticAsset.textureList[x].c_str(), sizeof(char) * staticAsset.textureList[x].length());
	}

	static_file.close();
}

void DataHandler::ExportCharacter(MString path, unsigned int charType, unsigned int perspType) {
	// #### CHARACTER ####
	if (character.header.vertexCount > 0) {
		unsigned int lastSlash = path.rindexW("/");
		MString meshPath = path + "/mesh_" + characterType[charType] + "_" + perspectiveType[perspType] + ".bin";

		ofstream file;
		file.open(meshPath.asChar(), ios::out | ios::binary);
		
		
		// Header
		file.write(reinterpret_cast<char*>(&character.header), sizeof(AnimAssetHeader));

		// Material Indices/Offsets
		file.write(reinterpret_cast<char*>(character.materialOffsets.data()), sizeof(unsigned int) * character.header.materialCount);

		// Indices
		for (unsigned int i = 0; i < character.header.materialCount; i++)
			file.write(reinterpret_cast<char*>(character.offsetIndices[i].data()), sizeof(unsigned int) * character.materialOffsets[i]);

		// Vertices
		file.write(reinterpret_cast<char*>(character.vertices.data()), sizeof(AnimVertex) * character.header.vertexCount);

		// Materials
		for (map<string, Material>::iterator it = character.materialList.begin(); it != character.materialList.end(); ++it)
			file.write(reinterpret_cast<char*>(&it->second), sizeof(Material));

		// Texture Header
		for (unsigned int x = 0; x < character.textureList.size(); x++) {
			unsigned int pathSize = (unsigned int)character.textureList[x].length();
			file.write(reinterpret_cast<char*>(&pathSize), sizeof(unsigned int));
		}

		// Textire Data
		for (unsigned int x = 0; x < character.textureList.size(); x++) {
			file.write(character.textureList[x].c_str(), sizeof(char) * character.textureList[x].length());
		}

		file.close();
	}

	// #### ANIMATIONS ####
	for (map<string, Animation>::iterator it = animationList.begin(); it != animationList.end(); ++it) {
		MString animationPath = path + "/anim_" + characterType[charType] + "_" + perspectiveType[perspType] + "_" + it->first.c_str() + ".bin";

		ofstream anim_file;
		anim_file.open(animationPath.asChar(), ios::out | ios::binary);

		// JointCount
		anim_file.write(reinterpret_cast<char*>(&it->second.jointCount), sizeof(unsigned int));

		// KeyCount
		anim_file.write(reinterpret_cast<char*>(&it->second.keyCount), sizeof(unsigned int));

		// Matrices
		for (unsigned int i = 0; i < it->second.keyCount; i++)
			anim_file.write(reinterpret_cast<char*>(it->second.animationMatrices[i].data()), sizeof(Transform) * it->second.jointCount);

		anim_file.close();
	}
}
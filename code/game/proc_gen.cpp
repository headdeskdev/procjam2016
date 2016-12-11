struct HighwayPos {
	I32 angleInFifteens;
	Vector3 position;
	U32 count;
};

struct GridLevel {
	U16 data[256][256];
};

struct Grid {
	GridLevel level[6];

};

I32 convertToGridPosRound(F32 worldPos) {
	I32 gridPos = 0;
	gridPos = (I32) ((worldPos / 2.0f) + 127.5f);
	if (gridPos < 0 || gridPos >= 256) {		
		return -1;	
	}
	return gridPos;	
}

I32 convertToGridPos(F32 worldPos, bool roundDown) {
	I32 gridPos = 0;
	if (roundDown) {
		gridPos = (I32) ((worldPos / 2.0f) + 127.0f);
	} else {
		gridPos = (I32) ((worldPos / 2.0f) + 128.0f);
	}
	if (gridPos < 0 || gridPos >= 256) {		
		return -1;	
	}
	return gridPos;	
}

F32 convertToWorldPos(I32 gridPos) {
	return (gridPos - 127.0f) * 2.0f;
}

void setGrid(GridLevel* level, U8 x, U8 y, U8 value) {
	level->data[x][y] = value;
}

void levelFillTriangle(GridLevel* level, Vector2 vt1, Vector2 vt2, Vector2 vt3, U16 value) {
	F32 minX = mmin(vt1.x, vt2.x);
	minX = mmin(minX, vt3.x);
	F32 minY = mmin(vt1.y, vt2.y);
	minY = mmin(minY, vt3.y);
	F32 maxX = mmax(vt1.x, vt2.x);
	maxX = mmax(maxX, vt3.x);
	F32 maxY = mmax(vt1.y, vt2.y);
	maxY = mmax(maxY, vt3.y);

	I32 gridmaxX = convertToGridPos(maxX, false);
	I32 gridmaxY = convertToGridPos(maxY, false);
	I32 gridminX = convertToGridPos(minX, true);
	I32 gridminY = convertToGridPos(minY, true);

	gridmaxX = mmin(gridmaxX, 255);
	gridmaxY = mmin(gridmaxY, 255);
	gridminX = mmax(gridminX, 0);
	gridminY = mmax(gridminY, 0);

	for (int x = gridminX; x <= gridmaxX; x++) {
   		for (int y = gridminY; y <= gridmaxY; y++) {   			
   			Vector2 gridPos = {convertToWorldPos(x),convertToWorldPos(y)};
   			// TODO: check collision with grid square
   			setGrid(level,x,y,value);
   		}
   	}
}

void levelFillQuad(GridLevel* level, Quad quad, U16 value) {
	levelFillTriangle(level, quad.bottomleft, quad.topright, quad.bottomright, value);
	levelFillTriangle(level, quad.bottomleft, quad.topright, quad.topleft, value);
}

HighwayPos createHighwaySegment(ProcObjectList* procObjs, HighwayPos pos, I32 turnDirection, F32 scale, GridLevel* level, U16 value) {
	F32 angle = pos.angleInFifteens * PI * (15.0f/180.0f);
	if (turnDirection == 0) {
		
		Quat quat = math_getAngleAxisQuat(angle,{0.0f,-1.0f,0.0f});
		Vector3 move = {cosf(angle),0.0f,sinf(angle)};
		pos.position = pos.position + move*scale*10.0f;
		procObjs->addObject({scale,scale,scale},pos.position,quat,2);
	} else {
		Vector3 move = {cosf(angle),0.0f,sinf(angle)};
		Vector3 moveTangent = {sinf(angle),0.0f,-cosf(angle)};
		pos.position = pos.position + move*scale*10.0f;

		if (turnDirection > 0) {
			angle = angle + PI;
		} else {
			angle = angle-PI * 15.0f/180.0f;
			pos.position = pos.position + move*scale*1.58f + moveTangent*scale*1.35f;
		}	
		Quat quat = math_getAngleAxisQuat(angle,{0.0f,-1.0f,0.0f});	
		procObjs->addObject({scale,scale,scale},pos.position,quat,3);		
		pos.angleInFifteens += turnDirection;

		if (turnDirection > 0) {
			F32 angle = pos.angleInFifteens * PI * 15.0f/180.0f;
			move = {cosf(angle),0.0f,sinf(angle)};
			moveTangent = {sinf(angle),0.0f,-cosf(angle)};
			pos.position = pos.position + move*scale*1.58f + moveTangent*scale*1.35f;
		} 			
	}	

	Quad quad = math_getAxisAlignedQuad({{-5.0f,-10.0f},{5.0f,10.0f}});
	ProcObject procObj = procObjs->objects[procObjs->objectsCount-1];

	if (pos.count % 4 == 0) {		
		for (int i = 0; i < 2; i++) {
			Vector3 yOffset = { 0.0, scale * 4.0f, 4.0 };
			if (i % 2 == 0) {
				yOffset.z = -yOffset.z;
			}
			yOffset = procObj.rotation.transformVector(yOffset);
			Vector3 lightPosition = procObj.position + yOffset;
			Vector3 color = { 0.7f,0.9f,0.9f };
			procObjs->addLight(color, lightPosition, scale * 18.0f, 1.3f);
		}
	}

	TransformMatrix2d matrix2d = math_get2dRigidTransformMatrix({procObj.position.x,procObj.position.z},angle,scale);
	quad = matrix2d.transformQuad(quad);
	
	if (turnDirection == 0) {
		// Vector2 tpA = {0.0f, 5.0f};
		// Vector2 tpB = {0.0f, -5.0f};
		// tpA = matrix2d.transformPoint(tpA);
		// tpB = matrix2d.transformPoint(tpB);
		// Quat quatA = getAngleAxisQuat(angle+PI,{0.0f,-1.0f,0.0f});
		// Quat quatB = getAngleAxisQuat(angle,{0.0f,-1.0f,0.0f});
		// for (int i = 0; i < 2; i++) {
		// 	Vector3 towerPosA = {tpA.x, procObj.position.y-scale*(10.0f + 20.0f*i), tpA.y};	
		// 	Vector3 towerPosB = {tpB.x, procObj.position.y-scale*(10.0f + 20.0f*i), tpB.y};
		// 	procObjs->addObject({scale,scale,scale},towerPosA,quatA,1);
		// 	procObjs->addObject({scale,scale,scale},towerPosB,quatB,1);
		// }

	}

	// TODO: more accurate?
	levelFillQuad(level, quad, value);
	pos.count += 1;
	return pos;
}

struct AccessPoint {
	U16 building;
	U16 level;
};

struct BuildingConnection {		
	U16 buildingA;
	U16 buildingB;
	U8 levelA;
	U8 levelB;
	bool enabled;	

	AccessPoint getOtherAccessPoint(AccessPoint other) {
		if (other.building == buildingA && other.level == levelA) {
			return {buildingB, levelB};
		} else {
			return {buildingA, levelA};
		}
	}
};

struct BuildingConnectionList {
	U16 connectionNum;
	BuildingConnectionList* next;
};

enum SpecialBuildingType {
	NORMAL,
	ROOF_ONLY,	
	NO_CONNECTION,
	EMPTY	
};

struct Building {
	F32 height;
	F32 maxHeight;
	Rect rect;
	U8 numLevels;	
	BuildingConnectionList* levelConnections[3];
	SpecialBuildingType special;

	F32 groundGapDistance;
	F32 ceilingGapDistance;
	//PortalPoint* portalPoints;
	//ClimbPoint* climbPoints;

	// TODO: accessways

	void addConnectionElement(BuildingConnectionList* newConnection, U8 level) {
		BuildingConnectionList* first = levelConnections[level];
		levelConnections[level] = newConnection;
		levelConnections[level]->next = first;
	}
};

struct GeneratePosition {
	I32 gridPosX;
	I32 gridPosY;
};



struct CityGraph {
	U16 numBuildings = 0;
	U16 numConnections = 0;
	U16 numConnectionListElements = 0;
	U16 numAccessPoints = 0;
	Building* buildings;
	BuildingConnection*  connections;
	BuildingConnectionList*  connectionListElements;
	AccessPoint* accessPoints;

	bool addAccessPoint(U16 building, U16 level) {
		// TODO: use hash / or other faster structure
		
		for (int i = 0; i < numAccessPoints; i++) {
			if (accessPoints[i].building == building && accessPoints[i].level == level) {
				return false;
			}
		}
		accessPoints[numAccessPoints++] = {building, level};			
		return true;
	}

	void addConnection(BuildingConnection newConnection) {
		connections[numConnections] = newConnection;
		BuildingConnection* connection = connections + numConnections;
		U16 connectionNum = numConnections;
		numConnections++;

		BuildingConnectionList* a = connectionListElements + numConnectionListElements;
		a->next = 0;
		a->connectionNum = connectionNum;
		BuildingConnectionList* b = connectionListElements + numConnectionListElements + 1;
		b->next = 0;
		b->connectionNum = connectionNum;
		numConnectionListElements += 2;

		buildings[newConnection.buildingA].addConnectionElement(a, newConnection.levelA);
		buildings[newConnection.buildingB].addConnectionElement(b, newConnection.levelB);
	}
};

void connectBuildings(CityGraph* cityGraph, U16 buildingANumber, U16 buildingBNumber) {
	Building* buildingA = cityGraph->buildings + buildingANumber;
	Building* buildingB = cityGraph->buildings + buildingBNumber;
	if (buildingA->special == ROOF_ONLY && buildingB->special == ROOF_ONLY) {
		BuildingConnection connection = {buildingANumber, buildingBNumber,  0, 0, false};
		cityGraph->addConnection(connection);
	} else if (buildingA->special == ROOF_ONLY) {
		if (buildingB->numLevels > 1) {
			BuildingConnection connection = {buildingANumber, buildingBNumber,  0, U8(buildingB->numLevels-1), false};
			cityGraph->addConnection(connection);
		}
	} else if (buildingB->special == ROOF_ONLY) {
		if (buildingA->numLevels > 1) {
			BuildingConnection connection = {buildingBNumber, buildingANumber,  0, U8(buildingA->numLevels-1), false};
			cityGraph->addConnection(connection);
		}
	} else {
		BuildingConnection connection = {buildingBNumber, buildingANumber,  0, 0, false};
		cityGraph->addConnection(connection);		
		if (buildingA->numLevels == buildingB->numLevels) {
			for (U8 c = 1; c < buildingA->numLevels; c++) {
				BuildingConnection connection = {buildingBNumber, buildingANumber,  c, c, false};
				cityGraph->addConnection(connection);		
			}
		} else if (buildingA->numLevels == 2 && buildingB->numLevels == 3) {
			if (buildingB->height / 2.0f + 12.0f > buildingA->height) {
				BuildingConnection connection = {buildingBNumber, buildingANumber,  1, 1, false};
				cityGraph->addConnection(connection);						
			} else {
				BuildingConnection connection = {buildingBNumber, buildingANumber,  2, 1, false};
				cityGraph->addConnection(connection);
			}
		} else if (buildingA->numLevels == 3 && buildingB->numLevels == 2) {
			if (buildingA->height / 2.0f + 12.0f > buildingB->height) {
				BuildingConnection connection = {buildingBNumber, buildingANumber,  1, 1, false};
				cityGraph->addConnection(connection);						
			} else {
				BuildingConnection connection = {buildingBNumber, buildingANumber,  1, 2, false};
				cityGraph->addConnection(connection);
			}
		}
		
	}

}

CityGraph createCityGraph() {
	CityGraph graph;
	graph.buildings = (Building*) malloc(sizeof(Building)*500);
 	graph.connections = (BuildingConnection*) malloc(sizeof(BuildingConnection)*10000);
 	graph.connectionListElements = (BuildingConnectionList*) malloc(sizeof(BuildingConnectionList)*20000);
 	graph.accessPoints = (AccessPoint*) malloc(sizeof(AccessPoint)*2000);
 	return graph;
}
void freeCityGraph(CityGraph graph) {
	free(graph.buildings);
	free(graph.connections);
	free(graph.connectionListElements);
}

static U32 disabled = 0;
static U32 enabled = 0;

// NOTE: graph generation not actually used yet
void generateScene(U32 seed, ProcObjectList* procObjs) {

	enabled = 0;
	disabled = 0;

	Grid* grid = (Grid*) malloc(sizeof(Grid));
	memset(grid,0,sizeof(Grid));
	
    RandomGenerator r = {seed};    
    Quat rotation = math_getAngleAxisQuat(0,{0.0f,0.0f,1.0f});
    F32 scale = 26.0f;
    procObjs->addObject({scale,scale/8.0f,scale},{-130.0f,-52.0f,0.0f},rotation,2);
    procObjs->addObject({scale,scale/8.0f,scale},{130.0f,-52.0f,0.0f},rotation,2);

   
    for (int j = 0; j < 5; j++) {
    	F32 scale = 0.5f + 0.15f*r.frand();
    	F32 angle = ((j * 5) % 8) * PI * (45.0f/180.0f);	
    	F32 offset = r.frand()*70.0f - 35.0f;	
    	F32 vertical = -25.0f;
    	Vector3 move = {-cosf(angle)*250.0f+sinf(angle)*offset,j * 10.0f + vertical,-sinf(angle)*250.0f-cosf(angle)*offset};
    	HighwayPos highwayPos = {3*((j * 5) % 8), move, 0};
	    for (int i = 0; i < 90; i++) {
	    	U16 value = 1;
	    	if (i == 0 || i == 89) { value = 2;}
	    	if (r.frand() > 0.9) {
	    		highwayPos = createHighwaySegment(procObjs, highwayPos, r.getInteger(2)*2 - 1, scale, &grid->level[j+1], value);	    		 
	    	} else {
	    		highwayPos = createHighwaySegment(procObjs, highwayPos, 0, scale, &grid->level[j+1], value);	    		
	    	}    	
	    }
	}

	CityGraph cityGraph = createCityGraph();
	
	U32 numPositions = 1;
	GeneratePosition* positions = (GeneratePosition*) malloc(sizeof(GeneratePosition)*200);
	positions[0] = {0,0};

	for (int it = 0; it < 500 && numPositions > 0; it++) {
		I32 selected = r.getInteger(numPositions);
		GeneratePosition pos = positions[selected];
		I32 maxWidth = 0;
		I32 maxHeight = 0;
		for (int i = pos.gridPosX; i < 256; i++) {			
			if (grid->level[0].data[pos.gridPosX + maxWidth][pos.gridPosY] != 0) {
				break;
			}
			maxWidth++;
		}
		for (int i = pos.gridPosY; i < 256; i++) {
			if (grid->level[0].data[pos.gridPosX][pos.gridPosY + maxHeight] != 0) {
				break;
			}
			maxHeight++;
		}
		GeneratePosition a = {pos.gridPosX,pos.gridPosY};
		GeneratePosition b = a;


		if (maxWidth > 3 && maxHeight > 3) {			
			Building newBuilding = {};
			if (r.getInteger(4) == 0) {
				if (r.frand() < 0.5) {
					maxWidth = mmin(maxWidth,60);
					maxHeight = mmin(maxHeight,12);
					I32 minWidth = mmin(maxWidth,20);
					I32 minHeight = mmin(maxHeight,6);
					maxWidth = r.getInteger(maxWidth - minWidth + 1) + minWidth;
					maxHeight = r.getInteger(maxHeight - minHeight + 1) + minHeight;
				} else {
					maxWidth = mmin(maxWidth,12);
					maxHeight = mmin(maxHeight,60);
					I32 minWidth = mmin(maxWidth,6);
					I32 minHeight = mmin(maxHeight,20);
					maxWidth = r.getInteger(maxWidth - minWidth + 1) + minWidth;
					maxHeight = r.getInteger(maxHeight - minHeight + 1) + minHeight;
				}

				newBuilding.height = 0.0f;
				newBuilding.rect = {convertToWorldPos(pos.gridPosX), convertToWorldPos(pos.gridPosY), convertToWorldPos(pos.gridPosX) + maxWidth*2.0f, convertToWorldPos(pos.gridPosY) + maxHeight*2.0f};
				newBuilding.special = EMPTY;

			} else {
				maxWidth = mmin(maxWidth,40);
				maxHeight = mmin(maxHeight,40);
				I32 minWidth = mmin(maxWidth,10);
				I32 minHeight = mmin(maxHeight,10);
				maxWidth = r.getInteger(maxWidth - minWidth + 1) + minWidth;
				maxHeight = r.getInteger(maxHeight - minHeight + 1) + minHeight;
				I32 minLevel = 100;
				I32 endLevel = -1;
				for (int x = 0; x < maxWidth; x++) {
					for (int y = 0; y < maxHeight; y++) {
						for (int l = 1; l < 6; l++) {
							if (grid->level[l].data[pos.gridPosX+x][pos.gridPosY+y] != 0) {
								minLevel = mmin(minLevel, l);
							}
							if (grid->level[l].data[pos.gridPosX+x][pos.gridPosY+y] == 2) {
								endLevel = mmax(endLevel, l);
							}
						}
					}
				}
				bool minMax = false;
				if (endLevel >= 0 && endLevel <= minLevel) {
					minMax = true;
				}
				F32 maxTall = minLevel * 10.0f + 12.0f;
				F32 maxYScale = maxTall/3.0f;
				F32 minYScale = mmin(maxYScale, 3.5f);
				if (minMax) {
					minYScale = maxYScale + 0.7f;
					maxYScale = minYScale + 2.6f;
				} else {
					if (r.frand() < 0.5 && maxYScale > 20.0f) {
						minYScale = 20.0f;
						maxYScale = mmin(35.0f, maxYScale);												
					} else {
						maxYScale = mmin(22.0f, maxYScale);			
					}
				}					
				

				F32 yScale = r.frand()*(maxYScale-minYScale)+minYScale;
				yScale = mmin(maxYScale, yScale);
				newBuilding.height = yScale*3.0f;
				newBuilding.rect = {convertToWorldPos(pos.gridPosX), convertToWorldPos(pos.gridPosY), convertToWorldPos(pos.gridPosX) + maxWidth*2.0f, convertToWorldPos(pos.gridPosY) + maxHeight*2.0f};
				newBuilding.maxHeight = maxTall;
				newBuilding.special = NORMAL;
			}

			if (newBuilding.special == EMPTY) {
				newBuilding.numLevels = 1;
			} else {
				if ((newBuilding.height < 12.0f && r.getInteger(2) == 0) || r.getInteger(5) == 0) {
					newBuilding.special = ROOF_ONLY;
					newBuilding.numLevels = 1;
				} else if (maxWidth < 15 || maxHeight < 15) {
					newBuilding.special = NO_CONNECTION;
					newBuilding.numLevels = 2;
				} else if (newBuilding.height < 55.0f) {
					newBuilding.numLevels = 2;
					if (r.getInteger(5) == 0) {
						newBuilding.special = NO_CONNECTION;
					}
				} else {
					if (r.getInteger(6) == 0) {
						newBuilding.numLevels = 2;
						newBuilding.special = NO_CONNECTION;	
					} else {
						newBuilding.numLevels = 3;
					}
					
				}				
			}



			cityGraph.buildings[cityGraph.numBuildings++] = newBuilding;
			U16 buildingNumber = (cityGraph.numBuildings-1);
			Building* building = cityGraph.buildings + buildingNumber;
			for (int i = 0; i < building->numLevels; i++) {
				building->levelConnections[i] = 0;
				if (i != 0 && newBuilding.special != NO_CONNECTION) {
					BuildingConnection connection = {buildingNumber, buildingNumber, (U8)i, (U8)(i-1), false};
					cityGraph.addConnection(connection);
				}
			}
			I32 connectionValues[20];
			I32 connectionValueCount = 0;
			I32 iterations[16] = {-1,0,0,maxHeight,maxWidth+1,maxWidth+1,0,maxHeight,0,maxWidth,-1,0,0,maxWidth,maxHeight+1,maxHeight+1};
			for (int i = 0; i < 4; i++) {
				for (int x = iterations[i*4]; x < iterations[i*4+1]; x++) {
					for (int y = iterations[i*4+2]; y < iterations[i*4+3]; y++) {
						I32 gridX = x+pos.gridPosX;
						I32 gridY = y+pos.gridPosY;
						if (gridX >= 0 && gridY >= 0 && gridX < 256 && gridY < 256) {
							I32 connectionValue = grid->level[0].data[gridX][gridY];
							bool add = true;
							for (int c = 0; c < connectionValueCount; c++) {
								if (connectionValues[c] == connectionValue) {
									add = false;
								}
							}
							if (connectionValueCount < 20 && add && connectionValue != 0) {
								connectionValues[connectionValueCount++] = connectionValue;
							}
						}						
					}
				}
			}
			for (int i = 0; i < connectionValueCount; i++) {	
				U16 conBuildingNumber = connectionValues[i] - 1;			
				connectBuildings(&cityGraph, buildingNumber, conBuildingNumber);				
			}
			
			U16 value = cityGraph.numBuildings;

			for (int x = 0; x < maxWidth; x++) {
				for (int y = 0; y < maxHeight; y++) {
					grid->level[0].data[pos.gridPosX+x][pos.gridPosY+y] = value;
				}
			}

			a.gridPosX = a.gridPosX + maxWidth;
			b.gridPosY = b.gridPosY + maxHeight;
			positions[numPositions++] = a;		
			positions[numPositions++] = b;				
		}	
		assert(numPositions < 200);
		positions[selected] = positions[--numPositions];
	}



	for (int it = 0; it < 3 && enabled <= disabled; it++) {
		enabled = 0;
		disabled = 0;
		I32 startBuilding = r.getInteger(cityGraph.numBuildings);
		U16 currentGraphTraversalAccessPoint = 0;
		cityGraph.numAccessPoints = 0;
		cityGraph.addAccessPoint(startBuilding, 0);
		while(currentGraphTraversalAccessPoint < cityGraph.numAccessPoints) {
			U16 swap = currentGraphTraversalAccessPoint + r.getInteger(cityGraph.numAccessPoints - currentGraphTraversalAccessPoint);
			AccessPoint current = cityGraph.accessPoints[swap];
			cityGraph.accessPoints[swap] = cityGraph.accessPoints[currentGraphTraversalAccessPoint];
			cityGraph.accessPoints[currentGraphTraversalAccessPoint] = current;

			Building* building = cityGraph.buildings + current.building;
			BuildingConnectionList* firstConnection = building->levelConnections[current.level];
			while(firstConnection) {
				BuildingConnection* connection = cityGraph.connections + firstConnection->connectionNum;
				AccessPoint other = connection->getOtherAccessPoint(current);
				if (cityGraph.addAccessPoint(other.building, other.level)) {
					connection->enabled = true;
				}
				firstConnection = firstConnection->next;
			}

			currentGraphTraversalAccessPoint++;
		}



		for (int i = 0; i < cityGraph.numConnections; i++) {
			BuildingConnection* connection = cityGraph.connections + i;
			// Building* buildingA = cityGraph.buildings + connection->buildingA;
			// Building* buildingB = cityGraph.buildings + connection->buildingB;
			// F32 heightA = connection->levelA*(buildingA->height/buildingA->numLevels);
			// if (buildingA->special == ROOF_ONLY) { heightA = buildingA->height; }		
			// F32 heightB = connection->levelB*(buildingB->height/buildingB->numLevels);
			// if (buildingB->special == ROOF_ONLY) { heightB = buildingB->height; }
			// Vector3 positionA = {(buildingA->rect.min.x + buildingA->rect.max.x)/2.0f, heightA - 42.0f,(buildingA->rect.min.y + buildingA->rect.max.y)/2.0f};
			// Vector3 positionB = {(buildingB->rect.min.x + buildingB->rect.max.x)/2.0f, heightB - 42.0f,(buildingB->rect.min.y + buildingB->rect.max.y)/2.0f};
			// procObjs->addObject({scale,scale,scale},middlePosition,rotation,1);
			if (connection->enabled) {		    	
		    	enabled++;
		    } else {
		    	disabled++;
		    }

		}
	}
	for (int i = 0; i < cityGraph.numBuildings; i++) {
		
		Building* building = cityGraph.buildings + i;
		if (building->special != EMPTY) {
			Vector2 size = {building->rect.max.x - building->rect.min.x, building->rect.max.y - building->rect.min.y};
			Vector3 scale = {size.x/2.0f,building->height/3.0f,size.y/2.0f};
			Quat rotation = math_getAngleAxisQuat(PI*r.getInteger(2),{0.0f,1.0f,0.0f});
			Vector3 position = {building->rect.min.x + scale.x, building->height/2.0f-49.0f, building->rect.min.y + scale.z};
			procObjs->addObject(scale,position,rotation,0);
		}
	}

	free(grid);
	freeCityGraph(cityGraph);
	free(positions);
	
}

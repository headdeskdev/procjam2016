enum collision_ObjectType {
	COLLISION_OBJECT_CUBOID,
	COLLISION_OBJECT_CAPSULE,
	COLLISION_OBJECT_QUADPRISM
};

struct collision_Cuboid {
	Vector3 halfSizeAxes[3];
};

struct collision_Capsule {
	F32 middleHeight;
	F32 capRadius;
	Vector3 up;
};

struct collision_QuadPrism {
	Vector3 up;
	Vector3 x;
	Vector3 y;
	Vector2 points[4];
	F32 middleHeight;	
};


struct collision_Object {
	Vector3 position;
	collision_ObjectType type;	
	union {
		collision_Cuboid cuboid;
		collision_Capsule capsule;
		collision_QuadPrism quadPrism;
	};
};

Vector3 collision_getSupport(collision_Object object, Vector3 v) {
	switch (object.type) {
		case COLLISION_OBJECT_CUBOID: {
			collision_Cuboid cuboid = object.cuboid;
			Vector3 supportPoint = object.position;
			// Check which half of the 
			for (int i = 0; i < 3; i++) {
				if (cuboid.halfSizeAxes[i].dot(v) > 0) {
					supportPoint = supportPoint + cuboid.halfSizeAxes[i];
				} else {
					supportPoint = supportPoint - cuboid.halfSizeAxes[i];
				}
			}
			return supportPoint;			
		} break;
		case COLLISION_OBJECT_QUADPRISM: {
			collision_QuadPrism prism = object.quadPrism;
			Vector3 supportPoint = object.position;

			if (prism.up.dot(v) > 0) {
				supportPoint = supportPoint + (prism.up * (prism.middleHeight/2.0f));
			} else {
				supportPoint = supportPoint - (prism.up * (prism.middleHeight/2.0f));
			}

			Vector2 vec2d = {v.dot(prism.x), v.dot(prism.y)};
			vec2d = vec2d.normalize();

			F32 maxDistance = prism.points[0].dot(vec2d);
			U32 maxPoint = 0;
			for (int i = 0; i < 4; i++) {
				if (prism.points[i].dot(vec2d) > maxDistance) {
					maxPoint = i;
					maxDistance = prism.points[i].dot(vec2d);
				}
			}

			supportPoint = supportPoint + prism.x * prism.points[maxPoint].x + prism.y * prism.points[maxPoint].y;
			return supportPoint;		
		} break;
		case COLLISION_OBJECT_CAPSULE: {
			collision_Capsule capsule = object.capsule;
			
			Vector3 supportPoint = v.normalize();
			supportPoint = supportPoint * capsule.capRadius;
			supportPoint = supportPoint + object.position;
			if (capsule.up.dot(v) > 0) {
				supportPoint = supportPoint + (capsule.up * (capsule.middleHeight/2.0f));
			} else {
				supportPoint = supportPoint + (capsule.up * (capsule.middleHeight/2.0f));
			}
			return supportPoint;			
		} break;
	}
	return {0.0f};
}

struct collision_SupportPoint {
	Vector3 diff;
	Vector3 supportA;
	Vector3 supportB;
};

collision_SupportPoint collision_getMinkowskiSupport(collision_Object objectA, collision_Object objectB, Vector3 v) {
	collision_SupportPoint result;
	result.supportA = collision_getSupport(objectA, v);
	result.supportB = collision_getSupport(objectB, -v);
	result.diff = result.supportA - result.supportB;

	return result;
}

struct collision_Simplex {
	collision_SupportPoint points[4];
	U32 pointsCount;
	void addPoint(collision_SupportPoint p) {
		points[pointsCount++] = p;
	}
	void translate(Vector3 v) {
		for (int i = 0; i < pointsCount; i++) {
			points[i].diff = points[i].diff + v;
		}
	}
};


struct collision_SimplexResult {
	collision_Simplex simplex;
	Vector3 v;
};

#define TEST_SIMPLEX(vector,point) (vector).dot(point) > 0

// TODO: optimise this (no recursion, minimal computation, caching?)
// Can not use simplified update simplex function (must consider all possible locations)
collision_SimplexResult collision_updateSimplex(collision_Simplex simplex) {
	collision_SimplexResult result = {0};
	if (simplex.pointsCount <= 1) {		
		// Single point is always closest point
		result.simplex = simplex;
		result.v = -simplex.points[0].diff;

	} else if (simplex.pointsCount == 2) {
		
		collision_SupportPoint a = simplex.points[0];
		collision_SupportPoint b = simplex.points[1];
		if (TEST_SIMPLEX(b.diff - a.diff,a.diff)) {
			result.simplex = {{a}, 1};
			result.v = -a.diff;
		} else if (TEST_SIMPLEX(a.diff - b.diff,b.diff)) {
			result.simplex = {{b}, 1};
			result.v = -b.diff;
		} else {
			// Else must be in the line region
			result.simplex = simplex;
			Vector3 dif = a.diff - b.diff;
			F32 proj = dif.dot(a.diff);
			result.v = (dif * (proj/dif.lengthSquared())) - a.diff;
		}

	} else if (simplex.pointsCount == 3) {		
		collision_SupportPoint a = simplex.points[0];
		collision_SupportPoint b = simplex.points[1];		
		collision_SupportPoint c = simplex.points[2];

		Vector3 atb = b.diff - a.diff;
		Vector3 btc = c.diff - b.diff;
		Vector3 cta = a.diff - c.diff;
		Vector3 atc = c.diff - a.diff;		

		Vector3 up = atc.cross(atb);
		Vector3 x = up.cross(atb);
		Vector3 y = up.cross(btc);
		Vector3 z = up.cross(cta);

		if (TEST_SIMPLEX(x, -a.diff)) {
			collision_Simplex newSimplex = {{a,b},2};
			result = collision_updateSimplex(newSimplex);
		} else if (TEST_SIMPLEX(y, -b.diff)) {
			collision_Simplex newSimplex = {{b,c},2};
			result = collision_updateSimplex(newSimplex);
		} else if (TEST_SIMPLEX(z, -c.diff)) {
			collision_Simplex newSimplex = {{c,a},2};
			result = collision_updateSimplex(newSimplex);
		} else {
			// Else must be in the line region
			result.simplex = simplex;
			F32 proj = up.dot(-a.diff);
			result.v = up * (proj/up.lengthSquared());
		}

	} else {
		collision_SupportPoint a = simplex.points[0];
		collision_SupportPoint b = simplex.points[1]; // looking from the top		
		collision_SupportPoint c = simplex.points[2]; // a,b,c is base in clockwise
		collision_SupportPoint d = simplex.points[3]; // d is top of tetrahedron (pointing up)

		Vector3 atb = b.diff - a.diff;
		Vector3 atc = c.diff - a.diff;
		Vector3 atd = d.diff - a.diff;

		Vector3 ctb = b.diff - c.diff;
		Vector3 ctd = d.diff - c.diff;
		Vector3 bta = a.diff - b.diff;

		Vector3 fcba = atb.cross(atc);
		if (fcba.dot(atd) > 0) { fcba = -fcba; }
		Vector3 fdab = atd.cross(atb);		
		if (fdab.dot(atc) > 0) { fdab = -fdab; }
		Vector3 fadc = atc.cross(atd);
		if (fadc.dot(atb) > 0) { fadc = -fadc; }
		Vector3 fbcd = ctb.cross(ctd);
		if (fbcd.dot(bta) > 0) { fbcd = -fbcd; }

		if (TEST_SIMPLEX(fcba, -a.diff)) {
			collision_Simplex newSimplex = {{c,b,a},3};
			result = collision_updateSimplex(newSimplex);
		} else if (TEST_SIMPLEX(fdab, -b.diff)) {
			collision_Simplex newSimplex = {{d,a,b},3};
			result = collision_updateSimplex(newSimplex);
		} else if(TEST_SIMPLEX(fadc, -c.diff)) {
			collision_Simplex newSimplex = {{a,d,c},3};
			result = collision_updateSimplex(newSimplex);
		} else if (TEST_SIMPLEX(fbcd, -d.diff)) {
			collision_Simplex newSimplex = {{b,c,d},3};
			result = collision_updateSimplex(newSimplex);
		} else {
			collision_Simplex newSimplex = { { c,b,a },3 };
			result = collision_updateSimplex(newSimplex); // get vector in case of a flat tetrahedron lying on the origin
			result.simplex = simplex;
		}		
	}
	return result;
}

#define MAX_ITERATIONS 128
#define MAX_VERTICES 64

struct collision_Contact {

	Vector3 pointA;
	Vector3 pointB;

	Vector3 normal;
	Vector3 tangentA;
	Vector3 tangentB;

	F32 depth;
	// TODO: Local contact points
};

struct collision_GJKResult {
	collision_Simplex simplex;
	bool result;
};

collision_GJKResult collision_doGJK(collision_Object objectA, collision_Object objectB) {
	collision_Simplex simplex = {0};
	Vector3 v = objectB.position - objectA.position;
	int iterations = 0;
	while(iterations++ < MAX_ITERATIONS) {
		collision_SupportPoint p = collision_getMinkowskiSupport(objectA, objectB, v);
		if (p.diff.dot(v) < 0) {
			return {simplex, false};
		}
		simplex.addPoint(p);
		collision_SimplexResult simplexResult = collision_updateSimplex(simplex);		
		if (simplexResult.simplex.pointsCount >= 4) {
			return {simplex, true};
		}
		simplex = simplexResult.simplex;
		v = simplexResult.v;
	}
	return {simplex, false};
}

struct collision_SupportTriangle {
	collision_SupportPoint pointA;
	collision_SupportPoint pointB;
	collision_SupportPoint pointC;
	Vector3 getNormal() {
		Vector3 a = pointA.diff;
		Vector3 b = pointB.diff;
		Vector3 c = pointC.diff;
		Vector3 atb = b - a;
		Vector3 atc = c - a;
		return atb.cross(atc).normalize();
	}
	F32 getDistance() {		
		Vector3 normal = getNormal();
		Vector3 point = pointA.diff;
		return normal.dot(point);
	}

	void getSupportPositions(Vector3* supportPointA, Vector3* supportPointB) {
		Vector3 normal = getNormal();
		Vector3 point = normal * (normal.dot(pointA.diff));
		Vector3 barycentric = math_getBarycentricCoordinates(point,pointA.diff,pointB.diff,pointC.diff);
		*supportPointA = pointA.supportA * (barycentric.x) + (pointB.supportA * (barycentric.y)) + (pointC.supportA * (barycentric.z));
		*supportPointB = pointA.supportB * (barycentric.x) + (pointB.supportB * (barycentric.y)) + (pointC.supportB * (barycentric.z));
	}
};

struct collision_PolytopeTriangle {
	U16 pointA;
	U16 pointB;
	U16 pointC;
};

struct collision_PolytopeEdge {
	U16 pointA;
	U16 pointB;
};

struct collision_Polytope {	
	collision_SupportPoint vertices[MAX_VERTICES];
	collision_PolytopeTriangle triangles[MAX_VERTICES*2];
	collision_PolytopeEdge edges[MAX_VERTICES*3];
	U16 verticesCount;
	U16 trianglesCount;
	U16 edgesCount;

	F32 getTriangleDistance(U16 index) {
		return getSupportTriangle(index).getDistance();
	}

	collision_SupportTriangle getSupportTriangle(U16 index) {
		collision_PolytopeTriangle triangle = triangles[index];
		return {vertices[triangle.pointA],vertices[triangle.pointB],vertices[triangle.pointC]};
	}

	bool canTriangleSeePoint(U16 index, Vector3 point) {
		Vector3 a = vertices[triangles[index].pointA].diff;
		Vector3 b = vertices[triangles[index].pointB].diff;
		Vector3 c = vertices[triangles[index].pointC].diff;
		Vector3 atb = b - a;
		Vector3 atc = c - a;
		Vector3 normal = atb.cross(atc);
		return atb.cross(atc).dot(point - a) >= 0;
	}

	void addEdge(U16 pointA, U16 pointB) {
		bool removed = false;
		for (int i = 0; i < edgesCount; i ++) {
			if (pointA == edges[i].pointB && pointB == edges[i].pointA) {
				edges[i--] = edges[--edgesCount];
				removed = true;
			}
		}
		if (!removed) {
			edges[edgesCount++] = {pointA, pointB};
		}
	}
};

inline collision_Polytope collision_createInitialPolytope(collision_Object objectA, collision_Object objectB, collision_Simplex simplex) {
	// TODO: handle if simplex has less than 4 points

	collision_Polytope polytope;	
	polytope.verticesCount = 4;
	polytope.trianglesCount = 4;
	polytope.edgesCount = 0;

	collision_SupportPoint a = simplex.points[0];
	collision_SupportPoint b = simplex.points[1]; 
	collision_SupportPoint c = simplex.points[2]; 
	collision_SupportPoint d = simplex.points[3]; 

	Vector3 atb = b.diff - a.diff;
	Vector3 atc = c.diff - a.diff;
	Vector3 atd = d.diff - a.diff;

	Vector3 fcba = atb.cross(atc);
	// Make sure the order is correct
	if (fcba.dot(atd) > 0) {
		collision_SupportPoint temp = a;
		a = c;
		c = temp;
	}	
	polytope.vertices[0] = a;
	polytope.vertices[1] = b;
	polytope.vertices[2] = c;
	polytope.vertices[3] = d;
	polytope.triangles[0] = {0,1,2};
	polytope.triangles[1] = {1,0,3};
	polytope.triangles[2] = {3,2,1};
	polytope.triangles[3] = {2,3,0};

	
	return polytope;
}

collision_Contact collision_doEPA(collision_Object objectA, collision_Object objectB, collision_GJKResult gjk) {
	collision_Contact contact;
	const F32 epsilon = 0.0001;
	if (gjk.result) {
		
		// Construct polytope
		collision_Polytope polytope = collision_createInitialPolytope(objectA, objectB, gjk.simplex);
		collision_SupportTriangle closest;
		for(int iterations = 0; iterations < MAX_ITERATIONS; iterations++) { 

			F32 distance = polytope.getTriangleDistance(0);
			U16 closestTriangleIndex = 0;
			for (int i = 1; i < polytope.trianglesCount; i++) {
				F32 polytopeDistance = polytope.getTriangleDistance(i);
				if (polytopeDistance < distance) {
					closestTriangleIndex = i;
					distance = polytopeDistance;
				}
			}
			closest = polytope.getSupportTriangle(closestTriangleIndex);
			Vector3 normal = closest.getNormal();
			collision_SupportPoint support = collision_getMinkowskiSupport(objectA, objectB, normal);	
			F32 supportDistance = support.diff.dot(normal);
			if (supportDistance < distance + epsilon || polytope.verticesCount >= MAX_VERTICES) {
				break;
			}
			// Remove faces, add edges
			for (int i = 0; i < polytope.trianglesCount; i++) {
				if (polytope.canTriangleSeePoint(i,support.diff)) {
					collision_PolytopeTriangle removedTriangle = polytope.triangles[i];
					polytope.addEdge(removedTriangle.pointA,removedTriangle.pointB);
					polytope.addEdge(removedTriangle.pointB,removedTriangle.pointC);
					polytope.addEdge(removedTriangle.pointC,removedTriangle.pointA);
					polytope.triangles[i--] = polytope.triangles[--polytope.trianglesCount];
				}
			}
			
			// Add support and new faces
			U16 newVertexNum = polytope.verticesCount;
			polytope.vertices[polytope.verticesCount++] = support;
			for (int i = 0; i < polytope.edgesCount; i++) {
				collision_PolytopeEdge edge = polytope.edges[i];
				polytope.triangles[polytope.trianglesCount++] = {edge.pointA, edge.pointB, newVertexNum};  				
			}
			// Reset edges
			polytope.edgesCount = 0;
		}
		// Generate contact from closest face
		contact.normal = closest.getNormal();
		contact.depth = closest.getDistance();
		closest.getSupportPositions(&contact.pointA,&contact.pointB);
		if (contact.normal.x > 0.57735f) {
			contact.tangentA = {contact.normal.y, -contact.normal.x, 0.0f};
		} else {
			contact.tangentA = {0.0, contact.normal.z, -contact.normal.y};
		}
		contact.tangentA = contact.tangentA.normalize();
		contact.tangentB = contact.normal.cross(contact.tangentA);	
		


	} else {
		contact.depth = 0.0f;
	}
	return contact;
}

collision_Contact collision_collideObjects(collision_Object objectA, collision_Object objectB) {
	collision_GJKResult result = collision_doGJK(objectA, objectB);
	return collision_doEPA(objectA, objectB, result);
}

bool collision_doObjectsIntersect(collision_Object objectA, collision_Object objectB) {
	return collision_doGJK(objectA,objectB).result;
}

F32 collision_getCollisionDistanceInDirection(collision_Object objectA, collision_Object objectB, Vector3 direction) {
	// TODO: make sure things work with opposite direction
	direction = (-direction).normalize();	
	Vector3 startPoint = collision_getMinkowskiSupport(objectA, objectB, direction).diff;
	startPoint = direction * (startPoint.dot(direction));
	Vector3 moveDirection = -direction;
	F32 maxT = startPoint.length();

	const F32 epsilon = 0.0001;
	const F32 epsilonSquared = epsilon*epsilon;
	F32 t = 0.0f;
	Vector3 x = startPoint;
	Vector3 v = objectA.position - objectB.position;
	collision_Simplex simplex = {0};
	int iterations = 0;
	while (v.lengthSquared() > epsilonSquared && iterations++ < MAX_ITERATIONS) {
		if (simplex.pointsCount == 4) {
			return maxT - t;
		}
		collision_SupportPoint p = collision_getMinkowskiSupport(objectA, objectB, v); // Get closest point to minkowski difference
		Vector3 w = x - p.diff;
		if (v.dot(w) > 0) {
			if (v.dot(moveDirection) >= 0) {
				return maxT - t;
			} else {
				t =  t -  v.dot(w) / v.dot(moveDirection);
				if (t >= maxT) {
					return 0.0f;
				}
				x = direction * (maxT-t);
			}
		}
		simplex.addPoint(p);
		simplex.translate(-x);
		collision_SimplexResult simplexResult = collision_updateSimplex(simplex);
		simplex = simplexResult.simplex;
		simplex.translate(x);
		v = simplexResult.v;
	}	
	return maxT - t;
}

// TOOD: convert into ray collision
// CollisionResult collideObjectsMoving(CollisionObject staticObject, CollisionObject movingObject, Vector3 moveVector, F32 maxT) {
// 	const F32 epsilon = 0.0001;
// 	const F32 epsilonSquared = epsilon*epsilon;
// 	// Minkowski difference is given by staticObject - movingObject; 

// 	CollisionResult result;
// 	result.t = 0.0f; // time of collision initial value
// 	Vector3 x = {0.0f}; // point of collision (idea is to move the origin)
// 	result.normal = {0.0f}; // collision normal
// 	Vector3 v = x.subtract(staticObject.position.subtract(movingObject.position));
// 	CollisionSimplex simplex = {0};
// 	while (v.lengthSquared() > epsilonSquared) {
// 		if (simplex.pointsCount == 4) {
// 			result.t = -1.0f; // should only happen if we started inside
// 			simplex.translate(x.negate());
// 			F32 length = -1.0f;
// 			for (int i = 0; i < 4; i++) {
// 				if (length < 0 || length > simplex.points[i].length()) {
// 					length = simplex.points[i].length();
// 					result.normal = simplex.points[i].negate();
// 				}
// 			}
// 			return result;
// 		}
// 		Vector3 p = getMinkowskiSupport(staticObject, movingObject, v); // Get closest point to minkowski difference
// 		Vector3 w = x.subtract(p);
// 		if (v.dot(w) > 0) {
// 			if (v.dot(moveVector) >= 0) {
// 				result.t = maxT;
// 				return result;
// 			} else {
// 				result.t =  result.t -  v.dot(w) / v.dot(moveVector);
// 				if (result.t >= maxT) {
// 					result.t = maxT;
// 					return result;
// 				}
// 				result.normal = v;
// 				x = moveVector.multiply(result.t);
// 			}
// 		}
// 		simplex.addPoint(p);
// 		simplex.translate(x.negate());
// 		CollisionSimplexResult simplexResult = updateSimplex(simplex);
// 		simplex = simplexResult.simplex;
// 		simplex.translate(x);

// 		v = simplexResult.v;
// 	}
// 	result.t = mmax(0.0,result.t - maxT*epsilon*100.0f);
// 	if (result.normal.lengthSquared() > 0) {
// 		result.normal = result.normal.normalize();
// 	} else {
// 		result.normal = v.normalize();
// 	}	
// 	return result;
// }
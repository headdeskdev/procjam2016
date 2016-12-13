enum PhysicsType {
	PHYSICS_STATIC,
	PHYSICS_DYNAMIC,
	PHYSICS_DYNAMIC_NO_ROTATE,
	PHYSICS_NO_CLIP,
	PHYSICS_CONTROLLER
};

// TODO: special static struct 


struct GameObjectReference {
	void* gameObject;
	U32 gameObjectType;
};

// struct StaticPhysicsStructure {
// 	GameObjectReference gameObject;

// 	// CollisionObject* collisions;

// 	U32 type;
// 	U32 clippableTypes;
// 	U32 interactableTypes;
// 	ContactList* firstContact;
// };

// struct ContactList;

struct PhysicsObject {
	GameObjectReference gameObject;

	collision_Object collision;
	AABB boundingBox;
	PhysicsType physicsType;

	Vector3 position;
	Vector3 velocity;
	Vector3 acceleration;
	
	//Quat rotation;
	//F32 mass;
	// Friction/Inertia/Restitution
	F32 restitution;
	F32 friction;
	F32 drag;
	// TODO: proper constraint system
	bool constantConstraint;

	U32 type;
	U32 clippableTypes;
	U32 interactableTypes;
	//ContactList* firstContact;

	// TODO: figure out how to make this just for objects that require the constraints player objects
	F32 maxNormal;
	F32 minNormal;
	Vector3 horizontalVelocity;
};

struct PhysicsContact {
	collision_Contact contact;
	PhysicsObject* objectA;
	PhysicsObject* objectB;	
	F32 restitutionA;
	F32 restitutionB;
	F32 normalImpulseSum;
	F32 tangentAImpulseSum;
	F32 tangentBImpulseSum;
};

struct PhysicsIntersection {
	PhysicsObject* objectA;
	PhysicsObject* objectB;	
};

// struct ContactList {
// 	CollisionContact contact;
// 	PhysicsObject* otherObject;
// 	ContactList* nextContact;
// };

struct PhysicsSystem {
	PhysicsObject* objects;	
	U32 objectsCount;	
	U32 maxObjects;

	PhysicsObject* staticObjects;
	U32 staticObjectsCount;
	U32 maxStaticObjects;

	PhysicsContact* contacts;
	U32 contactsCount;
	U32 maxContacts;

	PhysicsIntersection* intersections;
	U32 intersectionsCount;
	U32 maxIntersections;

	void clearStaticCollision() {
		staticObjectsCount = 0;
	}
};

AABB calculateBoundingBox(collision_Object collision) {
	AABB boundingBox;
	boundingBox.min.x = collision_getSupport(collision, {-1.0,0.0,0.0}).x;
	boundingBox.min.y = collision_getSupport(collision, {0.0,-1.0,0.0}).y;
	boundingBox.min.z = collision_getSupport(collision, {0.0,0.0,-1.0}).z;
	boundingBox.max.x = collision_getSupport(collision, {1.0,0.0,0.0}).x;
	boundingBox.max.y = collision_getSupport(collision, {0.0,1.0,0.0}).y;
	boundingBox.max.z = collision_getSupport(collision, {0.0,0.0,1.0}).z;
	return boundingBox;
}

PhysicsSystem createPhysicsSystem(U32 maxObjects, U32 maxStaticObjects, U32 maxContacts, U32 maxIntersections) {
	U32 maxSize = (maxObjects + maxStaticObjects) * sizeof(PhysicsObject) + maxContacts * sizeof(PhysicsContact) + maxIntersections * sizeof(PhysicsIntersection);
	memory_Arena memory = memory_createArena(malloc(maxSize), maxSize);

	PhysicsSystem physics = {0};
	physics.objects = ARENA_GET_ARRAY(memory,PhysicsObject,maxObjects);
	physics.maxObjects = maxObjects;

	physics.staticObjects = ARENA_GET_ARRAY(memory,PhysicsObject,maxStaticObjects);
	physics.maxStaticObjects = maxStaticObjects;

	physics.contacts = ARENA_GET_ARRAY(memory,PhysicsContact,maxContacts);
	physics.maxContacts = maxContacts;	

	physics.intersections = ARENA_GET_ARRAY(memory,PhysicsIntersection,maxIntersections);
	physics.maxIntersections = maxIntersections;	
	return physics;
}

PhysicsObject* addStaticCollision(PhysicsSystem* physics, collision_Object collision, F32 restitution, F32 friction) {
	PhysicsObject object = {0};
	object.position = collision.position;
	object.collision = collision;
	object.boundingBox = calculateBoundingBox(object.collision);
	object.physicsType = PHYSICS_STATIC;
	object.type = 1;
	object.clippableTypes = 0xFFFFFFFF;
	object.interactableTypes = 2;
	object.restitution = restitution;
	object.friction = friction;

	physics->staticObjects[physics->staticObjectsCount++] = object;
	return physics->staticObjects + (physics->staticObjectsCount - 1);
}

PhysicsObject* addPhysicsObject(PhysicsSystem* physics, PhysicsObject object) {
	physics->objects[physics->objectsCount++] = object;
	return physics->objects + (physics->objectsCount - 1);
}

#define MAX_CONSTRAINT_ITERATIONS 4
#define CONTACT_SLOP 0.001f
#define RESTITUTION_SLOP 0.01f

void runPhysics(PhysicsSystem* physics, F32 t) {	
	// "Integration" step	
	for (U32 i = 0; i < physics->objectsCount; i++) {
		PhysicsObject* object = physics->objects + i;
		if (object->physicsType != PHYSICS_STATIC) {
			Vector3 acceleration = object->acceleration - (object->velocity * object->drag);
			object->position = object->position + (object->velocity * t) + (object->acceleration * (0.5f*t*t));
			object->velocity = object->velocity + (object->acceleration * t);			
			object->collision.position = object->position;
			object->boundingBox = calculateBoundingBox(object->collision);
			
			object->maxNormal = -2.0f;
			object->minNormal = 2.0f;
			object->horizontalVelocity = {object->velocity.x,0.0f,object->velocity.z};
		}
	}

	physics->contactsCount = 0;
	physics->intersectionsCount = 0;

	for (U32 i = 0; i < physics->objectsCount; i++) {
		for (U32 j = 0; j < physics->objectsCount; j++) {
			// TODO: non static collisions
			// Check bounding box

			// If bounding box check types 			
				// do full collision with contact or not

				// Add contact if depth > 0 and can clip

				// If max contacts reached then ??? (should be quite high)	
		}
	}

	for (U32 i = 0; i < physics->objectsCount; i++) {
		PhysicsObject* objectB = physics->objects + i;
		for (U32 j = 0; j < physics->staticObjectsCount; j++) {
			PhysicsObject* objectA = physics->staticObjects + j;			
			// Check bounding box
			bool canClip = (objectA->type & objectB->clippableTypes) || (objectB->type & objectA->clippableTypes);
			bool canInteract = (objectA->type & objectB->interactableTypes) || (objectB->type & objectA->interactableTypes);
			
			if ((canClip || canInteract) && objectA->boundingBox.intersect(objectB->boundingBox)) {
				bool interact = false;
				if (canClip) {
					collision_Contact contact = collision_collideObjects(objectA->collision, objectB->collision);
					if (contact.depth > 0.0f) {						
						if (physics->contactsCount < physics->maxContacts) {							
							F32 normalAxisSpeedB = contact.normal.dot(objectB->velocity);
							normalAxisSpeedB = mmin(normalAxisSpeedB - RESTITUTION_SLOP,0.0f);
							F32 restitutionB = normalAxisSpeedB*objectA->restitution*objectB->restitution;							
							PhysicsContact physicsContact = {contact, objectA, objectB, 0.0f, restitutionB};
							physics->contacts[physics->contactsCount++] = physicsContact;
							// TODO: only for certain objects, also up should be generalised
							Vector3 up = {0.0, 1.0, 0.0};
							objectB->maxNormal = mmax(contact.normal.dot(up),objectB->maxNormal);
							objectB->minNormal = mmin(contact.normal.dot(up),objectB->minNormal);	

						} else {
							// SHIT!! WHAT DO WE DO NOW
						}
					}
				} else {
					interact = collision_doObjectsIntersect(objectA->collision, objectB->collision);
				}

				if (interact && canInteract) {
					PhysicsIntersection intersection = {objectA, objectB};
					physics->intersections[physics->intersectionsCount++] = intersection;
				}						
			}
		}
	}
	// TODO: determine correct number of iterations based on time available / average number of contraints
	// TODO: only iterate over constraints that are not alone (alone contraints converge immediately and probably make up a majority of constraints)
	for (int i = 0; i < MAX_CONSTRAINT_ITERATIONS; i++) {

		// TODO: handle other contraints
		// DO FRICTIONS
		// for (U32 c = 0; c < physics->contactsCount; c++) {
		// 	PhysicsContact* contact = physics->contacts + c;
		// 	F32 friction = contact->objectA->friction*contact->objectB->friction;
		// 	{
		// 		Vector3 j = contact->contact.tangentA;
		// 		Vector3 v = contact->objectB->velocity;				
		// 		F32 lagrangian = -(j.dot(v))/j.dot(j);

		// 		F32 tangentImpulseSum = contact->tangentAImpulseSum;
		// 		tangentImpulseSum += lagrangian;
		// 		F32 maxImpulse = friction*contact->normalImpulseSum;				
		// 		tangentImpulseSum = mmin(tangentImpulseSum,maxImpulse);
		// 		tangentImpulseSum = mmax(tangentImpulseSum,-maxImpulse);
		// 		lagrangian = tangentImpulseSum - contact->tangentAImpulseSum;
		// 		contact->tangentAImpulseSum = tangentImpulseSum;

		// 		Vector3 delta = j * lagrangian;
		// 		contact->objectB->velocity = contact->objectB->velocity + delta;	
		// 	}
		// 	{
		// 		Vector3 j = contact->contact.tangentB;
		// 		Vector3 v = contact->objectB->velocity;				
		// 		F32 lagrangian = -(j.dot(v))/j.dot(j);

		// 		F32 tangentImpulseSum = contact->tangentBImpulseSum;
		// 		tangentImpulseSum += lagrangian;
		// 		F32 maxImpulse = friction*contact->normalImpulseSum;				
		// 		tangentImpulseSum = mmin(tangentImpulseSum,maxImpulse);
		// 		tangentImpulseSum = mmax(tangentImpulseSum,-maxImpulse);
		// 		lagrangian = tangentImpulseSum - contact->tangentBImpulseSum;
		// 		contact->tangentBImpulseSum = tangentImpulseSum;

		// 		Vector3 delta = j * lagrangian;
		// 		contact->objectB->velocity = contact->objectB->velocity + delta;	
		// 	}
		// }
		// Handle contact constraints	
		// TODO: handle two bodies vs single body
		for (U32 c = 0; c < physics->contactsCount; c++) {			
			PhysicsContact* contact = physics->contacts + c;
			if (contact->objectA->physicsType == PHYSICS_STATIC) {
				Vector3 j = contact->contact.normal;
				Vector3 v = contact->objectB->velocity;

				F32 b = -(0.1*mmax(contact->contact.depth-CONTACT_SLOP,0.0f))/t + contact->restitutionB;				
				F32 lagrangian = -(j.dot(v)+b)/j.dot(j);

				// Clamp to stop us from moving backwards when evaluating multiple constraints
				F32 normalImpulseSum = contact->normalImpulseSum;
				normalImpulseSum += lagrangian;
				if (normalImpulseSum < 0) {
					normalImpulseSum = 0.0f;
				}
				lagrangian = normalImpulseSum - contact->normalImpulseSum;
				contact->normalImpulseSum = normalImpulseSum;

				Vector3 delta = j * (lagrangian);
				contact->objectB->velocity = contact->objectB->velocity + delta;	

				// Simple single object + constraint
			} else {
				// TODO: physics resoultion with non static objects
				// Determine which resolution to use
			}
		}
		// TODO: remove hardcoded values
		
		for (U32 i = 0; i < physics->objectsCount; i++) {
			PhysicsObject* object = physics->objects + i;
			if (object->constantConstraint) {							
				if (object->minNormal >= 0.55) {
					object->velocity.x = object->horizontalVelocity.x;
					object->velocity.z = object->horizontalVelocity.z;
				} 						
			}
		}
		for (U32 i = 0; i < physics->objectsCount; i++) {
			PhysicsObject* object = physics->objects + i;			
			Vector3 horizontalVelocity = {object->velocity.x,0.0f,object->velocity.z};
			F32 speed = horizontalVelocity.length();
			if (speed > 50.0f) {
				object->velocity.x = horizontalVelocity.x*50.0f/speed;	
				object->velocity.z = horizontalVelocity.z*50.0f/speed;
			}										
		}
	}
}

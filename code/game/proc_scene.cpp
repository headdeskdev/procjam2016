struct ProcObject {
	U32 type;
	Vector3	position;
	union {
		struct {
			Vector3 scale;
			Quat rotation;
		};
		struct {
			Vector3 color;
			Vector3 direction;
            F32 radius;
			F32 intensity;
            F32 coneDot;
		};
	};
};

struct ProcObjectList {
    ProcObject* objects;
    U32 objectsCount;
    U32 objectsMax;
    void addObject(Vector3 scale, Vector3 position, Quat rotation, U32 ctype)
    {
        if (objectsCount < objectsMax) {
			objects[objectsCount++] = { ctype, position, {scale, rotation} };
        }
    }
	void addLight(Vector3 color, Vector3 position, Vector3 direction, F32 radius, F32 intensity, F32 dot) {
		if (objectsCount < objectsMax) {
			objects[objectsCount] = { 4, position };
            objects[objectsCount].direction = direction;
            objects[objectsCount].color = color;
            objects[objectsCount].radius = radius;
            objects[objectsCount].intensity = intensity;
            objects[objectsCount].coneDot = dot;
            objectsCount++;
		}
	}
};

Matrix4 getModelMatrixForProcObject(ProcObject object) {
    Matrix4 translationMatrix = math_getTranslationMatrix(object.position);
    Matrix4 scaleMatrix = {  object.scale.x,0.0,0.0,0.0,
                             0.0,object.scale.y,0.0,0.0,
                             0.0,0.0,object.scale.z,0.0,
                             0.0,0.0,0.0,1.0f};

    return translationMatrix * (object.rotation.toRotationMatrix() * scaleMatrix);
}
Matrix4 getNormalModelMatrixForProcObject(ProcObject object) {
    Matrix4 translationMatrix = math_getTranslationMatrix(object.position);
    Matrix4 scaleMatrix = {  1.0f/object.scale.x,0.0,0.0,0.0,
                             0.0,1.0f/object.scale.y,0.0,0.0,
                             0.0,0.0,1.0f/object.scale.z,0.0,
                             0.0,0.0,0.0,1.0f};

    return translationMatrix * (object.rotation.toRotationMatrix() * scaleMatrix);
}

void renderProcObjectListScene(WorldRenderer* renderer, Player* player, ProcObjectList* procObjs, GameAssets* assets, F32 t) {
    
    // yellow -> pink -> light blue -> light blue -> orange -> purple -> darkblue -> darkblue -> darkpurple -> darkyellow -> yellow
    const F32 intervalTimes[10] = {2.0f,1.0f,15.0f,1.5f,1.5f, 0.5f,14.0f,1.0f,1.0f,3.5f};    
    Vector3 backgroundColor[10] = {{0.8f,0.75f,0.55f},{0.6f,0.6f,0.7f},{0.5f,0.7f,0.9f},{0.5f,0.7f,0.9f},{0.9f,0.7f,0.5f},
                                   {0.3f,0.2f,0.4f},{0.04f,0.04f,0.2f},{0.04f,0.04f,0.2f},{0.2f,0.15f,0.3f},{0.4f,0.35f,0.1f}};
	const F32 dynamicFactor[10] = { 0.0f,0.0f,0.0f,0.0f,0.5f,0.9f,1.0f,0.9f,0.8f,0.5f };
	Vector3 ambientColor[10] = { { 0.04f,0.02f,0.01f },{ 0.02f,0.01f,0.04f },{ 0.01f,0.02f,0.04f },{ 0.01f,0.02f,0.04f },{ 0.04f,0.02f,0.01f },
								 { 0.03f,0.02f,0.02f },{ 0.005f,0.01f,0.015f },{ 0.005f,0.01f,0.015f },{ 0.03f,0.02f,0.02f },{ 0.03f,0.03f,0.01f } };
	const F32 skyAngle[10] = { PI * 0.1f, PI * 0.2f, PI * 0.3f, PI*0.7f, PI * 0.9f, PI * 1.0f, PI * 0.8f, PI * 0.2f, PI * 0.1f, PI * 0.0f };
	Vector3 mainColor[10] = { { 0.35f,0.35f,0.2f },{ 0.4f,0.37f,0.22f },{ 0.45f,0.37f,0.25f },{ 0.45f,0.37f,0.25f },{ 0.45f,0.37f,0.15f },
								{ 0.15f,0.1f,0.15f },{ 0.07f,0.07f,0.1f },{ 0.07f,0.07f,0.1f },{ 0.15f,0.1f,0.2f },{ 0.25f,0.25f,0.2f } };
    
	F32 totalInterval = 0.0f;
    for (int i = 0; i < 10; i++) {
        totalInterval += intervalTimes[i];
    }
    F32 currentIntervalPoint = fmod(t,totalInterval);
    F32 intervalCounter = 0.0f;
	F32 dayFactor = 0.0f;
    for (int i = 0; i < 10; i++) {
		intervalCounter += intervalTimes[i];
        if (i == 9 || intervalCounter > currentIntervalPoint) {
            F32 intervalFraction = (currentIntervalPoint-intervalCounter+intervalTimes[i])/intervalTimes[i];
            renderer->backgroundColour = backgroundColor[(i+1)%10] * intervalFraction + backgroundColor[i] * (1-intervalFraction);
			dayFactor = dynamicFactor[(i + 1) % 10] * intervalFraction + dynamicFactor[i] * (1 - intervalFraction);
			renderer->ambientLight.color = ambientColor[(i + 1) % 10] * intervalFraction + ambientColor[i] * (1 - intervalFraction);
			// Make ambient lesser
			renderer->ambientLight.color = renderer->ambientLight.color * 0.3;
			F32 angle = skyAngle[(i + 1) % 10] * intervalFraction + skyAngle[i] * (1 - intervalFraction);
			Vector3 horizontalDirection = { 0.9f,0.0f,0.7f };
			Vector3 verticalDirection = { 0.0f,-1.0f,0.0f };
			Vector3 tangentDirection = { -0.7f,0.0f,0.9f };
			
			horizontalDirection = horizontalDirection.normalize();
			renderer->directionalLights[0].dir = verticalDirection*sinf(angle) + horizontalDirection*cosf(angle);
			renderer->directionalLights[0].color = mainColor[(i + 1) % 10] * intervalFraction + mainColor[i] * (1 - intervalFraction);
			// Make main light greater
			renderer->directionalLights[0].color = renderer->directionalLights[0].color * 2.0;
			renderer->directionalLights[1].dir = tangentDirection.cross(renderer->directionalLights[0].dir);
			if (renderer->directionalLights[1].dir.y > 0) {
				renderer->directionalLights[1].dir = renderer->directionalLights[1].dir * -1.0f;
			}
			break;
        }
        
    }
	renderer->directionalLights[1].color = { 0.02f,0.03f,0.05f };

	ProcObject grappleObject;
	grappleObject.position = player->grapplingHook.physics->position;
	grappleObject.rotation = math_getAngleAxisQuat(PI, { 0.0,0.0,1.0 });
	grappleObject.scale = { 0.175f,0.175f,0.175f };
	Matrix4 drawMatrix = getModelMatrixForProcObject(grappleObject);
	Matrix4 normalMatrix = getNormalModelMatrixForProcObject(grappleObject);
	graphics_Mesh* mesh = assets->meshes[0];
	renderer->addObject(mesh, assets->hyperMaterial, &drawMatrix, &normalMatrix);

	// Do hover lights
	for (int i = 0; i < 2; i++) {
		F32 hTime = t + i*3.5f;
		U32 num = ((U32) (hTime / 8.0f));
		F32 v = (hTime - num*8.0f) / 8.0f;
		RandomGenerator r = { num * 2 + i };
		Vector3 pos = { r.frand()*300.0f - 150.0f, 55.0f + i *4.0f, r.frand()*300.0f - 150.0f };
		Vector3 direction = { 0.0,-1.0,0.0 };
		switch (r.getInteger(4)) {
			case 0:
				direction.x += 0.3;
				pos.x = -250.0f + 500.0f * v;
				break;
			case 1:
				direction.x -= 0.3;
				pos.x = 250.0f - 500.0f * v;
				break;
			case 2:
				direction.z += 0.3;
				pos.z = -250.0f + 500.0f * v;
				break;
			default:
				direction.z -= 0.3;
				pos.z = 250.0f - 500.0f * v;
				break;
		}
		LightInstance newLight = { 0.0 };
		newLight.pos = pos;
		newLight.dir = direction.normalize();
		newLight.color = { 0.8f,0.0,0.0 };
		newLight.coneDot = 0.75f;
		newLight.radius = 100.0f;
		newLight.intensity = 5.0f;
		renderer->addLight(&newLight);
		ProcObject hoverObject;
		hoverObject.position = pos;
		hoverObject.rotation = math_getAngleAxisQuat(PI, { 0.0,0.0,1.0 });
		hoverObject.scale = { 1.0,1.0,1.0 };
		Matrix4 drawMatrix = getModelMatrixForProcObject(hoverObject);
		Matrix4 normalMatrix = getNormalModelMatrixForProcObject(hoverObject);
		graphics_Mesh* mesh = assets->meshes[2];
		renderer->addObject(mesh, assets->hyperMaterial, &drawMatrix, &normalMatrix);
	}
	// TODO: cull distant/outside frustum/small+far objects
    for(int i = 0; i < procObjs->objectsCount; i++) {
		if (procObjs->objects[i].type < 4) {
			Matrix4 drawMatrix = getModelMatrixForProcObject(procObjs->objects[i]);
			Matrix4 normalMatrix = getNormalModelMatrixForProcObject(procObjs->objects[i]);
			graphics_Mesh* mesh = assets->meshes[procObjs->objects[i].type];
			renderer->addObject(mesh, assets->hyperMaterial, &drawMatrix, &normalMatrix);
		} else if (procObjs->objects[i].type == 4 && dayFactor > 0.0f) {
			LightInstance newLight = { 0.0 };
			newLight.pos = procObjs->objects[i].position;
			newLight.dir = procObjs->objects[i].direction;
			newLight.color = procObjs->objects[i].color;
			newLight.coneDot = procObjs->objects[i].coneDot;
			newLight.radius = procObjs->objects[i].radius;
            newLight.intensity = procObjs->objects[i].intensity * dayFactor;
			renderer->addLight(&newLight);
		}
    }
}

collision_Object getAlignedCuboidCollisionObject(Vector3 size, Vector3 centralPosition, Vector3 offset, Quat rotation) {
    collision_Object collisionObject;
    collisionObject.position = centralPosition + rotation.transformVector(offset);
    collisionObject.type = COLLISION_OBJECT_CUBOID;

    collision_Cuboid cuboid = {{{size.x/2.0f,0.0f,0.0f},{0.0f,size.y/2.0f,0.0f},{0.0,0.0,size.z/2.0f}}};
    for (int i = 0; i < 3; i++) {
        cuboid.halfSizeAxes[i] = rotation.transformVector(cuboid.halfSizeAxes[i]);
    }
    collisionObject.cuboid = cuboid;
    return collisionObject;
}

collision_Object getCuboidCollisionObject(collision_Cuboid cuboid, Vector3 centralPosition, Vector3 offset, Quat rotation) {
    collision_Object collisionObject;
    collisionObject.position = centralPosition + rotation.transformVector(offset);
    collisionObject.type = COLLISION_OBJECT_CUBOID;    
    for (int i = 0; i < 3; i++) {
        cuboid.halfSizeAxes[i] = rotation.transformVector(cuboid.halfSizeAxes[i]);
    }
    collisionObject.cuboid = cuboid;
    return collisionObject;
}

collision_Object getPrismCollisionObject(collision_QuadPrism prism, Vector3 centralPosition, Vector3 offset, Quat rotation) {
    collision_Object collisionObject;
    collisionObject.position = centralPosition + rotation.transformVector(offset);
    collisionObject.type = COLLISION_OBJECT_QUADPRISM;    
    prism.x = rotation.transformVector(prism.x);
    prism.y = rotation.transformVector(prism.y);
    prism.up = rotation.transformVector(prism.up);
    collisionObject.quadPrism = prism;
    return collisionObject;

}
 
void generateStaticPhysicsFromProcObjectList(ProcObjectList* procObjs, PhysicsSystem* physics) {
    physics->clearStaticCollision();
    for (int i = 0; i < procObjs->objectsCount; i++) {
        ProcObject* object = procObjs->objects + i;

        Vector3 scale = object->scale;
        Vector3 position = object->position;
        Quat rotation = object->rotation;
    
        switch(object->type) {
            case 0: {
                if (scale.x < 2.0f && scale.y < 2.0f) {
                    Vector3 size = {scale.x*2.0f,scale.y*3.0f,scale.z*2.0f};
                    Vector3 offset = {0.0f};
                    collision_Object collision = getAlignedCuboidCollisionObject(size, position, offset, rotation);
                    addStaticCollision(physics,collision,0.0f,1.0f);                    
                } else {
                    {
                        Vector3 size = {scale.x*1.0f,scale.y*3.0f,scale.z*2.0f};
                        Vector3 offset = {scale.x*-0.5f,0.0f,0.0f};
                        collision_Object collision = getAlignedCuboidCollisionObject(size, position, offset, rotation);
                        addStaticCollision(physics,collision,0.0f,1.0f);                        
                    }
                    {
                        Vector3 size = {scale.x*2.0f,scale.y*3.0f,scale.z*1.0f};
                        Vector3 offset = {0.0f,0.0f,scale.z*-0.5f};
                        collision_Object collision = getAlignedCuboidCollisionObject(size, position, offset, rotation);
                        addStaticCollision(physics,collision,0.0f,1.0f);                        
                    }
                    {
                        collision_Cuboid cuboid = {{{scale.x/2.0f,0.0f,scale.z/2.0f},{0.0f,scale.y*1.5f,0.0f},{scale.x/2.0f,0.0f,-scale.z/2.0f}}};
                        Vector3 offset = {0.0f};
                        collision_Object collision = getCuboidCollisionObject(cuboid, position, offset, rotation);
                        addStaticCollision(physics,collision,0.0f,1.0f);                        
                    }
                    if (scale.z > 3.0f) {
                        for (int j = 0; j < 3; j++) {
                            Vector3 size = {scale.x*0.8f,scale.y*0.5f,scale.z*0.2f};
                            Vector3 offset = {scale.x*-0.5f,scale.y*(j*1.0f - 1.15f),scale.z*1.0f};
                            collision_Object collision = getAlignedCuboidCollisionObject(size, position, offset, rotation);
                            addStaticCollision(physics,collision,0.0f,1.0f);    
                        }
                    }
                }

                if (scale.y > 3.0f) {
                    Vector3 size = {scale.x * 0.8f, scale.y * 0.2f, scale.z * 0.4f};
                    Vector3 offset = {scale.x * 0.5f, scale.y * 1.5f, scale.z * -0.7f};
                    collision_Object collision = getAlignedCuboidCollisionObject(size, position, offset, rotation);
                    addStaticCollision(physics,collision,0.0f,1.0f);
                }

                        
            } break;
            case 1: {
                Vector3 size = {scale.x*1.0f,scale.y*16.0f,scale.z*1.0f};
                Vector3 offset = {0.0f};
                collision_Object collision = getAlignedCuboidCollisionObject(size, position, offset, rotation);
                addStaticCollision(physics,collision,0.0f,1.0f);    
            } break;
            case 2: {
                Vector3 size = {scale.x*10.0f,scale.y*2.0f,scale.z*20.0f};
                Vector3 offset = {0.0f};
                collision_Object collision = getAlignedCuboidCollisionObject(size, position, offset, rotation);
                addStaticCollision(physics,collision,0.0f,1.0f);
                if (scale.y > 0.5f) {
                    Vector3 size = {scale.x*7.0f,scale.y*2.0f,scale.z*6.0f};
                    {
                        Vector3 offset = {0.0f, -1.0f*scale.y, scale.z*5.0f};
                        collision_Object collision = getAlignedCuboidCollisionObject(size, position, offset, rotation);
                        addStaticCollision(physics,collision,0.0f,1.0f);
                    }
                    {
                        Vector3 offset = {0.0f, -1.0f*scale.y, scale.z*-5.0f};
                        collision_Object collision = getAlignedCuboidCollisionObject(size, position, offset, rotation);
                        addStaticCollision(physics,collision,0.0f,1.0f);
                    }
                }
            } break;
            case 3: {
                // TODO: other scale considerations
                collision_QuadPrism prism = {{0.0,1.0,0.0},{1.0,0.0,0.0},{0.0,0.0,1.0},
                                             {{scale.x*5.0f,scale.z*-10.0f},{scale.x*5.0f,scale.z*10.0f},{scale.x*-9.0f,scale.z*9.0f},{scale.x*-4.0f,scale.z*-10.0f}},
                                            scale.y*2.0f};
                Vector3 offset = {0.0f};
                collision_Object collision = getPrismCollisionObject(prism, position, offset, rotation);
                addStaticCollision(physics,collision,0.0f,1.0f);
            } break;
        }
    }
}
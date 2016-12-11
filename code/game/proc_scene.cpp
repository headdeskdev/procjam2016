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
			F32 radius;
			F32 intensity;
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
	void addLight(Vector3 color, Vector3 position, F32 radius, F32 intensity) {
		if (objectsCount < objectsMax) {
			objects[objectsCount++] = { 4, position,{ color, radius, intensity } };
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

void renderProcObjectListScene(ProcObjectList* procObjs, WorldRenderer* renderer, GameAssets* assets) {
    renderer->backgroundColour = {0.21763764f*0.6f,0.21763764f*0.6f,0.6f};    
	renderer->directionalLights[0].color = { 0.9f,0.75f,0.5f };
	renderer->directionalLights[0].dir = { 0.9f,-0.8f,0.7f };
	renderer->directionalLights[0].dir = renderer->directionalLights[0].dir.normalize();
	renderer->directionalLights[1].color = { 0.1f,0.2f,0.4f };
	renderer->directionalLights[1].dir = { -0.9f,-0.4f,-0.7f };
	renderer->directionalLights[1].dir = renderer->directionalLights[1].dir.normalize();
	renderer->ambientLight.color = { 0.05f,0.1f,0.15f };
	// DARKEN
	renderer->ambientLight.color = renderer->ambientLight.color * 0.5;
	renderer->directionalLights[0].color = renderer->directionalLights[0].color * 0.4;
	renderer->directionalLights[1].color = renderer->directionalLights[1].color * 0.3;

    for(int i = 0; i < procObjs->objectsCount; i++) {
		if (procObjs->objects[i].type < 4) {
			Matrix4 drawMatrix = getModelMatrixForProcObject(procObjs->objects[i]);
			Matrix4 normalMatrix = getNormalModelMatrixForProcObject(procObjs->objects[i]);
			graphics_Mesh* mesh = assets->meshes[procObjs->objects[i].type];
			Vector3 colour = { 0 };
			renderer->addObject(mesh, assets->hyperMaterial, &drawMatrix, &normalMatrix);
		} else {
			LightInstance newLight = { 0.0 };
			newLight.pos = procObjs->objects[i].position;
			newLight.dir.x = 1.0;
			newLight.color = procObjs->objects[i].color;
			newLight.coneDot = -2.0f;
			newLight.radius = procObjs->objects[i].radius;
            newLight.intensity = procObjs->objects[i].intensity;
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
                // TODO: long tall object collision
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
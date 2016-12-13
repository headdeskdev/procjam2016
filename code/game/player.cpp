struct GroundPlane {
    Vector3 tangentA;
    Vector3 tangentB;
};

struct Player {
    PhysicsObject* physics;
    PhysicsObject* groundConstraintPhysics;
    Vector2 lastMousePosition;   
    GroundPlane groundPlanes[10];
    U32 groundPlanesCount;
    U32 groundFrames;
    F32 tilt;
    F32 pan;
};

struct Camera3d {
    Matrix4 projectionMatrix;
    Matrix4 viewMatrix;
};

Camera3d getPlayerCamera(Player* player, Vector2 screenSize) {
    Vector3 viewDirection = {cosf(player->pan)*cosf(player->tilt),sinf(player->tilt),sinf(player->pan)*cosf(player->tilt)};
    Vector3 up = player->physics->collision.capsule.up;

    F32 cameraHeight = player->physics->collision.capsule.capRadius/2.0f+player->physics->collision.capsule.middleHeight/2.0f;

    return {math_getPerspectiveMatrix(0.1f,1000.0,screenSize.x/screenSize.y,1.396f),
            math_getViewMatrix(player->physics->position + (up * cameraHeight), viewDirection, up)};
}

void updatePlayerPostPhysics(Player* player, PhysicsSystem* physics) {
    // Generate standing planes from contacts
    player->groundPlanesCount = 0;
    for (U32 c = 0; c < physics->contactsCount; c++) {
        PhysicsContact* contact = physics->contacts + c;
        if (contact->objectA->physicsType == PHYSICS_STATIC) {
            if (contact->objectB->gameObject.gameObjectType == 1) {
                if (contact->contact.normal.dot({0.0,1.0f,0.0}) > 0.0f) {
                    GroundPlane groundPlane = {contact->contact.tangentA, contact->contact.tangentB};
                    player->groundPlanes[player->groundPlanesCount++] = groundPlane;
                }
            }
        }
                
    }
}

PhysicsObject getPlayerPhysicsObject() {
    PhysicsObject physicsObject = {0};

    collision_Object collisionObject;
    collisionObject.position = {1.4f,3.5f,0.2f};
    collisionObject.type = COLLISION_OBJECT_CAPSULE;

    collision_Capsule capsule = {0.6f,0.7f,{0.0,1.0,0.0}};            
    collisionObject.capsule = capsule;
                
    physicsObject.collision = collisionObject;
    physicsObject.position = physicsObject.collision.position;
    physicsObject.physicsType = PHYSICS_DYNAMIC_NO_ROTATE;            
    physicsObject.type = 4;
    physicsObject.clippableTypes = 1;
    physicsObject.interactableTypes = 0;
    physicsObject.restitution = 0.01f;
    physicsObject.friction = 0.05f;
    physicsObject.drag = 0.2f;

    physicsObject.gameObject.gameObjectType = 1;
    return physicsObject;
}

void updatePlayer(Player* player, platform_Input* input, F32 t, bool debugMode) {
    if (player->physics->maxNormal >= 0.55) {
        player->groundFrames = 0;
    } else {
        player->groundFrames += 1;
    }
    Vector2 lookDirection = {0.0};

    if (BUTTON_WAS_PRESSED(input->k_r)) {
        player->physics->position = {1.4f,150.0f,0.2f};
    }

    if (BUTTON_WAS_PRESSED(input->mousePointers[1].button)) {
        player->lastMousePosition = input->mousePointers[1].finalPosition();
    }
    if (input->lockMouse) {
        player->lastMousePosition = { (F32) (input->windowWidth / 2), (F32) (input->windowHeight / 2)};
    } 
    if (input->mousePointers[1].button.isDown || input->lockMouse) {
        Vector2 finalPosition = input->mousePointers[1].finalPosition();
        if (finalPosition.x > 0 && finalPosition.y > 0) {
            lookDirection = player->lastMousePosition - finalPosition;
            lookDirection.x = -lookDirection.x;
            player->lastMousePosition = finalPosition;
        }
    }

    Vector3 moveDirection = {0.0};
    bool isMoving = false;
    
    if (input->k_w.isDown) {
        moveDirection.y += 1.0;
    }
    if (input->k_d.isDown) {
        moveDirection.x += 1.0; 
    }
    if (input->k_a.isDown) {
        moveDirection.x -= 1.0; 
    }
    if (input->k_s.isDown) {
        moveDirection.y -= 1.0; 
    }
    if (moveDirection.length() > 1.0) {
        moveDirection = moveDirection.normalize();
    }
    if (moveDirection.length() > 0.1) {
        isMoving = true;
    }        

    if (isMoving) {
        player->physics->constantConstraint = false;
    } else {
        player->physics->constantConstraint = true;
    }
    
    F32 lookSpeed = 0.3;


    player->tilt += lookDirection.y * lookSpeed * t;  
    if (player->tilt > 1.55) player->tilt = 1.55;
    if (player->tilt < -1.55) player->tilt = -1.55;
    player->pan += lookDirection.x * lookSpeed * t;

    if (debugMode) {
        Vector3 forwardDirection = {cosf(player->pan)*cosf(player->tilt),sinf(player->tilt),sinf(player->pan)*cosf(player->tilt)};
        Vector3 horizontalDirection = {-sinf(player->pan), 0.0, cosf(player->pan)};                
        //Vector3 down = {0.0f, moveDirection.z, 0.0f};
        Vector3 moveVector = (forwardDirection * moveDirection.y) + 
                             (horizontalDirection * moveDirection.x);


        if (input->k_lshift.isDown) {
            player->physics->acceleration = moveVector * 200.0f; 
        } else {
            player->physics->acceleration = moveVector * 40.0f; 
        }
        

        player->physics->acceleration = player->physics->acceleration - (player->physics->velocity * 4.0f);

        player->physics->friction = 0.4f;  
        player->physics->clippableTypes = 0;    
    } else {
        Vector3 forwardDirection = {cosf(player->pan),0.0,sinf(player->pan)};
        Vector3 horizontalDirection = {-sinf(player->pan), 0.0, cosf(player->pan)};                
        //Vector3 down = {0.0f, moveDirection.z, 0.0f};
        Vector3 moveVector = (forwardDirection * moveDirection.y) + 
                             (horizontalDirection * moveDirection.x);


        Vector3 gravity = {0.0,-9.8f,0.0};                        
        if (player->groundFrames < 3) {
            if (player->groundPlanesCount > 0 && isMoving) {
                Vector3 moveDirection = {0.0,-1.0,0.0};
                for (int i = 0; i < player->groundPlanesCount; i++) {
                    Vector3 tangentA = player->groundPlanes[i].tangentA * (player->groundPlanes[i].tangentA.dot(moveVector));
                    Vector3 tangentB = player->groundPlanes[i].tangentB * (player->groundPlanes[i].tangentB.dot(moveVector));
                    Vector3 newMove = (tangentA + tangentB).normalize();
                    if (newMove.y > moveDirection.y) {
                        moveDirection = newMove;
                    }
                }
                moveVector = moveDirection;
            }
            player->physics->acceleration = moveVector * 80.0f + gravity;        
        } else {
            player->physics->acceleration = moveVector * 5.0f + gravity;
        }



        // Move resistance

        if (player->groundFrames < 2) {
            F32 drag = 7.2f;
            player->physics->acceleration = player->physics->acceleration - (player->physics->velocity * drag);
        } else {
            player->physics->acceleration = player->physics->acceleration - (player->physics->velocity * 0.185f);
        }

        if (player->groundFrames <= 2 && BUTTON_WAS_PRESSED(input->k_space)) {
            player->groundFrames = 3;
            Vector3 jump = {0.0f, 8.0f/t, 0.0f};
            player->physics->acceleration = player->physics->acceleration + jump;
        }

        player->physics->friction = 0.4f;  
        player->physics->clippableTypes = 1;          
    }
}
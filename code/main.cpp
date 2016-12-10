#define MAIN_FILE

#include "engine/engine.cpp"

#include "renderer/graphics.h"

#include "renderer/opengl/graphics_opengl.cpp"
//#include "renderer/directx/graphics_directx11.cpp"

#include "renderer/test/test_load.cpp"
#include "renderer/test/test_material.cpp"
#include "renderer/test/basic_renderer.cpp"

#include "engine/physics.cpp"
#include "engine/text.cpp"
#include "engine/assets.cpp"

#include "game/proc_gen.cpp"
#include "game/player.cpp"
//#include "game/world_renderer.cpp"

#include "debug/imgui.cpp"

struct GameState {
    bool loaded;

    // Assets
    // NOTE: Since there is a minimal number of assets, I have decided not to create/use asset manager for this
    graphics_Texture* mainTexture;
    graphics_Shader* hyperShader;
    graphics_Shader* debugShader;
    graphics_Shader* shader2d;    
    graphics_Mesh* meshes[4];
    graphics_Material* testMaterial;
    graphics_Material* debugMaterial;
    graphics_Material* material2d;
    Font* font;

    memory_Arena assetMemory;
    memory_Arena frameMemory;

    Player player;
    
    ProcObjectList objectList;
    PhysicsSystem physicsSystem;   

    ImguiGLState imguiState;

    //ProcWorldRenderer renderer;
    SimpleRenderer renderer;

    bool debugMode;
};



#define DEFAULT_MEMORY_STATE_STRUCT GameState
#include "engine/default_memory.cpp"

CORE_LOOP {
    BEGIN_CORE_LOOP();
    DefaultMemory* defMemory = getDefaultMemory(memory);    
    GameState* gameState = &defMemory->state;
    gameState->assetMemory = memory_createArena(defMemory->memory.get(100000000),100000000);

    // TEST LOAD SOME 3D Assets
    if (!gameState->loaded) {   
        // TODO: this should probably be in the platform code
        graphics_init();

        memory_Stack loadMemory;
        gameState->frameMemory = memory_createArena(defMemory->memory.get(30000000), 30000000);
        loadMemory.init(10000000, gameState->frameMemory.get(10000000));

        loadMemory.push();
        gameState->mainTexture = getTextureFromFile("maintexture.png", &loadMemory);
        loadMemory.pop();
        
        loadMemory.push();
        gameState->hyperShader = getShaderFromFile("shader.vert", "shader.frag", &loadMemory);
        loadMemory.pop();

        loadMemory.push();
        gameState->debugShader = getShaderFromFile("shaderdebug.vert", "shaderdebug.frag", &loadMemory);
        loadMemory.pop();

        loadMemory.push();
        gameState->shader2d = getShader2dFromFile("shader2d.vert", "shader2d.frag", &loadMemory);
        loadMemory.pop();

        loadMemory.push();
        gameState->meshes[0] = getMeshFromFile("block.vsmf", &loadMemory);        
        loadMemory.pop();
        loadMemory.push();
        gameState->meshes[1] = getMeshFromFile("bar.vsmf", &loadMemory);        
        loadMemory.pop();
        loadMemory.push();
        gameState->meshes[2] = getMeshFromFile("floor.vsmf", &loadMemory);        
        loadMemory.pop();
        loadMemory.push();
        gameState->meshes[3] = getMeshFromFile("floor2.vsmf", &loadMemory);
        loadMemory.pop();


        loadMemory.push();
        gameState->font = loadFontAndTexture("boldkeys32.fnt","boldkeys32.png",&loadMemory,gameState->assetMemory.get(getFontMemorySize("boldkeys32.fnt")));
        loadMemory.pop();

        gameState->testMaterial = createTestMaterial(gameState->hyperShader,gameState->mainTexture,&gameState->assetMemory);
        gameState->debugMaterial = createTestMaterial(gameState->debugShader,gameState->mainTexture,&gameState->assetMemory);
        gameState->material2d = create2dMaterial(gameState->shader2d,&gameState->assetMemory);

        gameState->objectList.objects = ARENA_GET_ARRAY(defMemory->memory, ProcObject, 20000);
        gameState->objectList.objectsMax = 20000;
        gameState->objectList.objectsCount = 0;        
        gameState->physicsSystem = createPhysicsSystem(&defMemory->memory, 40000, 40000, 10, 10);
        generateScene(1234, &gameState->objectList,&gameState->physicsSystem);        
        // PHYSICS
        
        gameState->player.physics = addPhysicsObject(&gameState->physicsSystem, getPlayerPhysicsObject());

        gameState->loaded = true;
        gameState->imguiState.ready = false;
        gameState->debugMode = false;

    }
    gameState->frameMemory.clear();
    
    ImGuiIO& io = beginImgui(input, &gameState->imguiState);
    ImGui::NewFrame();  

    F32 t = 1/60.0f;
    
    updatePlayer(&gameState->player, input, t, gameState->debugMode);        
    runPhysics(&gameState->physicsSystem, t);
    updatePlayerPostPhysics(&gameState->player, &gameState->physicsSystem);

    // RENDERER
    // Move renderer stuff into other 
    Vector2 screenSize = {(F32)input->windowWidth, (F32)input->windowHeight};

    Renderer2d renderer2d = createRenderer2d(gameState->frameMemory.get(10000000), 10000000, 20000, gameState->material2d);
    renderer2d.setScreenSize(screenSize);

    gameState->renderer = createSimpleRenderer(gameState->frameMemory.get(15000000), 15000000, 25000);
    gameState->renderer.backgroundColour = {0.21763764f*0.6f,0.21763764f*0.6f,0.6f};
    gameState->renderer.lightDirection = {0.7f,-0.6f,0.5f};
    gameState->renderer.screenSize = screenSize;
    Camera3d camera = getPlayerCamera(&gameState->player,screenSize);
    gameState->renderer.camera = camera;
    // gameState->renderer.renderWorld();
    

    for(int i = 0; i < gameState->objectList.objectsCount; i++) {
     Matrix4 drawMatrix = getModelMatrixForProcObject(gameState->objectList.objects[i]);   
     graphics_Mesh* mesh = gameState->meshes[gameState->objectList.objects[i].type];   
     Vector3 colour = {0};
     if (gameState->debugMode) {
        gameState->renderer.addMeshMaterialRenderObject(mesh, gameState->debugMaterial, &drawMatrix, colour);
     } else {
        gameState->renderer.addMeshMaterialRenderObject(mesh, gameState->testMaterial, &drawMatrix, colour);        
     }
    }

    gameState->renderer.renderSimple();

    if (BUTTON_WAS_PRESSED(input->k_g)) {
        gameState->objectList.objectsCount = 0;        
        generateScene(1234, &gameState->objectList, &gameState->physicsSystem);
    }
    
    if (BUTTON_WAS_PRESSED(input->k_r)) {
        gameState->objectList.objectsCount = 0;        
        generateScene(getClockTime(), &gameState->objectList, &gameState->physicsSystem);
    }

    if (BUTTON_WAS_PRESSED(input->k_m)) {
        gameState->debugMode = !gameState->debugMode;
    }

    // Draw UI:
    Quad quad = math_getAxisAlignedQuad({10.0f,10.0f, 200.0f,200.0f});
    Rect rect = {0.0f,0.0f,1.0f,1.0f};
    Vector4 colour = {1.0f, 1.0f, 1.0f, 1.0f};
    pushText(&renderer2d, gameState->font, {10.0f, 60.0f}, 32.0f, colour, "DEMO");
    // pushText(&renderer2d, gameState->font, {10.0f, 30.0f}, 32.0f, colour, "PRESS 1, 2, 3 TO CHANGE DEFAULT SETTINGS");
    
    renderer2d.render2d();

    ImGui::Text("Enabled: %d", enabled);
    ImGui::Text("Disabled: %d", disabled);
    glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);    
    ImGui::Render();


}
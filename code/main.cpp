#define MAIN_FILE

#include "engine/engine.cpp"

#include "renderer/graphics.h"

#include "renderer/opengl/graphics_opengl.cpp"
//#include "renderer/directx/graphics_directx11.cpp"

// #include "engine/text.cpp"
#include "engine/assets.cpp"

#include "game/world_renderer.cpp"
#include "game/world_asset_creation.cpp"
#include "game/game_assets.cpp"
#include "game/proc_scene.cpp"
#include "game/proc_gen.cpp"
#include "game/player.cpp"

#include "debug/imgui.cpp"

struct GameState {
    bool loaded;

    GameAssets assets;

    Player player;
    
    ProcObjectList objectList;
    PhysicsSystem physicsSystem;   

    ImguiGLState imguiState;

    WorldRenderer* renderer;

    bool debugMode;
};

CORE_LOOP {
    GameState* gameState = BEGIN_CORE_LOOP(GameState);

    if (!gameState->loaded) {   
        // TODO: this should probably be in the platform code
        graphics_init();

        loadAssets(&gameState->assets);

        gameState->objectList.objects = (ProcObject*) malloc(sizeof(ProcObject)*20000);
        gameState->objectList.objectsMax = 20000;
        gameState->objectList.objectsCount = 0;        
        
        gameState->physicsSystem = createPhysicsSystem(100, 40000, 500, 500);
        gameState->player.physics = addPhysicsObject(&gameState->physicsSystem, getPlayerPhysicsObject());
        
        generateScene(1234, &gameState->objectList);      
        generateStaticPhysicsFromProcObjectList(&gameState->objectList, &gameState->physicsSystem);
        
        gameState->renderer = createWorldRenderer(0x04000000); // 33554432 bytes

        gameState->loaded = true;
        gameState->imguiState.ready = false;
        gameState->debugMode = false;
    }
    F32 t = 1/60.0f;
    
    updatePlayer(&gameState->player, input, t, gameState->debugMode);        
    runPhysics(&gameState->physicsSystem, t);
    updatePlayerPostPhysics(&gameState->player, &gameState->physicsSystem);


    Vector2 screenSize = {(F32)input->windowWidth, (F32)input->windowHeight};
    Camera3d camera = getPlayerCamera(&gameState->player,screenSize);
    gameState->renderer->initFrame(camera.viewMatrix, camera.projectionMatrix, screenSize);
    renderProcObjectListScene(&gameState->objectList, gameState->renderer, &gameState->assets);
    gameState->renderer->render();

    // Debug buttons
    if (BUTTON_WAS_PRESSED(input->k_g)) {
        gameState->objectList.objectsCount = 0;        
        generateScene(1234, &gameState->objectList);
        generateStaticPhysicsFromProcObjectList(&gameState->objectList, &gameState->physicsSystem);
    }
    
    if (BUTTON_WAS_PRESSED(input->k_r)) {
        gameState->objectList.objectsCount = 0;        
        generateScene(getClockTime(), &gameState->objectList);
        generateStaticPhysicsFromProcObjectList(&gameState->objectList, &gameState->physicsSystem);
    }

    if (BUTTON_WAS_PRESSED(input->k_m)) {
        gameState->debugMode = !gameState->debugMode;
    }


    // Renderer2d renderer2d = createRenderer2d(gameState->frameMemory.get(10000000), 10000000, 20000, gameState->material2d);
    // renderer2d.setScreenSize(screenSize);

    // // Draw UI:
    // Quad quad = math_getAxisAlignedQuad({10.0f,10.0f, 200.0f,200.0f});
    // Rect rect = {0.0f,0.0f,1.0f,1.0f};
    // Vector4 colour = {1.0f, 1.0f, 1.0f, 1.0f};
    // pushText(&renderer2d, gameState->font, {10.0f, 60.0f}, 32.0f, colour, "DEMO");
    // // pushText(&renderer2d, gameState->font, {10.0f, 30.0f}, 32.0f, colour, "PRESS 1, 2, 3 TO CHANGE DEFAULT SETTINGS");
    
    // renderer2d.render2d();

    // ImGuiIO& io = beginImgui(input, &gameState->imguiState);
    // ImGui::NewFrame();  
    // ImGui::Text("Enabled: %d", enabled);
    // ImGui::Text("Disabled: %d", disabled);
    // glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);    
    // ImGui::Render();

}
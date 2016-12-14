#define MAIN_FILE

#include "engine/engine.cpp"

#include "renderer/graphics.h"

#include "renderer/opengl/graphics_opengl.cpp"
//#include "renderer/directx/graphics_directx11.cpp"

#include "engine/assets.cpp"

#include "game/world_renderer.cpp"
#include "game/world_asset_creation.cpp"
#include "game/ui_renderer.cpp"

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
    Renderer2d* renderer2d;

    bool flightMode;
    bool ui;

    F32 timeSinceBeginning;
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
        gameState->renderer2d = createRenderer2d(0x04000000, 10000, gameState->assets.material2d);

        gameState->loaded = true;
        gameState->imguiState.ready = false;
        gameState->flightMode = false;
        gameState->ui = true;
        gameState->timeSinceBeginning = 0.0f;


    }
    input->lockMouse = true;
    F32 t = 1/60.0f;
    gameState->timeSinceBeginning += t;
    updatePlayer(&gameState->player, input, t, gameState->flightMode);        
    runPhysics(&gameState->physicsSystem, t);
    updatePlayerPostPhysics(&gameState->player, &gameState->physicsSystem);


    Vector2 screenSize = {(F32)input->windowWidth, (F32)input->windowHeight};
    Camera3d camera = getPlayerCamera(&gameState->player,screenSize);
    gameState->renderer->initFrame(camera.viewMatrix, camera.projectionMatrix, screenSize);
    renderProcObjectListScene(&gameState->objectList, gameState->renderer, &gameState->assets, gameState->timeSinceBeginning);
    gameState->renderer->render();

    // Modify buttons

    if (BUTTON_WAS_PRESSED(input->k_esc)) {
        input->quit = true;
    }
    
    if (BUTTON_WAS_PRESSED(input->k_r)) {
        gameState->objectList.objectsCount = 0;        
        generateScene(getClockTime(), &gameState->objectList);
        generateStaticPhysicsFromProcObjectList(&gameState->objectList, &gameState->physicsSystem);
    }

    if (BUTTON_WAS_PRESSED(input->k_e)) {
        gameState->flightMode = !gameState->flightMode;
    }

    if (BUTTON_WAS_PRESSED(input->k_f)) {
        input->fullscreenToggle = true;
    }

    if (BUTTON_WAS_PRESSED(input->k_u)) {
        gameState->ui = !gameState->ui;
    }

    if (gameState->ui) {
        gameState->renderer2d->initFrame(screenSize);

        // Draw UI:
        Quad quad = math_getAxisAlignedQuad({10.0f,10.0f, 200.0f,200.0f});
        Rect rect = {0.0f,0.0f,1.0f,1.0f};
        Vector4 colour = {1.0f, 1.0f, 1.0f, 1.0f};
        if (!gameState->flightMode) {
            pushText(gameState->renderer2d, gameState->assets.font, {10.0f, 30.0f}, 32.0f, colour, "WASD + Mouse to Move. Space to jump. Press E to enter flying mode");
        } else {
            pushText(gameState->renderer2d, gameState->assets.font, {10.0f, 30.0f}, 32.0f, colour, "WASD + Mouse to Move. Hold Shift to move faster. Press E to exit flying mode");
        }
        pushText(gameState->renderer2d, gameState->assets.font, {10.0f, 60.0f}, 32.0f, colour, "Press R to regenerate the scene");
        pushText(gameState->renderer2d, gameState->assets.font, {10.0f, 90.0f}, 32.0f, colour, "Press ESC to exit. Press F to toggle fullscreen. Press U to show/hide this text");
        // pushText(&renderer2d, gameState->font, {10.0f, 30.0f}, 32.0f, colour, "PRESS 1, 2, 3 TO CHANGE DEFAULT SETTINGS");
        
        gameState->renderer2d->render2d();
    }

    // ImGuiIO& io = beginImgui(input, &gameState->imguiState);
    // ImGui::NewFrame();  
    // ImGui::Text("Enabled: %d", enabled);
    // ImGui::Text("Disabled: %d", disabled);
    // glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);    
    // ImGui::Render();

}
// Assets
// NOTE: Since there is a minimal number of assets, I have decided not to create/use any asset manager for this game

struct GameAssets {
    graphics_Texture* mainTexture;
    graphics_Shader* hyperShader;
    //graphics_Shader* debugShader;
    graphics_Shader* shader2d;    
    graphics_Mesh* meshes[4];
    graphics_Material* hyperMaterial;
    //graphics_Material* debugMaterial;
    graphics_Material* material2d;
    Font* font;
    memory_Arena assetMemory;
};

void loadAssets(GameAssets* assets) {
    assets->assetMemory = memory_createArena(malloc(20000000), 20000000);
    memory_Stack loadMemory;    
    void* loadMemoryBasePointer = malloc(10000000);
    loadMemory.init(10000000, loadMemoryBasePointer);

    loadMemory.push();
    assets->mainTexture = getTextureFromFile("maintexture.png", &loadMemory);
    loadMemory.pop();
    
    loadMemory.push();
    assets->hyperShader = getShaderFromFile("shader.vert", "shader.frag", &loadMemory);
    loadMemory.pop();

    // loadMemory.push();
    // assets->debugShader = getShaderFromFile("shaderdebug.vert", "shaderdebug.frag", &loadMemory);
    // loadMemory.pop();

    loadMemory.push();
    assets->shader2d = getShader2dFromFile("shader2d.vert", "shader2d.frag", &loadMemory);
    loadMemory.pop();

    loadMemory.push();
    assets->meshes[0] = getMeshFromFile("block.vsmf", &loadMemory);        
    loadMemory.pop();
    loadMemory.push();
    assets->meshes[1] = getMeshFromFile("lamp.vsmf", &loadMemory);        
    loadMemory.pop();
    loadMemory.push();
    assets->meshes[2] = getMeshFromFile("floor.vsmf", &loadMemory);        
    loadMemory.pop();
    loadMemory.push();
    assets->meshes[3] = getMeshFromFile("floor2.vsmf", &loadMemory);
    loadMemory.pop();


    loadMemory.push();
    assets->font = loadFontAndTexture("boldkeys32.fnt","boldkeys32.png",&loadMemory,assets->assetMemory.get(getFontMemorySize("boldkeys32.fnt")));
    loadMemory.pop();

    assets->hyperMaterial = createWorldRendererMaterial(assets->hyperShader,assets->mainTexture,&assets->assetMemory);
    assets->material2d = create2dMaterial(assets->shader2d,&assets->assetMemory);
    // assets->debugMaterial = createTestMaterial(assets->debugShader,assets->mainTexture,&assets->assetMemory);
    
    free(loadMemoryBasePointer);
}
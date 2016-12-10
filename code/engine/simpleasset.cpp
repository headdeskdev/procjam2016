// Potential future asset management. Probably just a system for a more jam-style game than a big project though.

union simpleasset_Asset {
    GraphicsTexture* texture;    
    GraphicsShader* shader;    
    GraphicsMesh* mesh;
    GraphicsMaterial* material;
    Font* font;
}

enum simpleasset_AssetType {
    simpleasset_TEXTURE,
    simpleasset_SHADER,
    simpleasset_MESH,
    simpleasset_MATERIAL,
    simpleasset_FONT
}

struct simpleasset_AssetTag {    
    U32 tag;
    simpleasset_AssetType type;
};

struct simpleasset_AssetLibrary {
    simpleasset_AssetTag* tags;
    simpleasset_Asset* assets;
    U32 tableSize;

    MemoryArena assetMemory;
    
    void loadTexture(U32 tag, char* file) {

    }
    void loadShader(U32 tag, char* vertexFile, char* fragmentFile) {

    }
    void loadMesh(U32 tag, char* file) {

    }
    void loadFont(U32 tag, char* fontFile, char* textureFile) {

    }

    GraphicsMaterial* createMaterial(U32 tag, GraphicsShader* shader, U16 parameterMapSize, U16 defaultParameterSize) {
        GraphicsMaterial* material = ARENA_GET_STRUCT(assetMemory, GraphicsMaterial);
        material->defaultParameters = {0};
        material->defaultParameters.parameters = ARENA_GET_ARRAY(assetMemory, GraphicsShaderParameter, 1);
        material->parameterMap.in = ARENA_GET_ARRAY(assetMemory, I32, 5);
        material->parameterMap.out = ARENA_GET_ARRAY(assetMemory, I32, 5);
        simpleasset_Asset asset; asset->material = material;
        simpleasset_AssetTag assetTag = {tag, simpleasset_MATERIAL};
        addAsset(asset,assetTag);
    }

    GraphicsTexture* getTexture()
};

void simpleasset_CreateAssetLibrary(void* assetMemory, U32 assetMemorySize) {
    simpleasset_AssetLibrary
}
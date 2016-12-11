#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_NO_STDIO
#include "../engine/stb/stb_image.h"

struct RawMeshData {
    U32 numVertices;
    U32 numTriangles;
    F32* vertexPositions;
    F32* vertexNormals;
    F32* vertexUVs;
    U32* triangles;  
};

struct UVData {
    F32 minU;
    F32 minV;
    F32 maxU;
    F32 maxV;   
};

struct GeneratedCube {
    AABB box;
    union {
        UVData uvData[6];
        struct {
            UVData minYuv;
            UVData maxZuv;
            UVData maxYuv;
            UVData minZuv;
            UVData minXuv;
            UVData maxXuv;
        };
    };  
};

U32 getRequiredMemoryForCubes(U32 numCubes) {
    return numCubes*(192*sizeof(F32) + 36*sizeof(U32));   
}

static const F32 cubeVertexPositions[24*3]= {-1.0,-1.0,-1.0,   -1.0,-1.0, 1.0,
                                      1.0,-1.0,-1.0,    1.0,-1.0, 1.0,
                                     -1.0,-1.0, 1.0,   -1.0, 1.0, 1.0,
                                      1.0,-1.0, 1.0,    1.0, 1.0, 1.0,
                                     -1.0, 1.0, 1.0,   -1.0, 1.0,-1.0,
                                      1.0, 1.0, 1.0,    1.0, 1.0,-1.0,
                                     -1.0, 1.0,-1.0,   -1.0,-1.0,-1.0,
                                      1.0, 1.0,-1.0,    1.0,-1.0,-1.0,
                                     -1.0,-1.0,-1.0,   -1.0, 1.0,-1.0,
                                     -1.0,-1.0, 1.0,   -1.0, 1.0, 1.0,
                                      1.0,-1.0, 1.0,    1.0, 1.0, 1.0,
                                      1.0,-1.0,-1.0,    1.0, 1.0,-1.0};

static const F32 cubeVertexNormals[24*3]= {0.0,-1.0,0.0,0.0,-1.0,0.0,
                                   0.0,-1.0,0.0,0.0,-1.0,0.0,
                                   0.0,0.0, 1.0,0.0,0.0, 1.0,
                                   0.0,0.0, 1.0,0.0,0.0, 1.0,
                                   0.0, 1.0,0.0,0.0, 1.0,0.0,
                                   0.0, 1.0,0.0,0.0, 1.0,0.0,
                                   0.0,0.0,-1.0,0.0,0.0,-1.0,
                                   0.0,0.0,-1.0,0.0,0.0,-1.0,
                                   -1.0,0.0,0.0,-1.0,0.0,0.0,
                                   -1.0,0.0,0.0,-1.0,0.0,0.0,
                                    1.0,0.0,0.0, 1.0,0.0,0.0,
                                    1.0,0.0,0.0, 1.0,0.0,0.0};

static const F32 cubeVertexUVs[24*2]={0.0,0.0,0.0,1.0,1.0,0.0,1.0,1.0,
                               0.0,0.0,0.0,1.0,1.0,0.0,1.0,1.0,
                               0.0,0.0,0.0,1.0,1.0,0.0,1.0,1.0,
                               0.0,0.0,0.0,1.0,1.0,0.0,1.0,1.0,
                               0.0,0.0,0.0,1.0,1.0,0.0,1.0,1.0,
                               0.0,0.0,0.0,1.0,1.0,0.0,1.0,1.0};
                                                   
                                                   
U32 cubeTriangles[12*3] = {1,2,3,2,1,0,5,6,7,6,5,4,9,10,11,10,9,8,
                                13,14,15,14,13,12,17,18,19,18,17,16,
                                21,22,23,22,21,20};

static graphics_Mesh* genMeshFromCubes(GeneratedCube* cubes, U32 numCubes, void* memory) {
    // Potentially remove redundant faces
    // Will increase gen time but decrease runtime
    // Can also return unused memory

    F32* meshVertexPositions = (F32*) memory;
    F32* meshVertexNormals = &meshVertexPositions[72*numCubes];
    F32* meshVertexUVs = &meshVertexNormals[72*numCubes];
    U32* meshTriangles = (U32*) (&meshVertexUVs[48*numCubes]);

    for (int i = 0; i < numCubes; i++) {
        GeneratedCube* cube = &cubes[i];
        Vector3 cubeExtent = (cube->box.max - cube->box.min) * 0.5;
        Vector3 offset = cube->box.max - cubeExtent;
        for (int v = 0; v < 24; v++) {

            meshVertexPositions[72*i+3*v+0] = cubeVertexPositions[3*v+0]*cubeExtent.x + offset.x;
            meshVertexPositions[72*i+3*v+1] = cubeVertexPositions[3*v+1]*cubeExtent.y + offset.y;
            meshVertexPositions[72*i+3*v+2] = cubeVertexPositions[3*v+2]*cubeExtent.z + offset.z;

            meshVertexNormals[72*i+3*v+0] = cubeVertexNormals[3*v+0];
            meshVertexNormals[72*i+3*v+1] = cubeVertexNormals[3*v+1];
            meshVertexNormals[72*i+3*v+2] = cubeVertexNormals[3*v+2];

            // TODO: fix uvs
            meshVertexUVs[48*i+2*v+0] = cubeVertexUVs[2*v+0];
            meshVertexUVs[48*i+2*v+1] = cubeVertexUVs[2*v+1];
        }
        for (int t = 0; t < 12; t++) {
            meshTriangles[36*i+3*t+0] = cubeTriangles[3*t+0]+24*i;
            meshTriangles[36*i+3*t+1] = cubeTriangles[3*t+1]+24*i;
            meshTriangles[36*i+3*t+2] = cubeTriangles[3*t+2]+24*i;
        }
    }  
    

    graphics_VertexAttribute attributes[3] = {{meshVertexPositions,3},{meshVertexNormals,3},{meshVertexUVs,2}};
    graphics_Mesh* mesh = graphics_createMesh(numCubes*24, numCubes*12, attributes, 3, meshTriangles);
    return mesh;
}

struct BitmapData {
    U8* data;
    U32 width;
    U32 height;
};

static inline platform_FileInfo loadIntoTempMemory(char* filename, memory_Stack* memory) {
    U32 size = getFileSize(filename);
    void* buffer = memory->get(size);    
    return readEntireFile(filename, buffer, size);      
}

static BitmapData loadBitmapData(char* filename, memory_Stack* memory) {
    int channels = 4;
    int width = 0;
    int height = 0;       
    memory->push();
    platform_FileInfo file = loadIntoTempMemory(filename,memory);
    U8* imgdata = stbi_load_from_memory((U8*)file.data, file.size, &width, &height, &channels, 4);
    BitmapData bitmap = {imgdata, (U32) width, (U32) height};
    memory->pop();
    return bitmap;
}

static void freeBitmapData(BitmapData bitmap) {
    stbi_image_free(bitmap.data);
}

static graphics_Texture* getTextureFromFile(char* filename, memory_Stack* memory) {
    BitmapData bitmap = loadBitmapData(filename, memory);

    graphics_TextureData textureBufferData = {0};
    textureBufferData.data = bitmap.data;
    textureBufferData.size[0] = bitmap.width; textureBufferData.size[1] = bitmap.height;
    textureBufferData.dimensions = 2;
    textureBufferData.input = GRAPHICS_INPUT_FOUR_U8;
    textureBufferData.storage = GRAPHICS_STORAGE_RGBA;
    textureBufferData.bufferType = GRAPHICS_BUFFER_TEXTURE;

    graphics_TextureParameters parameters = {0.0f};
    parameters.minFilter = GRAPHICS_FILTERING_LINEAR_MIPMAP_LINEAR;
    parameters.magFilter = GRAPHICS_FILTERING_LINEAR;

    return graphics_createTexture(&textureBufferData, &parameters);
}

graphics_Shader* getShaderFromFile(char* vertexfile, char* fragmentfile, memory_Stack* assetMemory)
{
    graphics_Shader* shader;
    assetMemory->push(); {
        platform_FileInfo vertexShader = readEntireFile(vertexfile,assetMemory->get(4194305),4194304);
        platform_FileInfo fragmentShader = readEntireFile(fragmentfile,assetMemory->get(4194305),4194304);
        ((char*)fragmentShader.data)[fragmentShader.size] = 0;
        ((char*)vertexShader.data)[vertexShader.size] = 0;
        graphics_ShaderData data = {(char*)vertexShader.data,(char*)fragmentShader.data};
        const char* names[3] = {"position","normal","uv"};    
        graphics_ShaderAttributes attributes = {names, 3};
        shader = graphics_createShader(&data, attributes);
    } assetMemory->pop();
    return shader;
}

graphics_Shader* getShader2dFromFile(char* vertexfile, char* fragmentfile, memory_Stack* assetMemory)
{
    graphics_Shader* shader;
    assetMemory->push(); {
        platform_FileInfo vertexShader = readEntireFile(vertexfile, assetMemory->get(4194305), 4194304);
        platform_FileInfo fragmentShader = readEntireFile(fragmentfile, assetMemory->get(4194305), 4194304);
        ((char*)fragmentShader.data)[fragmentShader.size] = 0;
        ((char*)vertexShader.data)[vertexShader.size] = 0;
        graphics_ShaderData data = { (char*)vertexShader.data,(char*)fragmentShader.data };
        const char* names[3] = { "position","uv","colour" };
        graphics_ShaderAttributes attributes = { names, 3 };
        shader = graphics_createShader(&data, attributes);
    } assetMemory->pop();
    return shader;
}

struct FontGlyph {  
    Rect textureRect;
    Rect normalizedDrawRect;    
};

struct Font {
    graphics_Texture* textureHandle; // TODO: do we want fonts to have a texture handle or ?
    U32 numGlyphs;
    FontGlyph* glyphs;
    F32* kerning;
    U32 highestCodepointPlusOne;
    U16* unicodeMap;
    U32 fallbackGlyph;
    F32 lineSpacing;
    F32 visibleHeight;

    // TODO: handle out of bounds codePoints
    inline F32 getKerning(U32 codePointA, U32 codePointB) {
        return kerning[unicodeMap[codePointA]*numGlyphs + unicodeMap[codePointB]];
    }

    inline FontGlyph getGlyph(U32 codePoint) {
        return glyphs[unicodeMap[codePoint]];
    }
};


static U32 getFontMemorySize(U32 numGlyphs, U32 highestCodepointPlusOne) {    
    U32 size = sizeof(Font);
    size += sizeof(FontGlyph)*numGlyphs;
    size += sizeof(F32)*numGlyphs*numGlyphs;
    size += sizeof(U16)*highestCodepointPlusOne;
    return size;
}

static U32 getFontMemorySize(char* fontFile) {
  Font font;
  readEntireFile(fontFile, &font, sizeof(Font));
  return getFontMemorySize(font.numGlyphs, font.highestCodepointPlusOne);
}

static void setFontArrayMemory(memory_Arena& assetMemoryArena, Font* font) {
    font->glyphs = ARENA_GET_ARRAY(assetMemoryArena, FontGlyph, font->numGlyphs);
    font->kerning = ARENA_GET_ARRAY(assetMemoryArena, F32, font->numGlyphs*font->numGlyphs);
    font->unicodeMap = ARENA_GET_ARRAY(assetMemoryArena, U16, font->highestCodepointPlusOne);
}

static Font* allocateFont(U32 numGlyphs, U32 highestCodepointPlusOne, void* memory) {
    memory_Arena assetMemoryArena = memory_createArena(memory, getFontMemorySize(numGlyphs, highestCodepointPlusOne));  
    Font* font = ARENA_GET_STRUCT(assetMemoryArena, Font);
    font->numGlyphs = numGlyphs;
    font->highestCodepointPlusOne = highestCodepointPlusOne;
    setFontArrayMemory(assetMemoryArena, font);
    return font;
}

static Font* loadFontAndTexture(char* fontFile, char* bitmapFile, memory_Stack* loadMemory, void* assetMemory) {    
    platform_FileInfo file = readEntireFile(fontFile, assetMemory, getFileSize(fontFile));
    memory_Arena assetMemoryArena = memory_createArena(assetMemory, file.size);         
    Font* font = ARENA_GET_STRUCT(assetMemoryArena, Font);
    font->textureHandle = getTextureFromFile(bitmapFile, loadMemory);
    setFontArrayMemory(assetMemoryArena, font);
    return font;
}

// TODO: handle unicode properly
static U32 getNumChars(char* text) {
    U32 numChars = 0;
    while (*(text++)) {     
        numChars++;
    }
    return numChars;
}

static U32 getNumGlyphs(char* text) {
    return getNumChars(text);
}

static F32 getTextWidth(Font* font, F32 size, char* text) {
    F32 x = 0;
    U32 numGlyphs = getNumGlyphs(text);
    for (int i = 0; i < numGlyphs; i++) {       
        if (i != 0) {x += size*font->getKerning(text[i-1], text[i]);}
    }
    x += size*font->getKerning(text[numGlyphs-1], 0);
    return x;
}

static U32 getLineCutoffPoint(Font font, F32 size, char* text, F32 width, char* cutoffChars) {
  return 0.0;
}

inline static U32 getLineCutoffPoint(Font font, F32 size, char* text, F32 width) {
  return getLineCutoffPoint(font, size, text, width, "");
}


graphics_Material* createWorldRendererMaterial(graphics_Shader* shader, graphics_Texture* texture, memory_Arena* assetMemory) {
    graphics_Material* material = ARENA_GET_STRUCT(*assetMemory, graphics_Material);
    material->defaultParameters = {0};
    material->defaultParameters.parameters = ARENA_GET_ARRAY(*assetMemory, graphics_ShaderParameter, 1);
    graphics_ShaderParameter param = {SHADER_PARAMETER_MAIN_TEXTURE, GRAPHICS_SHADER_PARAMETER_TEXTURE, 1, texture};
    material->defaultParameters.addSortedParameter(param);

    material->parameterMap.in = ARENA_GET_ARRAY(*assetMemory, I32, 10);
    material->parameterMap.out = ARENA_GET_ARRAY(*assetMemory, I32, 10);
    const char* names[10] = {"mainTexture",                  "ambientColor",
                            "directionalViewDirection",   "directionalColor",
                            "projectionMatrix",           "modelViewMatrix",
                            "normalModelViewMatrix",      "clusterBufferTexture",
                            "clusterItemBufferTexture",   "lightBufferTexture"};
    U32 refs[10] = {SHADER_PARAMETER_MAIN_TEXTURE,          SHADER_PARAMETER_AMBIENT,
                   SHADER_PARAMETER_DIRECTIONAL_DIRECTION,  SHADER_PARAMETER_DIRECTIONAL_COLOR,
                   SHADER_PARAMETER_PROJECTION,             SHADER_PARAMETER_MODEL_VIEW,
                   SHADER_PARAMETER_NORMAL_MODEL_VIEW,      SHADER_PARAMETER_CLUSTER_BUFFER,
                   SHADER_PARAMETER_CLUSTER_ITEM_BUFFER,    SHADER_PARAMETER_LIGHT_BUFFER};
    graphics_MaterialAttributes attributes = {names, refs, 10};
    graphics_initMaterial(material, shader, attributes);
    return material;
}
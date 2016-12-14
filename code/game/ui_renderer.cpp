enum UIRendererShaderParameter {
    SHADER_PARAMETER_SCREEN_VIEW_2D,
    SHADER_PARAMETER_TEXTURE,
};

struct Renderer2d {
    graphics_RenderObject* objects;
    U32 objectsCount;
    U32 objectsMax;

    memory_Arena memory;
    Vector2 screenSize;
    Matrix3 globalMatrix;
    graphics_Material* material2d;

    void initFrame(Vector2 screenSizeVector) {
        memory.clear();
        screenSize = screenSizeVector;
        TransformMatrix2d screenTransform = {2.0f/screenSize.x,0.0f,0.0f,-2.0f/screenSize.y,-1.0f,1.0f};
        TransformMatrix2d userTransform = {1.0f,0.0f,0.0f,1.0f};
        globalMatrix = screenTransform.toMatrix3();
        objects = ARENA_GET_ARRAY(memory, graphics_RenderObject, objectsMax);
        objectsCount = 0;
    }

    void render2d() {
        graphics_RenderPass pass = {0};
        pass.alphaBlending = true;
        pass.depthCheck = false;
        pass.cullFace = false;
        pass.viewportWidth = screenSize.x;
        pass.viewportHeight = screenSize.y;
        pass.globalParameters.parameters = ARENA_GET_ARRAY(memory, graphics_ShaderParameter, 1);        
        Matrix3* newMatrix = ARENA_GET_STRUCT(memory, Matrix3);
        *newMatrix = globalMatrix;
        graphics_ShaderParameter projectionParameter = {SHADER_PARAMETER_SCREEN_VIEW_2D,GRAPHICS_SHADER_PARAMETER_MAT3,1};
        projectionParameter.matrix3Param = newMatrix;
        pass.globalParameters.addSortedParameter(projectionParameter);
        graphics_render(&pass, objects, objectsCount);
    }

    void addTexturedQuad(graphics_Texture* texture, Quad quad, Rect textureRect, Vector4 colour) {

        graphics_Quad* gquad = ARENA_GET_STRUCT(memory, graphics_Quad);
        gquad->quad = quad;
        gquad->textureCoord = textureRect;
        gquad->colour = colour;

        graphics_ShaderParameterSortedList parameters = {0};
        parameters.parameters = ARENA_GET_ARRAY(memory, graphics_ShaderParameter, 1);
        graphics_ShaderParameter param = {SHADER_PARAMETER_TEXTURE, GRAPHICS_SHADER_PARAMETER_TEXTURE, 1, texture};
        parameters.addSortedParameter(param);

        graphics_QuadMaterial* quadMaterial = ARENA_GET_STRUCT(memory, graphics_QuadMaterial);
        quadMaterial->material = material2d;
        quadMaterial->quad = gquad;      

        graphics_RenderObject newObject = {RENDER_OBJECT_QUAD_MATERIAL};
        newObject.quadMaterial = quadMaterial;
        newObject.objectParameters = parameters;

        objects[objectsCount++] = newObject;
    }
};



Renderer2d* createRenderer2d(U32 size, U32 maxObjects, graphics_Material* material) {
    Renderer2d* renderer2d = (Renderer2d*) malloc(sizeof(Renderer2d)+size);
    void* memory = renderer2d + 1;
    renderer2d->memory = memory_createArena(memory, size);    
    renderer2d->material2d = material;
    renderer2d->objectsMax = maxObjects;
    return renderer2d;
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

graphics_Material* create2dMaterial(graphics_Shader* shader, memory_Arena* assetMemory) {
    graphics_Material* material = ARENA_GET_STRUCT(*assetMemory, graphics_Material);
    material->defaultParameters = {0};

    material->parameterMap.in = ARENA_GET_ARRAY(*assetMemory, I32, 2);
    material->parameterMap.out = ARENA_GET_ARRAY(*assetMemory, I32, 2);
    const char* names[2] = {"screenViewMatrix","textureMap"};
    U32 refs[2] = {SHADER_PARAMETER_SCREEN_VIEW_2D,SHADER_PARAMETER_TEXTURE};
    graphics_MaterialAttributes attributes = {names, refs, 2};
    graphics_initMaterial(material, shader, attributes);
    return material;
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

// TODO: where does this go
void concatStrings(char* stringA, char* stringB, char* buffer, U32 length) {
    int i = 0;
    while (i < length-1 && *stringA) {
        buffer[i++] = *(stringA++);
    }
    while (i < length-1 && *stringB) {
        buffer[i++] = *(stringB++);
    }
    buffer[i] = 0;
}

void concatStringsWithSpace(char* stringA, char* stringB, char* buffer, U32 length) {
    int i = 0;
    while (i < length-1 && *stringA) {
        buffer[i++] = *(stringA++);
    }
    buffer[i++] = ' ';
    while (i < length-1 && *stringB) {
        buffer[i++] = *(stringB++);
    }
    buffer[i] = 0;
}

// Uses RenderGroup2d + Assets to allow easy text rendering

// NOTE: position is from the baseline of the left of the first character

// TODO: should font + colour + size be a thing? what does that give us
static Vector2 pushText(Renderer2d* renderer, Font* font, Vector2 position,  F32 size, Vector4 colour, char* text) {    
    
    U32 numGlyphs = getNumGlyphs(text);
    
    F32 x = 0.0f;
    F32 y = 0.0f;
    for (int i = 0; i < numGlyphs; i++) {       
        if (i != 0 && text[i-1] != '\n') {x += size*font->getKerning(text[i-1], text[i]);}
        if (text[i] == '\n') {
            y += font->lineSpacing * size;
            x = 0.0f;
        } else {
            FontGlyph glyph = font->getGlyph(text[i]);
            Vector2 currentPosition = {position.x + x, position.y + y};
            Rect drawQuad = {glyph.normalizedDrawRect.min * size + (currentPosition),
                             glyph.normalizedDrawRect.max * size + (currentPosition)};

            renderer->addTexturedQuad(font->textureHandle, math_getAxisAlignedQuad(drawQuad), glyph.textureRect, colour);           
        }
    }       
    x += size*font->getKerning(text[numGlyphs-1], 0);
    return {x,y};
}

static void writeInt(char* number, I32 integer) {
    int c = 0;
    if (integer < 0) {
        number[c++] = '-';
        integer = -integer;
    } else if (integer == 0) {
        number[c++] = '0';
        number[c] = 0;
        return;
    }
    int numChars = 0;
    int i = integer;
    while (i > 0) {
        numChars++; 
        i = i / 10;
    }
    number[c+numChars] = 0;
    while (integer > 0) {
        numChars--;
        number[c+numChars] = '0' + (integer % 10);
        integer = integer / 10;
    }  
}

static Vector2 pushTextCentered(Renderer2d* renderer, Font* font, Rect box,  F32 size, Vector4 colour, char* text) {    
    F32 textWidth = getTextWidth(font,size,text);
    F32 textHeight = size * font->visibleHeight;
    Vector2 textPosition = {(box.max.x + box.min.x - textWidth)/2.0f,
                            (box.max.y + box.min.y + textHeight)/2.0f};
    return pushText(renderer, font, textPosition, size, colour, text);
}

// static pushText(RenderGroup2d* renderGroup, Font* font, F32 size, char* text, U32 numChars, F32 width, char* cutoffChars) {

// }




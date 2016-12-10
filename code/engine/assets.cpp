enum FileParseError {
    FILE_PARSE_ERROR_NONE,
    FILE_PARSE_ERROR_ANY
};

struct StringFileParser {    
    U32 pos;
    FileParseError error;
    char* fileString;
};

void readNewline(StringFileParser* parser) {
    char* file = parser->fileString;
    U32 pos = parser->pos;
    while(file[pos] == '\n' || file[pos] == '\r') {
        pos++;
    }
    parser->pos = pos;
}


U32 readUnsignedInteger(StringFileParser* parser) {
    U32 value = 0;
    bool didParseIntChar = true;    
    char* file = parser->fileString;
    U32 pos = parser->pos;

    while(didParseIntChar) {
        didParseIntChar = false;
        char currentChar = file[pos];
        I32 currentDigit = currentChar - 0x30;
        if (currentDigit >= 0 && currentDigit <= 9) {
            value = value * 10 + currentDigit;
            didParseIntChar = true; 
            pos++;              
        }

    }

    parser->pos = pos;
    return value;   
}

F32 readFloat(StringFileParser* parser) {
    char* file = parser->fileString;
    U32 pos = parser->pos;

    char* end = &file[pos];
    F32 value = strtod(&file[pos], &end);
    parser->pos = (U32)(end - &file[pos]) + pos;
    return value;
}

void ignoreChars(StringFileParser* parser, const char* chars, int charsLen) {
    char* file = parser->fileString;
    U32 pos = parser->pos;
    U32 matches = 1;
    while(matches) {
        matches = 0;
        for (int i = 0; i < charsLen; i++) {
            if (file[pos] == chars[i]) {
                pos++;
            }            
        }
    }
    parser->pos = pos;
}


static graphics_Mesh* constructVSMFMesh(platform_FileInfo file, memory_Stack* memory) {
    U32 numVertices = 0;
    U32 numTriangles = 0;
    StringFileParser parser= {0};
    parser.fileString = (char*) file.data;    
    parser.error = FILE_PARSE_ERROR_NONE;
    char* gapChars = " ,";

    numVertices = readUnsignedInteger(&parser);
    F32* vertexPositions = (F32*) memory->get(sizeof(F32)*3*numVertices);
    F32* vertexNormals = (F32*) memory->get(sizeof(F32)*3*numVertices);
    F32* vertexUVs = (F32*) memory->get(sizeof(F32)*2*numVertices);    
    ignoreChars(&parser,gapChars,2); 
    readNewline(&parser);    
    U32 vnum = 0;
    while(parser.error == FILE_PARSE_ERROR_NONE && vnum < numVertices) {     
        for (int i = 0; i<3; i++) {
            vertexPositions[i+vnum*3] = readFloat(&parser); ignoreChars(&parser,gapChars,2);        
        }
        for (int i = 0; i<2; i++) {
            vertexUVs[i+vnum*2] = readFloat(&parser); ignoreChars(&parser,gapChars,2);        
        }
        for (int i = 0; i<3; i++) {
            vertexNormals[i+vnum*3] = readFloat(&parser); ignoreChars(&parser,gapChars,2);        
        }
        readNewline(&parser);
        vnum++;
    }
    numTriangles = readUnsignedInteger(&parser);
    U32* triangles = (U32*) memory->get(sizeof(F32)*3*numTriangles);
    ignoreChars(&parser,gapChars,2); 
    readNewline(&parser);    
    U32 tnum = 0;
    while(parser.error == FILE_PARSE_ERROR_NONE && tnum < numTriangles) {     
        for (int i = 0; i<3; i++) {
            triangles[i + tnum * 3] = readUnsignedInteger(&parser); ignoreChars(&parser, gapChars, 2);
        }
        readNewline(&parser);
        tnum++;
    }

    graphics_VertexAttribute attributes[3] = {{vertexPositions,3},{vertexNormals,3},{vertexUVs,2}};
    return graphics_createMesh(numVertices, numTriangles, attributes, 3, triangles);
}

static graphics_Mesh* getMeshFromFile(char* file, memory_Stack* memory) {
    memory->push();
    U32 fsize = getFileSize(file);
    platform_FileInfo f = readEntireFile(file, memory->get(fsize+1), fsize);
    ((char*)f.data)[f.size] = 0;
        
    graphics_Mesh* mesh = constructVSMFMesh(f,memory);
    memory->pop();
    return mesh;
}
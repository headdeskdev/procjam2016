#include "../graphics.h"
//#include "../mmemory.h"
#define GL_LITE_IMPLEMENTATION
#include "gl_lite.h"

#include <stdlib.h>

struct graphics_Mesh {
    U32 trianglesCount;
    U32 reference;
    U32 objectBuffersCount;
    U32* objectBuffers;
};

struct graphics_Texture {
    U32 reference;
    U32 type;
    U32 buffer;
};

struct graphics_Shader {
    U32 reference;
    U32 vertex;
    U32 fragment;
};

// TODO: need some way to speed up constant global / material parameters

static void setMaterial(graphics_Material* material, graphics_ShaderParameterSortedList* global, graphics_ShaderParameterSortedList* object) {
    // TODO: Optimise this to only set object specific parameters if material is the same

    U32 currentParameters[3] = {0,0,0};
    graphics_ShaderParameterSortedList* parameterLists[3] = {&material->defaultParameters, global, object};

    glUseProgram(material->shader->reference);
    U32 currentTextureNum = 0;

    while (currentParameters[0] < parameterLists[0]->parameterCount || currentParameters[1] < parameterLists[1]->parameterCount ||
           currentParameters[2] < parameterLists[2]->parameterCount) {
        U32 listToUse = 0;
        U32 currentRef = 0xFFFFFFFF;
        for (int i = 0; i < 3; i++) {
            if (currentParameters[i] < parameterLists[i]->parameterCount) {
                if (parameterLists[i]->parameters[currentParameters[i]].reference <= currentRef) {
                    listToUse = i;
                    currentRef = parameterLists[i]->parameters[currentParameters[i]].reference;
                }
            }
        }

        graphics_ShaderParameter* parameterToUse = parameterLists[listToUse]->parameters + currentParameters[listToUse];

        for (int i = 0; i < 3; i++) {
            if (currentParameters[i] < parameterLists[i]->parameterCount) {
                graphics_ShaderParameter* parameter = parameterLists[i]->parameters + currentParameters[i];
                if (parameterToUse->reference == parameter->reference) {
                    currentParameters[i]++;
                }
            }
        }
        U32 location = material->parameterMap.getReference(parameterToUse->reference);
        switch(parameterToUse->type) {
            case GRAPHICS_SHADER_PARAMETER_TEXTURE:
                glActiveTexture(GL_TEXTURE0 + currentTextureNum);
                glBindTexture(parameterToUse->textureParam->type, parameterToUse->textureParam->reference);
                glUniform1i(location, currentTextureNum);
                currentTextureNum++;
                break;
            case GRAPHICS_SHADER_PARAMETER_FLOAT:
                glUniform1fv(location, parameterToUse->arrayCount, parameterToUse->floatParam);
                break;
            case GRAPHICS_SHADER_PARAMETER_VEC2:
                glUniform2fv(location, parameterToUse->arrayCount, parameterToUse->vector2Param->data);
                break;
            case GRAPHICS_SHADER_PARAMETER_VEC3:
                glUniform3fv(location, parameterToUse->arrayCount, parameterToUse->vector3Param->data);
                break;
            case GRAPHICS_SHADER_PARAMETER_VEC4:
                glUniform4fv(location, parameterToUse->arrayCount, parameterToUse->vector4Param->data);
                break;
            case GRAPHICS_SHADER_PARAMETER_MAT4:
                glUniformMatrix4fv(location, parameterToUse->arrayCount, false, parameterToUse->matrix4Param->data);
                break;
            case GRAPHICS_SHADER_PARAMETER_MAT3:
                glUniformMatrix3fv(location, parameterToUse->arrayCount, false, parameterToUse->matrix3Param->data);
                break;
        }
    }
}

static void renderMesh(graphics_Mesh* mesh) {
    glBindVertexArray(mesh->reference);
    glDrawElements(GL_TRIANGLES, mesh->trianglesCount*3, GL_UNSIGNED_INT, NULL);
}


graphics_RenderPassOutput graphics_render(graphics_RenderPass* pass, graphics_RenderObject* renderObjects, U32 renderObjectsCount) {
    // Set pass properties
    glViewport(0.0, 0.0, pass->viewportWidth, pass->viewportHeight);

    if (pass->depthCheck) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }

    if (pass->cullFace) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }

    if (pass->alphaBlending) {
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glDisable(GL_BLEND);
    }

    if (pass->writeDepth) {
        //glDepthMask(true);
    } else {
        //glDepthMask(false);
    }

    U32 clear = 0;
    if (pass->clearDepthEnabled) {
        clear = clear | GL_DEPTH_BUFFER_BIT;
    }
    if (pass->clearColourEnabled) {
        clear = clear | GL_COLOR_BUFFER_BIT;
        glClearColor(pass->clearColour.x, pass->clearColour.y , pass->clearColour.z,1.0);
    }
    if (clear) {
        glClear(clear);
    }


    for (int i = 0; i < renderObjectsCount; i++) {
        graphics_RenderObject* renderObject = renderObjects + i;
        switch(renderObject->type) {
            case RENDER_OBJECT_MESH_MATERIAL:
                setMaterial(renderObject->meshMaterial->material, &pass->globalParameters, &renderObject->objectParameters);
                renderMesh(renderObject->meshMaterial->mesh);
                break;
            default:
                break;
        }
    }

    // TODO: is this output actually something we will use?
    graphics_RenderPassOutput output;
    return output;

}

static U32 getInputFormatSize(graphics_InputFormat input) {
        switch(input) {
            case GRAPHICS_INPUT_SINGLE_U8:
                return 1;
                break;
            case GRAPHICS_INPUT_FOUR_U8:
                return 4;
                break;
            case GRAPHICS_INPUT_SINGLE_FLOAT:
                return 4;
                break;
            case GRAPHICS_INPUT_FOUR_FLOAT:
                return 16;
                break;
            case GRAPHICS_INPUT_SINGLE_UNSIGNED_16:
                return 2;
                break;
        }
        return 0;
}

#define BUFFER_MIPMAPS_ENABLED 1

graphics_Texture* graphics_createTexture(graphics_TextureData* textureData, graphics_TextureParameters* parameters) {
    U32 id = 0;
    U32 buffer = 0;
    U32 textureType = 0;

    glGenTextures(1, &id);

    if (textureData->bufferType == GRAPHICS_BUFFER_STATIC || textureData->bufferType == GRAPHICS_BUFFER_DYNAMIC) {
        textureType = GL_TEXTURE_BUFFER;
    } else {
        if (textureData->bufferType == GRAPHICS_BUFFER_TEXTURE_ARRAY) {
            switch(textureData->dimensions) {
                case 2:
                    textureType = GL_TEXTURE_1D_ARRAY;
                    break;
                case 3:
                    textureType = GL_TEXTURE_2D_ARRAY;
                    break;
            }
        } else {
              switch(textureData->dimensions) {
                case 1:
                    textureType = GL_TEXTURE_1D;
                    break;
                case 2:
                    textureType = GL_TEXTURE_2D;
                    break;
                case 3:
                    textureType = GL_TEXTURE_3D;
                    break;
            }
        }
    }
    if (textureType) {
        glBindTexture(textureType, id);
        U32 internalFormat = GL_RGBA;
        U32 dataFormat = GL_RGBA;
        U32 dataType = GL_UNSIGNED_BYTE;

        switch(textureData->storage) {
            case GRAPHICS_STORAGE_RGBA:
                internalFormat = GL_RGBA;
                break;
            case GRAPHICS_STORAGE_RGBA_32:
                internalFormat = GL_RGBA32F;
                break;
            case GRAPHICS_STORAGE_UNSIGNED_16:
                internalFormat = GL_R16UI;
                break;

        }

        switch(textureData->input) {
            case GRAPHICS_INPUT_SINGLE_U8:
                dataFormat = GL_RED;
                dataType = GL_UNSIGNED_BYTE;
                break;
            case GRAPHICS_INPUT_FOUR_U8:
                dataFormat = GL_RGBA;
                dataType = GL_UNSIGNED_BYTE;
                break;
            case GRAPHICS_INPUT_SINGLE_FLOAT:
                dataFormat = GL_RED;
                dataType = GL_FLOAT;
                break;
            case GRAPHICS_INPUT_FOUR_FLOAT:
                dataFormat = GL_RGBA;
                dataType = GL_FLOAT;
                break;
            case GRAPHICS_INPUT_SINGLE_UNSIGNED_16:
                dataFormat = GL_RED;
                dataType = GL_UNSIGNED_SHORT;
                break;
        }


        if (textureData->bufferType == GRAPHICS_BUFFER_STATIC || textureData->bufferType == GRAPHICS_BUFFER_DYNAMIC) {
            U32 usage = (textureData->bufferType == GRAPHICS_BUFFER_STATIC) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
            glGenBuffers(1, &buffer);
            glBindBuffer(textureType, buffer);
            glBufferData(GL_TEXTURE_BUFFER, textureData->size[0]*getInputFormatSize(textureData->input), textureData->data, usage);
            glBindTexture(textureType, id);
            glTexBuffer(textureType, internalFormat, buffer);
            glBindBuffer(textureType, 0);

        } else if (textureData->dimensions == 1) {
            glTexImage1D(textureType, 0, internalFormat, textureData->size[0], 0, dataFormat, dataType, textureData->data);
        } else if (textureData->dimensions == 2) {
            glTexImage2D(textureType, 0, internalFormat, textureData->size[0], textureData->size[1], 0, dataFormat, dataType, textureData->data);
        } else if (textureData->dimensions == 3) {
            glTexImage3D(textureType, 0, internalFormat, textureData->size[0], textureData->size[1], textureData->size[2], 0, dataFormat, dataType, textureData->data);
        }

        if (textureData->bufferType != GRAPHICS_BUFFER_STATIC && textureData->bufferType != GRAPHICS_BUFFER_DYNAMIC) {
            switch(parameters->minFilter) {
                case GRAPHICS_FILTERING_NEAREST:
                    glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    break;
                case GRAPHICS_FILTERING_LINEAR_MIPMAP_LINEAR:
                    glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                    break;
                case GRAPHICS_FILTERING_NEAREST_MIPMAP_LINEAR:
                    glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
                    break;
                case GRAPHICS_FILTERING_LINEAR_MIPMAP_NEAREST:
                    glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
                    break;
                case GRAPHICS_FILTERING_NEAREST_MIPMAP_NEAREST:
                    glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                    break;
                default:
                    glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    break;
            }

            if (parameters->magFilter == GRAPHICS_FILTERING_NEAREST) {
                glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            } else {
                glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }

            if (parameters->minFilter != GRAPHICS_FILTERING_LINEAR && parameters->minFilter != GRAPHICS_FILTERING_NEAREST) {
                glGenerateMipmap(textureType);
                buffer = BUFFER_MIPMAPS_ENABLED;
            }
        // TODO: wrapping and anistropy
        }

        glBindTexture(textureType, 0);
    }
    graphics_Texture texture = {id,textureType,buffer};
    graphics_Texture* newTexture = (graphics_Texture*) malloc(sizeof(graphics_Texture));
    *newTexture = texture;
    return newTexture;
}

void graphics_updateTexture(graphics_Texture* texture, graphics_TextureData* textureData, graphics_TextureOffsets* offsets) {
    if (texture->type) {
        glBindTexture(texture->type, texture->reference);
        U32 dataFormat = GL_RGBA;
        U32 dataType = GL_UNSIGNED_BYTE;
        // TODO: get this from a function
        switch(textureData->input) {
            case GRAPHICS_INPUT_SINGLE_U8:
                dataFormat = GL_RED;
                dataType = GL_UNSIGNED_BYTE;
                break;
            case GRAPHICS_INPUT_FOUR_U8:
                dataFormat = GL_RGBA;
                dataType = GL_UNSIGNED_BYTE;
                break;
            case GRAPHICS_INPUT_SINGLE_FLOAT:
                dataFormat = GL_RED;
                dataType = GL_FLOAT;
                break;
            case GRAPHICS_INPUT_FOUR_FLOAT:
                dataFormat = GL_RGBA;
                dataType = GL_FLOAT;
                break;
            case GRAPHICS_INPUT_SINGLE_UNSIGNED_16:
                dataFormat = GL_RED;
                dataType = GL_UNSIGNED_SHORT;
                break;
        }


        if (textureData->bufferType == GRAPHICS_BUFFER_STATIC || textureData->bufferType == GRAPHICS_BUFFER_DYNAMIC) {
            U32 usage = (textureData->bufferType == GRAPHICS_BUFFER_STATIC) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
            glBindBuffer(texture->type, texture->buffer);
            glBufferData(GL_TEXTURE_BUFFER, textureData->size[0]*getInputFormatSize(textureData->input), textureData->data, usage);
            glBindBuffer(texture->type, 0);

        } else if (textureData->dimensions == 1) {
            glTexSubImage1D(texture->type, 0, offsets->offset[0], textureData->size[0], dataFormat, dataType, textureData->data);
        } else if (textureData->dimensions == 2) {
            glTexSubImage2D(texture->type, 0, offsets->offset[0], offsets->offset[1], textureData->size[0], textureData->size[1], dataFormat, dataType, textureData->data);
        } else if (textureData->dimensions == 3) {
            glTexSubImage3D(texture->type, 0, offsets->offset[0], offsets->offset[1], offsets->offset[2], textureData->size[0], textureData->size[1], textureData->size[2], dataFormat, dataType, textureData->data);
        }

        if (textureData->bufferType != GRAPHICS_BUFFER_STATIC && textureData->bufferType != GRAPHICS_BUFFER_DYNAMIC) {
            if (texture->buffer & BUFFER_MIPMAPS_ENABLED) {
                glGenerateMipmap(texture->type);
            }
        }



        glBindTexture(texture->type, 0);
    }
}

void graphics_destroyTexture(graphics_Texture* texture) {
    if (texture->type == GL_TEXTURE_BUFFER) {
        glDeleteBuffers(1, &texture->buffer);
    }
    glDeleteTextures(1, &texture->reference);
    free(texture);
}
const U32 MAX_ATTRIBUTES = 10;
graphics_Mesh* graphics_createMesh(U32 verticesCount, U32 trianglesCount, graphics_VertexAttribute* attributes, U32 attributesCount, U32* triangleElements) {
    GLuint objectID;
    graphics_Mesh* mesh = (graphics_Mesh*) malloc(sizeof(graphics_Mesh) + sizeof(GLuint)*(attributesCount+1));
    mesh->objectBuffers = (U32*) (mesh + 1);
    mesh->objectBuffersCount = attributesCount + 1;

    glGenVertexArrays(1, &objectID);
    glBindVertexArray(objectID);
    // Generate Vertex data and elements
    glGenBuffers(attributesCount+1, mesh->objectBuffers);

    for (int i = 0; i < attributesCount; i++) {
        graphics_VertexAttribute attribute = attributes[i];
        glBindBuffer(GL_ARRAY_BUFFER, mesh->objectBuffers[i]);
        glBufferData(GL_ARRAY_BUFFER, verticesCount * attribute.attributesPerVertex * sizeof(F32), attribute.attributeList, GL_STATIC_DRAW);
        glVertexAttribPointer((GLuint)i, attribute.attributesPerVertex, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(i);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->objectBuffers[attributesCount]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, trianglesCount * 3 * sizeof(U32), triangleElements, GL_STATIC_DRAW);

    mesh->trianglesCount = trianglesCount;
    mesh->reference = objectID;
    return mesh;
}

void graphics_destroyMesh(graphics_Mesh* mesh) {
    glDeleteBuffers(mesh->objectBuffersCount, mesh->objectBuffers);
    glDeleteVertexArrays(1, &mesh->reference);
    free(mesh);
}

static void validateProgram(GLuint program)
{
    const U32 maxSize = 512;
    char buffer[maxSize];
    I32 length = 0;

    glGetProgramInfoLog(program, maxSize, &length, buffer);
    if (length > 0) {
        // TODO : error
        return;
    }

    glValidateProgram(program);
    GLint status;
    glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
    if (status == GL_FALSE) {
        // TODO: program validation status debug info
        return;
    }
}

graphics_Shader* graphics_createShader(graphics_ShaderData* shaderData, graphics_ShaderAttributes shaderAttributes) {
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertex, 1, &shaderData->vertexText, 0);
    glCompileShader(vertex);
    // TODO: validateShader(vertex);

    glShaderSource(fragment, 1, &shaderData->fragmentText, 0);
    glCompileShader(fragment);
    // TODO: validateShader(fragment);

    GLuint id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);

    for (int i = 0; i < shaderAttributes.attributesCount; i++) {
        glBindAttribLocation(id, i, shaderAttributes.attributeNames[i]);
    }

    glLinkProgram(id);
    validateProgram(id);

    graphics_Shader* shader = (graphics_Shader*) malloc(sizeof(graphics_Shader));
    shader->reference = id;
    shader->fragment = fragment;
    shader->vertex = vertex;
    return shader;
}

void graphics_destroyShader(graphics_Shader* shader) {
    glDeleteProgram(shader->fragment);
    glDeleteShader(shader->vertex);
    glDeleteShader(shader->fragment);
    free(shader);
}

void graphics_initMaterial(graphics_Material* material, graphics_Shader* shader, graphics_MaterialAttributes materialAttributes) {
    material->shader = shader;
    for (int i = 0; i < materialAttributes.parametersCount; i++) {
        U32 location = glGetUniformLocation(shader->reference, materialAttributes.parameterNames[i]);
        material->parameterMap.in[i] = materialAttributes.parameterReferences[i];
        material->parameterMap.out[i] = location;
    }
    material->parameterMap.size = materialAttributes.parametersCount;
}

void graphics_init() {
    gl_lite_init();
}

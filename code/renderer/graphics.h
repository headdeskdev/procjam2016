#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "../engine/types.h"
#include "../engine/mmath.h"

// TODO: should the textures stuff be simplified for cross platform reasons?
enum graphics_InputFormat {
    GRAPHICS_INPUT_SINGLE_U8,
    GRAPHICS_INPUT_FOUR_U8,
    GRAPHICS_INPUT_SINGLE_FLOAT,
    GRAPHICS_INPUT_FOUR_FLOAT,
    GRAPHICS_INPUT_SINGLE_UNSIGNED_16
};

enum graphics_StorageFormat {
    GRAPHICS_STORAGE_RGBA,
    GRAPHICS_STORAGE_RGBA_32,
    GRAPHICS_STORAGE_UNSIGNED_16
};

enum graphics_FilteringType {
    GRAPHICS_FILTERING_LINEAR,
    GRAPHICS_FILTERING_NEAREST,
    GRAPHICS_FILTERING_LINEAR_MIPMAP_LINEAR,
    GRAPHICS_FILTERING_NEAREST_MIPMAP_LINEAR,
    GRAPHICS_FILTERING_LINEAR_MIPMAP_NEAREST,
    GRAPHICS_FILTERING_NEAREST_MIPMAP_NEAREST,
};

enum graphics_WrappingType {
    GRAPHICS_WRAPPING_REPEAT,
    GRAPHICS_WRAPPING_REPEAT_MIRROR,
    GRAPHICS_WRAPPING_CLAMP
};

struct graphics_TextureParameters {
    F32 ansitropyLevelMinusOne;
    graphics_FilteringType minFilter;
    graphics_FilteringType magFilter;
    graphics_WrappingType wrapping;
};

struct graphics_TextureOffsets {
    I32 offset[3];
    U8 dimensions;
};

enum graphics_BufferType {
    GRAPHICS_BUFFER_TEXTURE,
    GRAPHICS_BUFFER_TEXTURE_ARRAY,
    GRAPHICS_BUFFER_STATIC,
    GRAPHICS_BUFFER_DYNAMIC
};

struct graphics_TextureData {
    void* data;
    U32 size[3];
    U8 dimensions;
    graphics_InputFormat input;
    graphics_StorageFormat storage;
    graphics_BufferType bufferType;
};

// Internal types

struct graphics_Mesh;
struct graphics_Texture;
struct graphics_Shader;

// Other objects -
// Rendertarget/framebuffers
// Uniform Buffer Object
// Non mesh buffer object
// Other texture types


enum graphics_ShaderParameterType {
    GRAPHICS_SHADER_PARAMETER_TEXTURE,
    GRAPHICS_SHADER_PARAMETER_FLOAT,
    GRAPHICS_SHADER_PARAMETER_VEC2,
    GRAPHICS_SHADER_PARAMETER_VEC3,
    GRAPHICS_SHADER_PARAMETER_VEC4,
    GRAPHICS_SHADER_PARAMETER_MAT3,
    GRAPHICS_SHADER_PARAMETER_MAT4
};

struct graphics_ShaderParameter {
    U32 reference;
    graphics_ShaderParameterType type;
    U32 arrayCount;
    union {
        graphics_Texture* textureParam;
        F32* floatParam;
        Vector2* vector2Param;
        Vector3* vector3Param;
        Vector4* vector4Param;
        Matrix3* matrix3Param;
        Matrix4* matrix4Param;
    };
};

struct graphics_StaticReferenceMap {
    I32* in;
    I32* out;
    I32 size;
    I32 getReference(I32 inValue) {
        for (int i = 0; i < size; i++) {
            if (in[i] == inValue) {
                return out[i];
            }
        }
        return -1;
    }
};

// List of shader parameters sorted by reference
struct graphics_ShaderParameterSortedList {
    graphics_ShaderParameter* parameters;
    U32 parameterCount;

    // NOTE: This function give the impression this is easily modifiable which it is not
    // Should be something else

    void addSortedParameter(graphics_ShaderParameter param) {
        I32 addLocation = 0;
        for (int i = 0; i < parameterCount; i++) {
            if (parameters[i].reference > param.reference) {
				break;
            }
			addLocation++;
        }
        for (int i = parameterCount; i > addLocation; i--) {
            parameters[i] = parameters[i-1];
        }
        parameters[addLocation] = param;

        parameterCount++;
    }
};

struct graphics_Material {
    // TODO: more advanced materials
    graphics_Shader* shader;
    // TODO: handle special texture types (e.g. loading in different textures, references to texture objects etc)
    // NOTE: could just have the user have to update the material parameters if things change
    //       also textures work differently so may need to handled differently to other parameters
    graphics_ShaderParameterSortedList defaultParameters;
    graphics_StaticReferenceMap parameterMap;
};

struct graphics_MeshMaterial {
    graphics_Mesh* mesh;
    graphics_Material* material;
};

struct graphics_Quad {
    Quad quad;
    Rect textureCoord;
    Vector4 colour;
};

struct graphics_QuadMaterial {
    graphics_Quad* quad;
    graphics_Material* material;  
};

enum graphics_RenderObjectType {
    RENDER_OBJECT_MESH_MATERIAL,

    // TOOD: Maybe just use meshes
    RENDER_OBJECT_QUAD_MATERIAL
    // NOTE: Are there actually good reasons to have other types?
};

struct graphics_RenderObject {
    graphics_RenderObjectType type;
    union {
        graphics_MeshMaterial* meshMaterial;
        graphics_QuadMaterial* quadMaterial;
    };
    graphics_ShaderParameterSortedList objectParameters;
};

struct graphics_RenderPass {
    Vector3 clearColour;
    bool clearColourEnabled;   

    bool clearDepthEnabled;

    bool alphaBlending;
    // Blending type (also how to deal with DX11)
    bool depthCheck;
    // Depth check type
    bool writeDepth;
    bool cullFace;
    // Cull face type

    // Render target

    F32 viewportX;
    F32 viewportY;
    F32 viewportWidth;
    F32 viewportHeight;

    // Material overrride
    // Material aspect (we want to have the ability to use specific aspects of materials -
    ///                 like texture/specularity/diffuse etc) Note sure how it will work yet

    graphics_ShaderParameterSortedList globalParameters;
};

struct graphics_RenderPassOutput {

};

graphics_RenderPassOutput graphics_render(graphics_RenderPass* pass, graphics_RenderObject* renderObjects, U32 renderObjectCount);

// TODO: expand this to allow different texture properties
graphics_Texture* graphics_createTexture(graphics_TextureData* textureData, graphics_TextureParameters* parameters);
void graphics_updateTexture(graphics_Texture* texture, graphics_TextureData* textureData, graphics_TextureOffsets* offsets);
void graphics_destroyTexture(graphics_Texture* texture);

// TODO: non floating point attributes
struct graphics_VertexAttribute {
    F32* attributeList;
    U32 attributesPerVertex;
};

graphics_Mesh* graphics_createMesh(U32 verticesCount, U32 trianglesCount, graphics_VertexAttribute* attributes, U32 attributesCount, U32* triangleElements);
void graphics_destroyMesh(graphics_Mesh* mesh);

struct graphics_MaterialAttributes {
    const char** parameterNames;
    U32* parameterReferences;
    U32 parametersCount;
};

struct graphics_ShaderAttributes {
    const char** attributeNames;
    U32 attributesCount;
};

// TODO: how to make this api independent??????
struct graphics_ShaderData {
    char* vertexText;
    char* fragmentText;
};

graphics_Shader* graphics_createShader(graphics_ShaderData* shaderData, graphics_ShaderAttributes shaderAttributes);
void graphics_destroyShader(graphics_Shader* shader);

// Memory must be initialised
void graphics_initMaterial(graphics_Material* material, graphics_Shader* shader, graphics_MaterialAttributes materialAttributes);

void graphics_init();

graphics_RenderPassOutput graphics_render(graphics_RenderPass* pass, graphics_RenderObject* renderObjects, U32 renderObjectsCount);

#endif

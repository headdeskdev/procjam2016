#define WORLD_RENDERER_CLUSTER_WIDTH 16
#define WORLD_RENDERER_CLUSTER_HEIGHT 8
#define WORLD_RENDERER_CLUSTER_DEPTH 24
#define WORLD_RENDERER_AVERAGE_CLUSTER_LIGHTS 8
#define WORLD_RENDERER_MAX_LIGHTS 8192
#define WORLD_RENDERER_MAX_OBJECTS 16384

struct LightInstance {
    Vector3 pos;
    Vector3 dir;
    Vector3 color;
    F32 coneDot;
    F32 radius;
    F32 intensity;
    U8 shadowsEnabled;
};

struct Cluster {
    Cluster* nextCluster;
    I32 light;
};

struct ClusterLightingSystem {
    Cluster** clusters;
    LightInstance* lights;
    U16 lightsCount;    

	U32 additionalClusters;

    graphics_Texture* clusterBuffer;
    graphics_Texture* clusterItemBuffer;
    graphics_Texture* clusterLights;  
};

enum WorldRendererShaderParameter {
    SHADER_PARAMETER_PROJECTION,

    SHADER_PARAMETER_AMBIENT,
    SHADER_PARAMETER_DIRECTIONAL_DIRECTION,
    SHADER_PARAMETER_DIRECTIONAL_COLOR,
    SHADER_PARAMETER_CLUSTER_BUFFER,
    SHADER_PARAMETER_CLUSTER_ITEM_BUFFER,
    SHADER_PARAMETER_LIGHT_BUFFER,

    SHADER_PARAMETER_MODEL_VIEW,
    SHADER_PARAMETER_NORMAL_MODEL_VIEW,

    SHADER_PARAMETER_MAIN_TEXTURE
};

struct WorldRenderer {
    ClusterLightingSystem lightingSystem;

    LightInstance directionalLights[2];
    LightInstance ambientLight;

    Matrix4 viewMatrix;
    Matrix4 perspectiveMatrix;
    Vector2 screenSize;
    Vector3 backgroundColour;

    graphics_RenderObject* objects;
    U32 objectsCount;

    memory_Arena rendererFrameMemory;

    void initFrame(Matrix4 viewMatrix, Matrix4 perspectiveMatrix, Vector2 screenSize) {        
        rendererFrameMemory.clear();
        // Allocate cluster lighting memory
        U32 baseClusters = WORLD_RENDERER_CLUSTER_WIDTH*WORLD_RENDERER_CLUSTER_HEIGHT*WORLD_RENDERER_CLUSTER_DEPTH;
        lightingSystem.clusters = ARENA_GET_ARRAY(rendererFrameMemory, Cluster*, baseClusters);
        for (int i = 0; i < baseClusters; i++) {
            lightingSystem.clusters[i] = 0;
        }
        lightingSystem.lights = ARENA_GET_ARRAY(rendererFrameMemory, LightInstance, WORLD_RENDERER_MAX_LIGHTS);
        lightingSystem.lightsCount = 0;
        // Allocate render object memory
        objects = ARENA_GET_ARRAY(rendererFrameMemory, graphics_RenderObject, WORLD_RENDERER_MAX_OBJECTS);
        objectsCount = 0;

        this->screenSize = screenSize;
        this->viewMatrix = viewMatrix;
        this->perspectiveMatrix = perspectiveMatrix;

		//
		lightingSystem.additionalClusters = 0;
    }

    inline I32 getClusterZ(Vector3 position, bool isMin) {
        I32 clusterValue = 0;
        if (position.z < 0) {
            if (isMin) {
                clusterValue = floor((0.125662*logf(-position.z + 0.25) + 0.131923)*WORLD_RENDERER_CLUSTER_DEPTH);
            } else {
                clusterValue = ceil((0.125662*logf(-position.z + 0.25) + 0.131923)*WORLD_RENDERER_CLUSTER_DEPTH);
            }
        }
        return math_clampI32(clusterValue,0,WORLD_RENDERER_CLUSTER_DEPTH);
    }

    inline F32 getInverseClusterZMin(I32 z) {
        F32 nz = (z * 1.0f/WORLD_RENDERER_CLUSTER_DEPTH);
        return -(exp((nz - 0.131923)/0.125662) - 0.25);
    }

    inline I32 getClusterX(Vector3 position, bool isMin) {
        Vector3 perspective = perspectiveMatrix.transformPerspective(position);
        I32 clusterValue;
        if (isMin) {
            clusterValue = floor((perspective.x*0.5 + 0.5)*WORLD_RENDERER_CLUSTER_WIDTH);
        } else {
            clusterValue = ceil((perspective.x*0.5 + 0.5)*WORLD_RENDERER_CLUSTER_WIDTH);
        }
        return math_clampI32(clusterValue,0,WORLD_RENDERER_CLUSTER_WIDTH);
    }

    inline I32 getClusterY(Vector3 position, bool isMin) {
        Vector3 perspective = perspectiveMatrix.transformPerspective(position);
        I32 clusterValue;
        if (isMin) {
            clusterValue = floor((perspective.y*0.5 + 0.5)*WORLD_RENDERER_CLUSTER_HEIGHT);
        } else {
            clusterValue = ceil((perspective.y*0.5 + 0.5)*WORLD_RENDERER_CLUSTER_HEIGHT);
        }
        return math_clampI32(clusterValue,0,WORLD_RENDERER_CLUSTER_HEIGHT);
    }

    void addLight(LightInstance* light) {                
        Vector3 lightViewPos = viewMatrix.transformPoint(light->pos);
        Vector3 size = {1.0,1.0,-1.0};
        F32 radius = light->radius;
        Vector3 box[2] = {lightViewPos + size * -radius, lightViewPos + size * radius};
        U32 currentLight = lightingSystem.lightsCount;
        bool isActive = false;
        for (I32 cz = getClusterZ(box[0],true); cz < getClusterZ(box[1],false); cz++) {
            F32 z = getInverseClusterZMin(cz);
            F32 zFar = getInverseClusterZMin(cz + 1);
            //F32 t = (z - box[0].z) / (box[1].z - box[0].z);
            Vector3 minNear = box[0]; minNear.z = z;
            Vector3 maxNear = box[1]; maxNear.z = z;
            Vector3 minFar = box[0]; minFar.z = zFar;
            Vector3 maxFar = box[1]; minFar.z = zFar;
            I32 cxMin = mmin(getClusterX(minNear,true),getClusterX(minFar,true));
            I32 cxMax = mmax(getClusterX(maxNear,false),getClusterX(maxFar,false));
            I32 cyMin = mmin(getClusterY(minNear,true),getClusterY(minFar,true));
            I32 cyMax = mmax(getClusterY(maxNear,false),getClusterY(maxFar,false));

            for (I32 cx = cxMin; cx < cxMax; cx++) {
                for (I32 cy = cyMin; cy < cyMax; cy++) {
                    int clusterNum = (cx + cy * WORLD_RENDERER_CLUSTER_WIDTH + cz * WORLD_RENDERER_CLUSTER_HEIGHT * WORLD_RENDERER_CLUSTER_WIDTH);
                    Cluster* currentCluster = lightingSystem.clusters[clusterNum];
                    Cluster* newCluster = ARENA_GET_STRUCT(rendererFrameMemory, Cluster);
					lightingSystem.clusters[clusterNum] = newCluster;
					newCluster->nextCluster = currentCluster;
                    newCluster->light = currentLight;
                    lightingSystem.additionalClusters++;
                    isActive = true;
                }
            }
        }
        if (isActive) {
			LightInstance newLight = *light;
			newLight.dir = viewMatrix.transformVector(newLight.dir);
			newLight.pos = viewMatrix.transformPoint(newLight.pos);
			lightingSystem.lights[lightingSystem.lightsCount++] = newLight;

        }
    }

    void addObject(graphics_Mesh* mesh, graphics_Material* material, Matrix4* modelMatrix, Matrix4* normalModelMatrix) {
        graphics_MeshMaterial* meshMaterial = ARENA_GET_STRUCT(rendererFrameMemory, graphics_MeshMaterial);
        meshMaterial->material = material;
        meshMaterial->mesh = mesh;

        graphics_ShaderParameterSortedList parameters = {0};
        parameters.parameters = ARENA_GET_ARRAY(rendererFrameMemory, graphics_ShaderParameter, 3);

        graphics_ShaderParameter modelViewParameter = {SHADER_PARAMETER_MODEL_VIEW,GRAPHICS_SHADER_PARAMETER_MAT4,1};
        Matrix4* newModelViewMatrix = ARENA_GET_STRUCT(rendererFrameMemory, Matrix4);
        *newModelViewMatrix = viewMatrix * (*modelMatrix);
        modelViewParameter.matrix4Param = newModelViewMatrix;
        parameters.addSortedParameter(modelViewParameter);

        graphics_ShaderParameter normalModelViewParameter = {SHADER_PARAMETER_NORMAL_MODEL_VIEW,GRAPHICS_SHADER_PARAMETER_MAT4,1};
        Matrix4* newNormalModelViewMatrix = ARENA_GET_STRUCT(rendererFrameMemory, Matrix4);
        *newNormalModelViewMatrix = viewMatrix * (*normalModelMatrix);
        normalModelViewParameter.matrix4Param = newNormalModelViewMatrix;
        parameters.addSortedParameter(normalModelViewParameter);        

        graphics_RenderObject newObject = {RENDER_OBJECT_MESH_MATERIAL,meshMaterial,parameters};
		objects[objectsCount++] = newObject;
    }

    void render() {
        U32 baseClusters = WORLD_RENDERER_CLUSTER_WIDTH*WORLD_RENDERER_CLUSTER_HEIGHT*WORLD_RENDERER_CLUSTER_DEPTH;
        U32 clusterItemsMax = baseClusters*WORLD_RENDERER_AVERAGE_CLUSTER_LIGHTS;

        U16* clusterData = ARENA_GET_ARRAY(rendererFrameMemory,U16,baseClusters*2);
        U16* clusterItemData = ARENA_GET_ARRAY(rendererFrameMemory,U16,clusterItemsMax);
        F32* lightData = ARENA_GET_ARRAY(rendererFrameMemory,F32,WORLD_RENDERER_MAX_LIGHTS*12);

        U16 clusterItemCurrent = 0;
        // Convert clusters into buffer data format
        for (int z = 0; z < WORLD_RENDERER_CLUSTER_DEPTH; z++) {
            for (int y = 0; y < WORLD_RENDERER_CLUSTER_HEIGHT; y++) {
                for (int x = 0; x < WORLD_RENDERER_CLUSTER_WIDTH; x++) {
                    int clusterNum = (x + y * WORLD_RENDERER_CLUSTER_WIDTH + z * WORLD_RENDERER_CLUSTER_HEIGHT * WORLD_RENDERER_CLUSTER_WIDTH);

                    U16 currentCount = 0;
                    Cluster* currentCluster = lightingSystem.clusters[clusterNum];
                    while(currentCluster) {
                        clusterItemData[clusterItemCurrent+currentCount] = currentCluster->light;
                        currentCluster = currentCluster->nextCluster;
                        currentCount++;
                    }
                    // Position of cluster item in cluster item array
                    clusterData[2*clusterNum] = clusterItemCurrent;
                    // Number of lights in the cluster
                    clusterData[2*clusterNum+1] = currentCount;
                    clusterItemCurrent += currentCount;
                }
            }
        }
        // Convert lights into buffer data format
        for (int i = 0; i < lightingSystem.lightsCount; i++) {
            LightInstance instance = lightingSystem.lights[i];
            lightData[i*12 + 0] = instance.pos.x;
            lightData[i*12 + 1] = instance.pos.y;
            lightData[i*12 + 2] = instance.pos.z;
            lightData[i*12 + 3] = instance.radius;
            lightData[i*12 + 4] = instance.dir.x;
            lightData[i*12 + 5] = instance.dir.y;
            lightData[i*12 + 6] = instance.dir.z;
            lightData[i*12 + 7] = instance.coneDot;
            lightData[i*12 + 8] = instance.color.x;
            lightData[i*12 + 9] = instance.color.y;
            lightData[i*12 + 10] = instance.color.z;
            lightData[i*12 + 11] = instance.intensity;
        }
        graphics_TextureOffsets offsets = {0};
        {
            graphics_TextureData data = {0};
            data.size[0] = baseClusters*2;
            data.data = clusterData;
            data.dimensions = 1;
            data.input = GRAPHICS_INPUT_SINGLE_UNSIGNED_16;
            data.storage = GRAPHICS_STORAGE_UNSIGNED_16;
            data.bufferType = GRAPHICS_BUFFER_DYNAMIC;
            graphics_updateTexture(lightingSystem.clusterBuffer,&data,&offsets);
        }
        {
            graphics_TextureData data = {0};
            data.size[0] = clusterItemsMax;
            data.data = clusterItemData;
            data.dimensions = 1;
            data.input = GRAPHICS_INPUT_SINGLE_UNSIGNED_16;
            data.storage = GRAPHICS_STORAGE_UNSIGNED_16;
            data.bufferType = GRAPHICS_BUFFER_DYNAMIC;
            graphics_updateTexture(lightingSystem.clusterItemBuffer,&data,&offsets);
        }
        {
            graphics_TextureData data = {0};
            data.size[0] = WORLD_RENDERER_MAX_LIGHTS*3;
            data.data = lightData;
            data.dimensions = 1;
            data.input = GRAPHICS_INPUT_FOUR_FLOAT;
            data.storage = GRAPHICS_STORAGE_RGBA_32;
            data.bufferType = GRAPHICS_BUFFER_DYNAMIC;
            graphics_updateTexture(lightingSystem.clusterLights,&data,&offsets);
        }    

        // RENDER PASS: render all objects with lights
        graphics_RenderPass pass = {0};
        pass.clearColourEnabled = true;
        pass.clearColour = backgroundColour;
        pass.clearDepthEnabled = true;
        pass.depthCheck = true;
        pass.writeDepth = true;
        pass.cullFace = true;
        pass.viewportWidth = screenSize.x;
        pass.viewportHeight = screenSize.y;
        // NOTE: Is there a way of doing this that handles the memory nicely + doesn't produce
        // lots of code
        pass.globalParameters.parameters = ARENA_GET_ARRAY(rendererFrameMemory, graphics_ShaderParameter, 7);

        {graphics_ShaderParameter projectionParameter = {SHADER_PARAMETER_PROJECTION,GRAPHICS_SHADER_PARAMETER_MAT4,1};
        Matrix4* newMatrix = ARENA_GET_STRUCT(rendererFrameMemory, Matrix4);
        *newMatrix = perspectiveMatrix;            
        projectionParameter.matrix4Param = newMatrix;
        pass.globalParameters.addSortedParameter(projectionParameter);}

        {graphics_ShaderParameter ambientParameter = {SHADER_PARAMETER_AMBIENT,GRAPHICS_SHADER_PARAMETER_VEC3,1};
        Vector3* ambient = ARENA_GET_STRUCT(rendererFrameMemory, Vector3);
        *ambient = ambientLight.color;
        ambientParameter.vector3Param = ambient;
        pass.globalParameters.addSortedParameter(ambientParameter);}

        {graphics_ShaderParameter directionalDirectionParameter = {SHADER_PARAMETER_DIRECTIONAL_DIRECTION,GRAPHICS_SHADER_PARAMETER_VEC3,2};
        Vector3* directionalDirection = ARENA_GET_ARRAY(rendererFrameMemory, Vector3, 2);
        directionalDirection[0] = viewMatrix.transformVector(directionalLights[0].dir);
        directionalDirection[1] = viewMatrix.transformVector(directionalLights[1].dir);
        directionalDirectionParameter.vector3Param = directionalDirection;
        pass.globalParameters.addSortedParameter(directionalDirectionParameter);}

        {graphics_ShaderParameter directionalColorParameter = {SHADER_PARAMETER_DIRECTIONAL_COLOR,GRAPHICS_SHADER_PARAMETER_VEC3,2};
        Vector3* directionalColor = ARENA_GET_ARRAY(rendererFrameMemory, Vector3, 2);
        directionalColor[0] = directionalLights[0].color;
        directionalColor[1] = directionalLights[1].color;
        directionalColorParameter.vector3Param = directionalColor;
        pass.globalParameters.addSortedParameter(directionalColorParameter);}

        {graphics_ShaderParameter textureParameter = {SHADER_PARAMETER_CLUSTER_BUFFER, GRAPHICS_SHADER_PARAMETER_TEXTURE, 1};
        textureParameter.textureParam = lightingSystem.clusterBuffer;
        pass.globalParameters.addSortedParameter(textureParameter);}

        {graphics_ShaderParameter textureParameter = {SHADER_PARAMETER_CLUSTER_ITEM_BUFFER, GRAPHICS_SHADER_PARAMETER_TEXTURE, 1};
        textureParameter.textureParam = lightingSystem.clusterItemBuffer;
        pass.globalParameters.addSortedParameter(textureParameter);}

        {graphics_ShaderParameter textureParameter = {SHADER_PARAMETER_LIGHT_BUFFER, GRAPHICS_SHADER_PARAMETER_TEXTURE, 1};
        textureParameter.textureParam = lightingSystem.clusterLights;
        pass.globalParameters.addSortedParameter(textureParameter);}    

        graphics_render(&pass, objects, objectsCount);
    }
};

WorldRenderer* createWorldRenderer(U32 memorySize) {    
    WorldRenderer* worldRenderer = (WorldRenderer*) malloc(sizeof(WorldRenderer)+memorySize);
    void* memory = worldRenderer + 1;
    worldRenderer->rendererFrameMemory = memory_createArena(memory, memorySize);

    U32 baseClusters = WORLD_RENDERER_CLUSTER_WIDTH*WORLD_RENDERER_CLUSTER_HEIGHT*WORLD_RENDERER_CLUSTER_DEPTH;
    U32 clusterItemsMax = baseClusters*WORLD_RENDERER_AVERAGE_CLUSTER_LIGHTS;
    // Create intial shader buffer textures for lighting
    {
        graphics_TextureParameters parameters = {0.0f};
        graphics_TextureData data = {0};
        data.size[0] = baseClusters*2;
        data.dimensions = 1;
        data.input = GRAPHICS_INPUT_SINGLE_UNSIGNED_16;
        data.storage = GRAPHICS_STORAGE_UNSIGNED_16;
        data.bufferType = GRAPHICS_BUFFER_DYNAMIC;
        worldRenderer->lightingSystem.clusterBuffer = graphics_createTexture(&data,&parameters);
    }
    {
        graphics_TextureParameters parameters = {0.0f};
        graphics_TextureData data = {0};
        data.size[0] = clusterItemsMax;
        data.dimensions = 1;
        data.input = GRAPHICS_INPUT_SINGLE_UNSIGNED_16;
        data.storage = GRAPHICS_STORAGE_UNSIGNED_16;
        data.bufferType = GRAPHICS_BUFFER_DYNAMIC;
        worldRenderer->lightingSystem.clusterItemBuffer = graphics_createTexture(&data,&parameters);
    }
    {
        graphics_TextureParameters parameters = {0.0f};
        graphics_TextureData data = {0};
        data.size[0] = WORLD_RENDERER_MAX_LIGHTS*3;
        data.dimensions = 1;
        data.input = GRAPHICS_INPUT_FOUR_FLOAT;
        data.storage = GRAPHICS_STORAGE_RGBA_32;
        data.bufferType = GRAPHICS_BUFFER_DYNAMIC;
        worldRenderer->lightingSystem.clusterLights = graphics_createTexture(&data,&parameters);
    }
    return worldRenderer;
}

void destroyWorldRenderer(WorldRenderer* worldRenderer) {
    free(worldRenderer);
}





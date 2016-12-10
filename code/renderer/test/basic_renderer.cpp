struct Camera3d {
	Matrix4 projectionMatrix;
	Matrix4 viewMatrix;
};

struct SimpleRenderer {
	graphics_RenderObject* objects;
	U32 objectsCount;

	memory_Arena memory;
	Camera3d camera;
	
	Vector3 backgroundColour;
	Vector3 lightDirection;
	Vector2 screenSize;

	void renderSimple() {
		graphics_RenderPass pass = {0};
		pass.clearColourEnabled = true;
		pass.clearColour = backgroundColour;
		pass.clearDepthEnabled = true;
		pass.alphaBlending = false;
		pass.depthCheck = true;
		pass.cullFace = true;
		pass.viewportWidth = screenSize.x;
		pass.viewportHeight = screenSize.y;
		pass.globalParameters.parameters = ARENA_GET_ARRAY(memory, graphics_ShaderParameter, 2);
		Matrix4* newMatrix = ARENA_GET_STRUCT(memory, Matrix4);
		*newMatrix = camera.projectionMatrix;
		graphics_ShaderParameter projectionParameter = {SHADER_PARAMETER_PROJECTION,GRAPHICS_SHADER_PARAMETER_MAT4,1};
		projectionParameter.matrix4Param = newMatrix;
		pass.globalParameters.addSortedParameter(projectionParameter);
		graphics_ShaderParameter lightingParameter = {SHADER_PARAMETER_LIGHT_DIRECTION,GRAPHICS_SHADER_PARAMETER_VEC3,1};
		Vector3* light = ARENA_GET_STRUCT(memory, Vector3);
		*light = camera.viewMatrix.transformVector(lightDirection);
		lightingParameter.vector3Param = light;
		pass.globalParameters.addSortedParameter(lightingParameter);

		graphics_render(&pass, objects, objectsCount);
	}

	void addMeshMaterialRenderObject(graphics_Mesh* mesh, graphics_Material* material, Matrix4* modelMatrix, Vector3 colour) {
		graphics_MeshMaterial* meshMaterial = ARENA_GET_STRUCT(memory, graphics_MeshMaterial);
		meshMaterial->material = material;
		meshMaterial->mesh = mesh;

		graphics_ShaderParameterSortedList parameters = {0};
		parameters.parameters = ARENA_GET_ARRAY(memory, graphics_ShaderParameter, 3);
		Matrix4* newMatrix = ARENA_GET_STRUCT(memory, Matrix4);
		*newMatrix = camera.viewMatrix * (*modelMatrix);		
		graphics_ShaderParameter modelViewParameter = {SHADER_PARAMETER_MODEL_VIEW,GRAPHICS_SHADER_PARAMETER_MAT4,1};
		modelViewParameter.matrix4Param = newMatrix;
		parameters.addSortedParameter(modelViewParameter);

		graphics_ShaderParameter modelParameter = {SHADER_PARAMETER_MODEL,GRAPHICS_SHADER_PARAMETER_MAT4,1};
		Matrix4* newModelMatrix = ARENA_GET_STRUCT(memory, Matrix4);
		*newModelMatrix = *modelMatrix;
		modelParameter.matrix4Param = newModelMatrix;
	    parameters.addSortedParameter(modelParameter);

	    graphics_ShaderParameter colourParameter = {SHADER_PARAMETER_COLOUR,GRAPHICS_SHADER_PARAMETER_VEC3,1};
	    Vector3* colourRef = ARENA_GET_STRUCT(memory, Vector3);
	    *colourRef = colour;
		colourParameter.vector3Param = colourRef;
	    parameters.addSortedParameter(colourParameter);
		
		graphics_RenderObject newObject = {RENDER_OBJECT_MESH_MATERIAL,meshMaterial,parameters};

		objects[objectsCount++] = newObject;
	}
};

SimpleRenderer createSimpleRenderer(void* memory, U32 size, U32 maxObjects) {
	SimpleRenderer renderer = {};
	renderer.memory = memory_createArena(memory, size);
	renderer.objects = ARENA_GET_ARRAY(renderer.memory, graphics_RenderObject, maxObjects);
	return renderer;
}

struct Renderer2d {
	graphics_RenderObject* objects;
	U32 objectsCount;

	memory_Arena memory;
	Vector2 screenSize;
	Matrix3 globalMatrix;
	graphics_Material* material2d;

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

	void setScreenSize(Vector2 screenSizeVector) {
		screenSize = screenSizeVector;
		TransformMatrix2d screenTransform = {2.0f/screenSize.x,0.0f,0.0f,-2.0f/screenSize.y,-1.0f,1.0f};
    	TransformMatrix2d userTransform = {1.0f,0.0f,0.0f,1.0f};
    	globalMatrix = screenTransform.toMatrix3();
	}

	void addTexturedQuad(graphics_Texture* texture, Quad quad, Rect textureRect, Vector4 colour) {

		// graphics_Quad* gquad = ARENA_GET_STRUCT(memory, graphics_Quad);
		// gquad->quad = quad;
		// gquad->textureCoord = textureRect;
		// gquad->colour = colour;

		// graphics_ShaderParameterSortedList parameters = {0};
		// parameters.parameters = ARENA_GET_ARRAY(memory, graphics_ShaderParameter, 1);
		// graphics_ShaderParameter param = {SHADER_PARAMETER_TEXTURE, GRAPHICS_SHADER_PARAMETER_TEXTURE, 1, texture};
		// parameters.addSortedParameter(param);

	 //    graphics_QuadMaterial* quadMaterial = ARENA_GET_STRUCT(memory, graphics_QuadMaterial);
		// quadMaterial->material = material2d;
		// quadMaterial->quad = gquad;		

		// graphics_RenderObject newObject = {RENDER_OBJECT_QUAD_MATERIAL};
		// newObject.quadMaterial = quadMaterial;
		// newObject.objectParameters = parameters;

		// objects[objectsCount++] = newObject;
	}
};



Renderer2d createRenderer2d(void* memory, U32 size, U32 maxObjects, graphics_Material* material) {
	Renderer2d renderer = {};
	renderer.memory = memory_createArena(memory, size);
	renderer.objects = ARENA_GET_ARRAY(renderer.memory, graphics_RenderObject, maxObjects);
	renderer.material2d = material;
	return renderer;	
}


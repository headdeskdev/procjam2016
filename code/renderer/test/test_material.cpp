enum ShaderParameters {
	SHADER_PARAMETER_LIGHT_DIRECTION,
	SHADER_PARAMETER_MODEL_VIEW,
	SHADER_PARAMETER_PROJECTION,
	SHADER_PARAMETER_MODEL,
	SHADER_PARAMETER_TEXTURE,
	SHADER_PARAMETER_COLOUR,
	SHADER_PARAMETER_SCREEN_VIEW_2D,
};

graphics_Material* createTestMaterial(graphics_Shader* shader, graphics_Texture* texture, memory_Arena* assetMemory) {
	graphics_Material* material = ARENA_GET_STRUCT(*assetMemory, graphics_Material);
	material->defaultParameters = {0};
	material->defaultParameters.parameters = ARENA_GET_ARRAY(*assetMemory, graphics_ShaderParameter, 1);
	graphics_ShaderParameter param = {SHADER_PARAMETER_TEXTURE, GRAPHICS_SHADER_PARAMETER_TEXTURE, 1, texture};
	material->defaultParameters.addSortedParameter(param);

	material->parameterMap.in = ARENA_GET_ARRAY(*assetMemory, I32, 5);
	material->parameterMap.out = ARENA_GET_ARRAY(*assetMemory, I32, 5);
	const char* names[5] = {"cameraSpaceLightDirection","modelViewMatrix","projectionMatrix","modelMatrix","textureMap"};
	U32 refs[5] = {SHADER_PARAMETER_LIGHT_DIRECTION,SHADER_PARAMETER_MODEL_VIEW,SHADER_PARAMETER_PROJECTION,SHADER_PARAMETER_MODEL,SHADER_PARAMETER_TEXTURE};
	graphics_MaterialAttributes attributes = {names, refs, 5};
	graphics_initMaterial(material, shader, attributes);
	return material;
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

// graphics_Material* createTestMaterial(graphics_Shader* shader, graphics_Texture* texture, MemoryArena* assetMemory) {
// 	graphics_Material* material = ARENA_GET_STRUCT(*assetMemory, graphics_Material);
// 	material->defaultParameters = {0};
// 	material->defaultParameters.parameters = ARENA_GET_ARRAY(*assetMemory, graphics_ShaderParameter, 1);
// 	graphics_ShaderParameter param = {SHADER_PARAMETER_MAIN_TEXTURE, GRAPHICS_SHADER_PARAMETER_TEXTURE, texture};
// 	material->defaultParameters.addSortedParameter(param);

// 	material->parameterMap.in = ARENA_GET_ARRAY(*assetMemory, I32, 4);
// 	material->parameterMap.out = ARENA_GET_ARRAY(*assetMemory, I32, 4);
// 	char* names[4] = {"textureMap","cameraSpaceLightDirection","modelViewMatrix","projectionMatrix"};
// 	U32 refs[4] = {SHADER_PARAMETER_MAIN_TEXTURE,SHADER_PARAMETER_LIGHT_DIRECTION,
// 					 SHADER_PARAMETER_MODEL_VIEW,SHADER_PARAMETER_PROJECTION};
// 	MaterialAttributes attributes = {names, refs, 4};
// 	initgraphics_Material(material, shader, attributes);
// 	return material;
// }



#include <SDL.h>

// TODO: make this cross platform

#include "imgui/imgui.cpp"
#include "imgui/imgui_draw.cpp"

struct ImguiGLState {
    bool ready;
    GLuint fontTexture;
    int shaderHandle, vertHandle, fragHandle;
    int attribLocationTex, attribLocationProjMtx;
    int attribLocationPosition, attribLocationUV, attribLocationColor;
    unsigned int vboHandle, vaoHandle, elementsHandle;    
};

static ImguiGLState renderGUIState;

static void renderGUICallback(ImDrawData* draw_data) 
{
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    ImGuiIO& io = ImGui::GetIO();
    int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fb_width == 0 || fb_height == 0)
        return;
    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    // Backup GL state
    GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
    GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
    GLint last_element_array_buffer; glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
    GLint last_vertex_array; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
    GLint last_blend_src; glGetIntegerv(GL_BLEND_SRC, &last_blend_src);
    GLint last_blend_dst; glGetIntegerv(GL_BLEND_DST, &last_blend_dst);
    GLint last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, &last_blend_equation_rgb);
    GLint last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &last_blend_equation_alpha);
    GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
    GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
    GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
    GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
    GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glActiveTexture(GL_TEXTURE0);

    // Setup orthographic projection matrix
    glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
    const float ortho_projection[4][4] =
    {
        { 2.0f/io.DisplaySize.x, 0.0f,                   0.0f, 0.0f },
        { 0.0f,                  2.0f/-io.DisplaySize.y, 0.0f, 0.0f },
        { 0.0f,                  0.0f,                  -1.0f, 0.0f },
        {-1.0f,                  1.0f,                   0.0f, 1.0f },
    };
    glUseProgram(renderGUIState.shaderHandle);
    glUniform1i(renderGUIState.attribLocationTex, 0);
    glUniformMatrix4fv(renderGUIState.attribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);
    glBindVertexArray(renderGUIState.vaoHandle);

    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawIdx* idx_buffer_offset = 0;

        glBindBuffer(GL_ARRAY_BUFFER, renderGUIState.vboHandle);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.size() * sizeof(ImDrawVert), (GLvoid*)&cmd_list->VtxBuffer.front(), GL_STREAM_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderGUIState.elementsHandle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx), (GLvoid*)&cmd_list->IdxBuffer.front(), GL_STREAM_DRAW);

        for (const ImDrawCmd* pcmd = cmd_list->CmdBuffer.begin(); pcmd != cmd_list->CmdBuffer.end(); pcmd++)
        {
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
                glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
            }
            idx_buffer_offset += pcmd->ElemCount;
        }
    }

    // Restore modified GL state
    glUseProgram(last_program);
    glBindTexture(GL_TEXTURE_2D, last_texture);
    glBindVertexArray(last_vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
    glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
    glBlendFunc(last_blend_src, last_blend_dst);
    if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
    if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
    if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
    glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
}

static void init(ImGuiIO& io, ImguiGLState* imguiGLState) {
        GLint last_texture, last_array_buffer, last_vertex_array;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

    const GLchar *vertex_shader =
        "#version 330\n"
        "uniform mat4 ProjMtx;\n"
        "in vec2 Position;\n"
        "in vec2 UV;\n"
        "in vec4 Color;\n"
        "out vec2 Frag_UV;\n"
        "out vec4 Frag_Color;\n"
        "void main()\n"
        "{\n"
        "   Frag_UV = UV;\n"
        "   Frag_Color = Color;\n"
        "   gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
        "}\n";

    const GLchar* fragment_shader =
        "#version 330\n"
        "uniform sampler2D Texture;\n"
        "in vec2 Frag_UV;\n"
        "in vec4 Frag_Color;\n"
        "out vec4 Out_Color;\n"
        "void main()\n"
        "{\n"
        "   Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
        "}\n";

    imguiGLState->shaderHandle = glCreateProgram();
    imguiGLState->vertHandle = glCreateShader(GL_VERTEX_SHADER);
    imguiGLState->fragHandle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(imguiGLState->vertHandle, 1, &vertex_shader, 0);
    glShaderSource(imguiGLState->fragHandle, 1, &fragment_shader, 0);
    glCompileShader(imguiGLState->vertHandle);
    glCompileShader(imguiGLState->fragHandle);
    glAttachShader(imguiGLState->shaderHandle, imguiGLState->vertHandle);
    glAttachShader(imguiGLState->shaderHandle, imguiGLState->fragHandle);
    glLinkProgram(imguiGLState->shaderHandle);

    imguiGLState->attribLocationTex = glGetUniformLocation(imguiGLState->shaderHandle, "Texture");
    imguiGLState->attribLocationProjMtx = glGetUniformLocation(imguiGLState->shaderHandle, "ProjMtx");
    imguiGLState->attribLocationPosition = glGetAttribLocation(imguiGLState->shaderHandle, "Position");
    imguiGLState->attribLocationUV = glGetAttribLocation(imguiGLState->shaderHandle, "UV");
    imguiGLState->attribLocationColor = glGetAttribLocation(imguiGLState->shaderHandle, "Color");

    glGenBuffers(1, &imguiGLState->vboHandle);
    glGenBuffers(1, &imguiGLState->elementsHandle);

    glGenVertexArrays(1, &imguiGLState->vaoHandle);
    glBindVertexArray(imguiGLState->vaoHandle);
    glBindBuffer(GL_ARRAY_BUFFER, imguiGLState->vboHandle);
    glEnableVertexAttribArray(imguiGLState->attribLocationPosition);
    glEnableVertexAttribArray(imguiGLState->attribLocationUV);
    glEnableVertexAttribArray(imguiGLState->attribLocationColor);

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
    glVertexAttribPointer(imguiGLState->attribLocationPosition, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, pos));
    glVertexAttribPointer(imguiGLState->attribLocationUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, uv));
    glVertexAttribPointer(imguiGLState->attribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, col));
#undef OFFSETOF

    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits for OpenGL3 demo because it is more likely to be compatible with user's existing shader.

    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGenTextures(1, &imguiGLState->fontTexture);
    glBindTexture(GL_TEXTURE_2D, imguiGLState->fontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    
    // Restore modified GL state
    glBindTexture(GL_TEXTURE_2D, last_texture);
    glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
    glBindVertexArray(last_vertex_array);
}

static ImGuiIO& beginImgui(platform_Input* input, ImguiGLState* imguiGLState) {
    ImGuiIO& io = ImGui::GetIO();
    if (!imguiGLState->ready) {
        init(io, imguiGLState);
        imguiGLState->ready = true;
    }
    if (input->reload) {
        unsigned char* pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    }
    io.Fonts->TexID = (void *)(intptr_t)(imguiGLState->fontTexture);
    renderGUIState = *imguiGLState;

    io.MousePos = ImVec2(input->mousePointers[0].finalX(), input->mousePointers[0].finalY());
    io.MouseDown[0] = input->mousePointers[0].button.isDown || BUTTON_WAS_PRESSED(input->mousePointers[0].button);
    io.MouseDown[1] = input->mousePointers[1].button.isDown || BUTTON_WAS_PRESSED(input->mousePointers[1].button);
    io.MouseDown[2] = input->mousePointers[2].button.isDown || BUTTON_WAS_PRESSED(input->mousePointers[2].button);
    io.MouseWheel = (float) input->mouseWheel;

    io.DisplaySize.x = (F32) input->windowWidth;
    io.DisplaySize.y = (F32) input->windowHeight;
    io.DeltaTime = 1.0f/60.0f;
    io.IniFilename = "ui.ini"; 
    io.RenderDrawListsFn = renderGUICallback;
    io.SetClipboardTextFn = input->setClipboardText;
    io.GetClipboardTextFn = input->getClipboardText;

    for (int i = 0; i < 300; i++) {
      io.KeysDown[i] = BUTTON_WAS_PRESSED(input->keyboardButtons[i]);
    }
    io.KeyCtrl = input->keyboardButtons[224].isDown || input->keyboardButtons[228].isDown;
    io.KeyShift = input->keyboardButtons[225].isDown || input->keyboardButtons[229].isDown;
    io.KeyAlt = input->keyboardButtons[226].isDown || input->keyboardButtons[230].isDown;

    io.KeyMap[ImGuiKey_Tab] = 43;
    io.KeyMap[ImGuiKey_LeftArrow] = 80;
    io.KeyMap[ImGuiKey_RightArrow] = 79;
    io.KeyMap[ImGuiKey_UpArrow] = 82;
    io.KeyMap[ImGuiKey_DownArrow] = 81;
    io.KeyMap[ImGuiKey_PageUp] = 75;
    io.KeyMap[ImGuiKey_PageDown] = 78;
    io.KeyMap[ImGuiKey_Home] = 74;
    io.KeyMap[ImGuiKey_End] = 77;
    io.KeyMap[ImGuiKey_Delete] = 76;
    io.KeyMap[ImGuiKey_Backspace] = 42;
    io.KeyMap[ImGuiKey_Enter] = 40;
    io.KeyMap[ImGuiKey_Escape] = 41;
    io.KeyMap[ImGuiKey_A] = 4;
    io.KeyMap[ImGuiKey_C] = 6;
    io.KeyMap[ImGuiKey_V] = 25;
    io.KeyMap[ImGuiKey_X] = 27;
    io.KeyMap[ImGuiKey_Y] = 28;
    io.KeyMap[ImGuiKey_Z] = 29;

    io.AddInputCharactersUTF8(input->textInput);

    return io;
}

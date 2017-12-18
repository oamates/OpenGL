#include <GL/glew.h>
#include <GLFW/glfw3.h>

#ifdef _WIN32
    #undef APIENTRY
    #define GLFW_EXPOSE_NATIVE_WIN32
    #define GLFW_EXPOSE_NATIVE_WGL
    #include <GLFW/glfw3native.h>
#endif

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ui_renderer.hpp"
#include "../make_unique.hpp"
#include "../rendering/render_window.hpp"

#include "log.hpp"

std::unique_ptr<ui_renderer_t::render_data_t> ui_renderer_t::render_data = nullptr;

void ui_renderer_t::RenderDrawList(ImDrawData* drawData)
{
    /* save current active shader program, texture binding, VAO and IBO bindings */
    GLint program_id; glGetIntegerv(GL_CURRENT_PROGRAM, &program_id);
    GLint tex_id; glGetIntegerv(GL_TEXTURE_BINDING_2D, &tex_id);
    GLint vao_id; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao_id);
    GLint vbo_id; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &vbo_id);
    GLint ibo_id; glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &ibo_id);

    /* save blend equation state */
    GLint blend_src;            glGetIntegerv(GL_BLEND_SRC, &blend_src);
    GLint blend_dst;            glGetIntegerv(GL_BLEND_DST, &blend_dst);
    GLint blend_equation_rgb;   glGetIntegerv(GL_BLEND_EQUATION_RGB, &blend_equation_rgb);
    GLint blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &blend_equation_alpha);

    /* save GL render state variables */
    GLboolean blend_state        = glIsEnabled(GL_BLEND);
    GLboolean cull_face_state    = glIsEnabled(GL_CULL_FACE);
    GLboolean depth_test_state   = glIsEnabled(GL_DEPTH_TEST);
    GLboolean scissor_test_state = glIsEnabled(GL_SCISSOR_TEST);

    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glActiveTexture(GL_TEXTURE0);

    // Handle cases of screen coordinates != from framebuffer coordinates (e.g. retina displays)
    ImGuiIO& io = ImGui::GetIO();
    float fb_height = io.DisplaySize.y * io.DisplayFramebufferScale.y;
    drawData->ScaleClipRects(io.DisplayFramebufferScale);

    // Setup orthographic projection matrix
    glm::mat4 projection_matrix = glm::mat4(
        glm::vec4( 2.0f / io.DisplaySize.x, 0.0f,                     0.0f, 0.0f),
        glm::vec4( 0.0f,                   -2.0f / io.DisplaySize.y,  0.0f, 0.0f),
        glm::vec4( 0.0f,                    0.0f,                    -1.0f, 0.0f),
        glm::vec4(-1.0f,                    1.0f,                     0.0f, 1.0f)
    );

    glUseProgram(render_data->shaderHandle);
    glUniform1i(render_data->attribLocationTex, 0);
    glUniformMatrix4fv(render_data->attribLocationProjMtx, 1, GL_FALSE, glm::value_ptr(projection_matrix));
    glBindVertexArray(render_data->vao_id);

    for (int index = 0; index < drawData->CmdListsCount; index++)
    {
        const ImDrawList* cmd_list = drawData->CmdLists[index];
        const ImDrawIdx* idx_buffer_offset = 0;
        glBindBuffer(GL_ARRAY_BUFFER, render_data->vbo_id);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.size() * sizeof(ImDrawVert), (GLvoid *)&cmd_list->VtxBuffer.front(), GL_STREAM_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_data->ibo_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx), (GLvoid *)&cmd_list->IdxBuffer.front(), GL_STREAM_DRAW);

        for (const ImDrawCmd* pcmd = cmd_list->CmdBuffer.begin(); pcmd != cmd_list->CmdBuffer.end(); pcmd++)
        {
            if (pcmd->UserCallback)
                pcmd->UserCallback(cmd_list, pcmd);
            else
            {
                glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
                glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
            }

            idx_buffer_offset += pcmd->ElemCount;
        }
    }

    /* restore OpenGL state */
    glUseProgram(program_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glBindVertexArray(vao_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);

    glBlendEquationSeparate(blend_equation_rgb, blend_equation_alpha);
    glBlendFunc(blend_src, blend_dst);

    if (!blend_state) glDisable(GL_BLEND);                      // disable GL_BLEND if it was disabled initially
    if (cull_face_state) glEnable(GL_CULL_FACE);                // enable GL_CULL_FACE if it was enabled initially
    if (depth_test_state) glEnable(GL_DEPTH_TEST);              // enable GL_DEPTH_TEST if it was enabled initially
    if (!scissor_test_state) glDisable(GL_SCISSOR_TEST);        // disable GL_SCISSOR_TEST if it was disabled initially
}

void ui_renderer_t::initialize(const render_window_t& active_window, bool instantCallbacks /* = true */)
{
    if (!render_data)
        render_data = std::make_unique<render_data_t>();

    render_data->window = active_window.Handler();
    ImGuiIO &io = ImGui::GetIO();    
    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;                     // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
    io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;
    io.RenderDrawListsFn = RenderDrawList;
    io.SetClipboardTextFn = SetClipboardText;
    io.GetClipboardTextFn = GetClipboardText;
  #ifdef _WIN32
    io.ImeWindowHandle = glfwGetWin32Window(render_data->window);
  #endif

    if (instantCallbacks)
    {
        glfwSetMouseButtonCallback(render_data->window, MouseButtonCallback);
        glfwSetScrollCallback(render_data->window, ScrollCallback);
        glfwSetKeyCallback(render_data->window, KeyCallback);
        glfwSetCharCallback(render_data->window, CharCallback);
    }

    int w, h, display_w, display_h;;
    glfwGetWindowSize(render_data->window, &w, &h);
    glfwGetFramebufferSize(render_data->window, &display_w, &display_h);
    io.DisplaySize.x = static_cast<float>(w);
    io.DisplaySize.y = static_cast<float>(h);
    io.DisplayFramebufferScale.x = static_cast<float>(display_w) / w;
    io.DisplayFramebufferScale.y = static_cast<float>(display_h) / h;
}

void ui_renderer_t::render()
{
    if (render_data->disabled)
        return;
    ImGui::Render();
}

void ui_renderer_t::new_frame()
{
    if (render_data->disabled)
        return;

    if (!render_data->fontTexture)
        CreateDeviceObjects();

    static ImGuiIO& io = ImGui::GetIO();

    if (glfwGetWindowAttrib(render_data->window, GLFW_RESIZABLE))
    {
        static int w, h, display_w, display_h;                              // setup display size (every frame to accommodate for window resizing)
        glfwGetWindowSize(render_data->window, &w, &h);
        glfwGetFramebufferSize(render_data->window, &display_w, &display_h);
        io.DisplaySize.x = static_cast<float>(w);
        io.DisplaySize.y = static_cast<float>(h);;
        io.DisplayFramebufferScale.x = static_cast<float>(display_w) / w;
        io.DisplayFramebufferScale.y = static_cast<float>(display_h) / h;
    }

    // setup time step
    double current_time = glfwGetTime();
    io.DeltaTime = render_data->time > 0.0 ? static_cast<float>(current_time - render_data->time) : static_cast<float>(1.0f / 60.0f);
    render_data->time = current_time;

    // setup inputs (we already got mouse wheel, keyboard keys & characters from glfw callbacks polled in glfwPollEvents())
    if (glfwGetWindowAttrib(render_data->window, GLFW_FOCUSED))
    {
        // Mouse position in screen coordinates (set to -1,-1 if no mouse / on another screen, etc.)
        double mouse_x, mouse_y;
        glfwGetCursorPos(render_data->window, &mouse_x, &mouse_y);
        io.MousePosPrev = io.MousePos;
        io.MousePos = ImVec2(static_cast<float>(mouse_x), static_cast<float>(mouse_y));
    }
    else
        io.MousePos = ImVec2(-1, -1);

    for (int i = 0; i < 3; i++)
    {
        io.MouseDown[i] = render_data->mousePressed[i] || glfwGetMouseButton(render_data->window, i) != 0;
        // If a mouse press event came, always pass it as "this frame", so we don't miss click-release events that are shorter than 1 frame.
        render_data->mousePressed[i] = false;
    }

    io.MouseWheel = render_data->mouseWheel;
    render_data->mouseWheel = 0.0f;

    // Hide OS mouse cursor if ImGui is drawing it
    glfwSetInputMode(render_data->window, GLFW_CURSOR, io.MouseDrawCursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
    ImGui::NewFrame();                                                      // Start the frame
}


void ui_renderer_t::CreateFontsTexture()
{
    ImGuiIO& io = ImGui::GetIO();
    unsigned char * pixels;                                                 // Build texture atlas
    int width, height;
    io.Fonts->AddFontFromFileTTF("fonts/DroidSans.ttf", 13);                // Load as RGBA 32-bits for OpenGL3 demo because it is more likely to be compatible with user's existing shader.
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    glGenTextures(1, &render_data->fontTexture);                               // Create OpenGL texture
    glBindTexture(GL_TEXTURE_2D, render_data->fontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    io.Fonts->TexID = (void *)(intptr_t)render_data->fontTexture;              // Store our identifier
}

void ui_renderer_t::CreateDeviceObjects()
{
    GLint tex_id, vbo_id, vao_id;                                           // Backup GL state
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &tex_id);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &vbo_id);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao_id);

    const GLchar* vs_source =
        "#version 330 core\n"
        "uniform mat4 projection_matrix;\n"
        "layout(location = 0) in vec2 position_in;\n"
        "layout(location = 1) in vec2 uv_in;\n"
        "layout(location = 2) in vec4 color_in;\n"
        "out vec2 uv;\n"
        "out vec4 color;\n"
        "void main()\n"
        "{\n"
        "   uv = uv_in;\n"
        "   color = color_in;\n"
        "   gl_Position = projection_matrix * vec4(position_in, 0.0f, 1.0f);\n"
        "}\n";
    
    const GLchar* fs_source =
        "#version 330 core\n"
        "uniform sampler2D font_tex;\n"
        "in vec2 uv;\n"
        "in vec4 color;\n"
        "out vec4 FragmentColor;\n"
        "void main()\n"
        "{\n"
        "   FragmentColor = color * texture(font_tex, uv);\n"
        "}\n";

    render_data->shaderHandle = glCreateProgram();
    render_data->vertHandle = glCreateShader(GL_VERTEX_SHADER);
    render_data->fragHandle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(render_data->vertHandle, 1, &vs_source, 0);
    glShaderSource(render_data->fragHandle, 1, &fs_source, 0);
    glCompileShader(render_data->vertHandle);
    glCompileShader(render_data->fragHandle);
    glAttachShader(render_data->shaderHandle, render_data->vertHandle);
    glAttachShader(render_data->shaderHandle, render_data->fragHandle);
    glLinkProgram(render_data->shaderHandle);
    render_data->attribLocationTex = glGetUniformLocation(render_data->shaderHandle, "font_tex");
    render_data->attribLocationProjMtx = glGetUniformLocation(render_data->shaderHandle, "projection_matrix");

    glGenBuffers(1, &render_data->vbo_id);
    glGenBuffers(1, &render_data->ibo_id);
    glGenVertexArrays(1, &render_data->vao_id);
    glBindVertexArray(render_data->vao_id);
    glBindBuffer(GL_ARRAY_BUFFER, render_data->vbo_id);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid *) offsetof(ImDrawVert, pos));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid *) offsetof(ImDrawVert, uv));
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid *) offsetof(ImDrawVert, col));

    CreateFontsTexture();
    glBindTexture(GL_TEXTURE_2D, tex_id);                                   // Restore modified GL state
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBindVertexArray(vao_id);
}

void ui_renderer_t::terminate()
{
    if (render_data->vao_id)
        glDeleteVertexArrays(1, &render_data->vao_id);

    if (render_data->vbo_id)
        glDeleteBuffers(1, &render_data->vbo_id);

    if (render_data->ibo_id)
        glDeleteBuffers(1, &render_data->ibo_id);

    render_data->vao_id = render_data->vbo_id = render_data->ibo_id = 0;
    glDetachShader(render_data->shaderHandle, render_data->vertHandle);
    glDeleteShader(render_data->vertHandle);
    render_data->vertHandle = 0;
    glDetachShader(render_data->shaderHandle, render_data->fragHandle);
    glDeleteShader(render_data->fragHandle);
    render_data->fragHandle = 0;
    glDeleteProgram(render_data->shaderHandle);
    render_data->shaderHandle = 0;

    if (render_data->fontTexture)
    {
        glDeleteTextures(1, &render_data->fontTexture);
        ImGui::GetIO().Fonts->TexID = 0;
        render_data->fontTexture = 0;
    }

    ImGui::Shutdown();
    delete render_data.release();
}

void ui_renderer_t::InvalidateDeviceObjects()
{
    if (render_data->vao_id)
        glDeleteVertexArrays(1, &render_data->vao_id);

    if (render_data->vbo_id)
        glDeleteBuffers(1, &render_data->vbo_id);

    if (render_data->ibo_id)
        glDeleteBuffers(1, &render_data->ibo_id);

    render_data->vao_id = render_data->vbo_id = render_data->ibo_id = 0;
    glDetachShader(render_data->shaderHandle, render_data->vertHandle);
    glDeleteShader(render_data->vertHandle);
    render_data->vertHandle = 0;
    glDetachShader(render_data->shaderHandle, render_data->fragHandle);
    glDeleteShader(render_data->fragHandle);
    render_data->fragHandle = 0;
    glDeleteProgram(render_data->shaderHandle);
    render_data->shaderHandle = 0;

    if (render_data->fontTexture)
    {
        glDeleteTextures(1, &render_data->fontTexture);
        ImGui::GetIO().Fonts->TexID = 0;
        render_data->fontTexture = 0;
    }

    ImGui::Shutdown();
}

void ui_renderer_t::MouseButtonCallback(GLFWwindow * window, int button, int action, int mods)
{
    ImGuiIO& io = ImGui::GetIO();
    if (action == GLFW_PRESS && button >= 0 && button < 3)
        render_data->mousePressed[button] = true;
    io.MouseClickedPos[button] = io.MousePos;
}

void ui_renderer_t::ScrollCallback(GLFWwindow * window, double xoffset, double yoffset)
    { render_data->mouseWheel += static_cast<float>(yoffset); }        // Use fractional mouse wheel, 1.0 unit 5 lines.

void ui_renderer_t::KeyCallback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
    ImGuiIO& io = ImGui::GetIO();

    if (action == GLFW_PRESS)
        io.KeysDown[key] = true;

    if (action == GLFW_RELEASE)
        io.KeysDown[key] = false;

    (void) mods;                                                    // Modifiers are not reliable across systems
    io.KeyCtrl  = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
    io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT]   || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
    io.KeyAlt   = io.KeysDown[GLFW_KEY_LEFT_ALT]     || io.KeysDown[GLFW_KEY_RIGHT_ALT];

    if (io.KeysDown[GLFW_KEY_H])
        render_data->disabled = !render_data->disabled;
}

void ui_renderer_t::CharCallback(GLFWwindow* window, unsigned int c)
{
    ImGuiIO& io = ImGui::GetIO();
    if (c > 0 && c < 0x10000)
        io.AddInputCharacter(static_cast<unsigned short>(c));
}

void ui_renderer_t::SetClipboardText(void* user_data, const char* text)
    { glfwSetClipboardString((GLFWwindow*)user_data, text); }

const char* ui_renderer_t::GetClipboardText(void* user_data)
    { return glfwGetClipboardString((GLFWwindow*) user_data); }


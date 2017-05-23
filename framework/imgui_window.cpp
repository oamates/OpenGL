//=======================================================================================================================================================================================================================
// GLFW + ImGui based application window structure
//=======================================================================================================================================================================================================================
#include <thread> 
#include <glm/gtc/type_ptr.hpp>

#include "glfw_window.hpp"
#include "imgui_window.hpp"

#include "log.hpp"
#include "image.hpp"

#ifdef _WIN32
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>
#endif

//=======================================================================================================================================================================================================================
// static, window-independent functions from GLFW3 library
//=======================================================================================================================================================================================================================
namespace glfw {

void imgui_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{   
    (void) mods;
    imgui_window_t* imgui_window = static_cast<imgui_window_t*> (glfwGetWindowUserPointer(window));

    //===================================================================================================================================================================================================================
    // screenshot maker data
    //===================================================================================================================================================================================================================
    static char file_name[20] = {"screenshot/0000.png"};
    static char hex_digit[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    static uint8_t* pixel_buffer = 0;
    static std::thread screenshot_thread;
    static unsigned short shot = 0;    

    if ((key == GLFW_KEY_PRINT_SCREEN) && (action == GLFW_RELEASE))
    {
        debug_msg("Saving screenshot #%d: %s.", shot, file_name);

        int res_x = imgui_window->res_x; 
        int res_y = imgui_window->res_y; 

        if (pixel_buffer) 
            screenshot_thread.join();
        else
            pixel_buffer = (unsigned char *) malloc (res_x * res_y * 3);

        glReadPixels(0, 0, res_x, res_y, GL_RGB, GL_UNSIGNED_BYTE, pixel_buffer);
        screenshot_thread = std::thread(image::png::write, file_name, res_x, res_y, pixel_buffer, PNG_COLOR_TYPE_RGB);

        ++shot;
        file_name[11] = hex_digit[(shot & 0xF000) >> 0xC];
        file_name[12] = hex_digit[(shot & 0x0F00) >> 0x8];
        file_name[13] = hex_digit[(shot & 0x00F0) >> 0x4];
        file_name[14] = hex_digit[(shot & 0x000F) >> 0x0];
        return;
    }

    //===================================================================================================================================================================================================================
    // ESCAPE key will switch input handler from application to imgui and back
    //===================================================================================================================================================================================================================
    if ((key == GLFW_KEY_ESCAPE) && (action == GLFW_RELEASE))
    {
        imgui_window->imgui_active = !imgui_window->imgui_active;
        //===============================================================================================================================================================================================================
        // Disable mouse cursor if ImGui is not active, and hide OS mouse cursor if ImGui is active and drawing it
        //===============================================================================================================================================================================================================
        int cursor_mode = GLFW_CURSOR_DISABLED;
        if (imgui_window->imgui_active)
        {
	        ImGuiIO& io = ImGui::GetIO();
	        cursor_mode = io.MouseDrawCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL;
        }
	    glfwSetInputMode(window, GLFW_CURSOR, cursor_mode);
        return;
    }

    if (imgui_window->imgui_active)
    {
        //===============================================================================================================================================================================================================
        // handle key input to ImGui
        //===============================================================================================================================================================================================================
        ImGuiIO& io = ImGui::GetIO();

        if (action == GLFW_PRESS)
            io.KeysDown[key] = true;

        if (action == GLFW_RELEASE)
            io.KeysDown[key] = false;

        io.KeyCtrl  = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
        io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT]   || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
        io.KeyAlt   = io.KeysDown[GLFW_KEY_LEFT_ALT]     || io.KeysDown[GLFW_KEY_RIGHT_ALT];
        io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER]   || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
        return;            
    }
    //===================================================================================================================================================================================================================
    // handle key input to application
    //===================================================================================================================================================================================================================
    imgui_window->on_key(key, scancode, action, mods);
}

void imgui_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    (void) mods;
    imgui_window_t* imgui_window = static_cast<imgui_window_t*> (glfwGetWindowUserPointer(window));

    if (imgui_window->imgui_active)
    {
        if ((action == GLFW_PRESS) && (button >= 0) && (button < 3))
            imgui_window->mouse_pressed[button] = true;
        return;
    }

    //===================================================================================================================================================================================================================
    // handle mouse button event to application
    //===================================================================================================================================================================================================================
    imgui_window->on_mouse_button(button, action, mods);
}

void imgui_mouse_move_callback(GLFWwindow* window, double x, double y)
{
    double t = glfw::time();
    imgui_window_t* imgui_window = static_cast<imgui_window_t*> (glfwGetWindowUserPointer(window));
    glm::dvec2 mouse_np = glm::dvec2(x, y);
    imgui_window->mouse_delta = mouse_np - imgui_window->mouse;
    imgui_window->mouse = mouse_np;
    imgui_window->mouse_dt = t - imgui_window->mouse_ts;
    imgui_window->mouse_ts = t;

    if (imgui_window->imgui_active) return;

    //===================================================================================================================================================================================================================
    // handle mouse move event to application, which can use new cursor position and dx, dy, dt to update camera
    //===================================================================================================================================================================================================================
    imgui_window->on_mouse_move();
}

void imgui_cursor_enter_callback(GLFWwindow* window, int enter)
{
    imgui_window_t* imgui_window = static_cast<imgui_window_t*> (glfwGetWindowUserPointer(window));
    imgui_window->on_cursor_enter(enter);
}

void imgui_character_callback(GLFWwindow* window, unsigned int codepoint)
{
    imgui_window_t* imgui_window = static_cast<imgui_window_t*> (glfwGetWindowUserPointer(window));

    if (imgui_window->imgui_active)
    {
        //===============================================================================================================================================================================================================
        // handle key input to ImGui
        //===============================================================================================================================================================================================================
        ImGuiIO& io = ImGui::GetIO();
        if ((codepoint > 0) && (codepoint < 0x10000))
            io.AddInputCharacter((unsigned short) codepoint);
        return;
    }

    //===================================================================================================================================================================================================================
    // handle character input to application
    //===================================================================================================================================================================================================================
    imgui_window->on_character(codepoint);
}

void imgui_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    imgui_window_t* imgui_window = static_cast<imgui_window_t*> (glfwGetWindowUserPointer(window));

    if (imgui_window->imgui_active)
    {
        imgui_window->mouse_wheel += yoffset;
        return;
    }
    //===================================================================================================================================================================================================================
    // handle scroll input to application
    //===================================================================================================================================================================================================================
    imgui_window->on_scroll(xoffset, yoffset);
}

void imgui_resize_callback(GLFWwindow* window, int width, int height) 
{ 
    imgui_window_t* imgui_window = static_cast<imgui_window_t*> (glfwGetWindowUserPointer(window));
    imgui_window->on_resize(width, height);
}

const char* GetClipboardText(void* user_data)
    { return glfwGetClipboardString((GLFWwindow*) user_data); }

void SetClipboardText(void* user_data, const char* text)
    { glfwSetClipboardString((GLFWwindow*)user_data, text); }

//=======================================================================================================================================================================================================================
// This is the main rendering function that is provided to ImGui via setting RenderDrawListsFn field in the ImGuiIO structure
// It alters :: 
//  - GL_CULL_FACE, GL_DEPTH_TEST settings 
//  - GL_BLEND settings, glBlendEquation and glBlendFunc 
//  - GL_SCISSOR_TEST is disabled on function exit  
//=======================================================================================================================================================================================================================
static void RenderDrawLists(ImDrawData* draw_data)
{
    //===================================================================================================================================================================================================================
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    //===================================================================================================================================================================================================================
    ImGuiIO& io = ImGui::GetIO();
    int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fb_width == 0 || fb_height == 0) return;
    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    // glActiveTexture(GL_TEXTURE0);
        
    //===================================================================================================================================================================================================================
    // Setup viewport
    //===================================================================================================================================================================================================================
    glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
    
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawIdx* idx_buffer_offset = 0;

        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);
    
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                // glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
                glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
            }
            idx_buffer_offset += pcmd->ElemCount;
        }
    }    
}

} // namespace glfw



//=======================================================================================================================================================================================================================
// GLFW + ImGui window constructor + GLEW library initialization
// the application will be terminated if one of the library fails to initialize or 
// requested context version is not supported by OpenGL driver
//=======================================================================================================================================================================================================================
imgui_window_t::imgui_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen, bool debug_context)
{
    frame = 0;
    imgui_window_t::res_x = res_x;
    imgui_window_t::res_y = res_y;

    //===================================================================================================================================================================================================================
    // Set the number of antialiasing samples, OpenGL major and minor context version and
    // forward compatible core profile (deprecated functions not to be supported)
    //===================================================================================================================================================================================================================
    glfwWindowHint(GLFW_SAMPLES, glfw_samples);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, version_major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, version_minor);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);                                                                    
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if (debug_context)
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    //===================================================================================================================================================================================================================
    // Create window ...
    //===================================================================================================================================================================================================================
    window = glfwCreateWindow(res_x, res_y, title, fullscreen ? glfwGetPrimaryMonitor() : 0, 0); 
    if(!window)
    {
        glfw::terminate();
        exit_msg("Failed to open GLFW window. No open GL %d.%d support.", version_major, version_minor);
    }

    //===================================================================================================================================================================================================================
    // ... and make it current
    //===================================================================================================================================================================================================================
    glfwMakeContextCurrent(window);
    debug_msg("GLFW initialization done ... ");

    //===================================================================================================================================================================================================================
    // GLEW library initialization
    //===================================================================================================================================================================================================================
    glewExperimental = true;                                                                                                // needed in core profile 
    GLenum result = glewInit();                                                                                             // initialise GLEW
    if (result != GLEW_OK) 
    {
        glfw::terminate();
        exit_msg("Failed to initialize GLEW : %s", glewGetErrorString(result));
    }
    debug_msg("GLEW library initialization done ... ");

    //===================================================================================================================================================================================================================
    // enable debug output and set up callback routine
    //===================================================================================================================================================================================================================
    if (debug_context)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(debugCallback, 0);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, true);
    }

    //===================================================================================================================================================================================================================
    // mouse and keyboard event callback functions
    //===================================================================================================================================================================================================================
    glfwSetCursorPos(window, 0.5 * res_x, 0.5 * res_y);
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, glfw::imgui_key_callback);
    glfwSetMouseButtonCallback(window, glfw::imgui_mouse_button_callback);
    glfwSetCursorPosCallback(window, glfw::imgui_mouse_move_callback);
    glfwSetCursorEnterCallback(window, glfw::imgui_cursor_enter_callback);
    glfwSetCharCallback(window, glfw::imgui_character_callback);
    glfwSetScrollCallback(window, glfw::imgui_scroll_callback);

    //===================================================================================================================================================================================================================
    // time and mouse state variables
    //===================================================================================================================================================================================================================
    mouse_pressed[0] = mouse_pressed[1] = mouse_pressed[2] = false;
    initial_ts = mouse_ts = frame_ts = glfw::time();
    mouse_delta = mouse = glm::dvec2(0.0);
    frame_dt = mouse_dt = 0.0;

    //===================================================================================================================================================================================================================
    // ImGui initialization
    //===================================================================================================================================================================================================================
    ImGuiIO& io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;                         // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
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
    
    io.RenderDrawListsFn = glfw::RenderDrawLists;       // Alternatively you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.
    io.SetClipboardTextFn = glfw::SetClipboardText;
    io.GetClipboardTextFn = glfw::GetClipboardText;
    io.ClipboardUserData = window;

  #ifdef _WIN32
    io.ImeWindowHandle = glfwGetWin32Window(window);
  #endif

    // Load Fonts (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
    //ImGuiIO& io = ImGui::GetIO();
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyClean.ttf", 13.0f);
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyTiny.ttf", 10.0f);
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());

    //===================================================================================================================================================================================================================
    // OpenGL objects for UI rendering
    //===================================================================================================================================================================================================================

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
        "	uv = uv_in;\n"
        "	color = color_in;\n"
        "	gl_Position = projection_matrix * vec4(position_in, 0.0f, 1.0f);\n"
        "}\n";
    
    const GLchar* fs_source =
        "#version 330 core\n"
        "uniform sampler2D font_tex;\n"
        "in vec2 uv;\n"
        "in vec4 color;\n"
        "out vec4 FragmentColor;\n"
        "void main()\n"
        "{\n"
        "	FragmentColor = color * texture(font_tex, uv);\n"
        "}\n";
    
    ui_program_id = glCreateProgram();
    ui_vs_id = glCreateShader(GL_VERTEX_SHADER);
    ui_fs_id = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(ui_vs_id, 1, &vs_source, 0);
    glShaderSource(ui_fs_id, 1, &fs_source, 0);
    glCompileShader(ui_vs_id);
    glCompileShader(ui_fs_id);
    glAttachShader(ui_program_id, ui_vs_id);
    glAttachShader(ui_program_id, ui_fs_id);
    glLinkProgram(ui_program_id);
    
    glUseProgram(ui_program_id);
    glUniform1i(glGetUniformLocation(ui_program_id, "font_tex"), 0);
    uni_projection_matrix_id = glGetUniformLocation(ui_program_id, "projection_matrix");
    
    glGenVertexArrays(1, &ui_vao_id);
    glBindVertexArray(ui_vao_id);
    glGenBuffers(1, &ui_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, ui_vbo_id);
    glGenBuffers(1, &ui_ibo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ui_ibo_id);
    
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (const GLvoid*) offsetof(ImDrawVert, pos));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (const GLvoid*) offsetof(ImDrawVert, uv));
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (const GLvoid*) offsetof(ImDrawVert, col));

    //===================================================================================================================================================================================================================
    //===================================================================================================================================================================================================================

    //===================================================================================================================================================================================================================
    // create font texture and build texture atlas
    // load as RGBA 32-bits (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders.
    // If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.
    //===================================================================================================================================================================================================================
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   
    
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &ui_font_tex_id);
    glBindTexture(GL_TEXTURE_2D, ui_font_tex_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    
    io.Fonts->TexID = (void*)(intptr_t)ui_font_tex_id;
    imgui_active = false;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

}

void imgui_window_t::new_frame()
{
    //===================================================================================================================================================================================================================
    // Setup time step
    //===================================================================================================================================================================================================================
    ImGuiIO& io = ImGui::GetIO();
    double current_ts = glfw::time();
    frame_dt = current_ts - frame_ts;
    io.DeltaTime = frame_dt;
    frame_ts = current_ts;

    glfw::poll_events();

    if (imgui_active)
    {

        //===============================================================================================================================================================================================================
        // Setup display size (every frame to accommodate for window resizing)
        //===============================================================================================================================================================================================================
        int w, h;
        int display_w, display_h;
        glfwGetWindowSize(window, &w, &h);
        glfwGetFramebufferSize(window, &display_w, &display_h);
        io.DisplaySize = ImVec2((float)w, (float)h);
        io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);

        //===============================================================================================================================================================================================================
        // Setup inputs ::
        //  - we already got mouse wheel, keyboard keys & characters from glfw callbacks polled in glfwPollEvents())
        //  - mouse position in screen coordinates (set to -1,-1 if no mouse / window is not in focus)
        //  - if a mouse press event came, always pass it as if mouse held this frame, so we don't miss click-release events that are shorter than 1 frame
        //===============================================================================================================================================================================================================
        io.MousePos = (glfwGetWindowAttrib(window, GLFW_FOCUSED)) ? ImVec2((float)mouse.x, (float)mouse.y) : ImVec2(-1.0f, -1.0f);  // 

        for (int i = 0; i < 3; i++)
        {
            io.MouseDown[i] = mouse_pressed[i] || (glfwGetMouseButton(window, i) != 0);    
            mouse_pressed[i] = false;
        }

        io.MouseWheel = mouse_wheel;
        mouse_wheel = 0.0;

        ImGui::NewFrame();
        update_ui();
    }    
}

void imgui_window_t::end_frame()
{
    if (imgui_active)
    {
        ImGuiIO& io = ImGui::GetIO();

        //===============================================================================================================================================================================================================
        // Setup render state :: alpha-blending enabled, no face culling, no depth testing, scissor enabled :: we do not care restoring these settings 
        //===============================================================================================================================================================================================================
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_SCISSOR_TEST);

        //===============================================================================================================================================================================================================
        // Bind vertex and index buffers as ImGui::Render() is going to modify their data
        //===============================================================================================================================================================================================================
        glBindVertexArray(ui_vao_id);
        glBindBuffer(GL_ARRAY_BUFFER, ui_vbo_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ui_ibo_id);

        glm::mat4 projection_matrix = glm::mat4(
            glm::vec4( 2.0f / io.DisplaySize.x, 0.0f,                     0.0f, 0.0f),
            glm::vec4( 0.0f,                    2.0f / -io.DisplaySize.y, 0.0f, 0.0f),
            glm::vec4( 0.0f,                    0.0f,                    -1.0f, 0.0f),
            glm::vec4(-1.0f,                    1.0f,                     0.0f, 1.0f)
        );
    
        glUseProgram(ui_program_id);
        glUniformMatrix4fv(uni_projection_matrix_id, 1, GL_FALSE, glm::value_ptr(projection_matrix));

        ImGui::Render();
    }

    frame++;
    glfwSwapBuffers(window);
}

void imgui_window_t::set_title(const char* title)
    { glfwSetWindowTitle(window, title); }

void imgui_window_t::set_should_close(int value)
    { glfwSetWindowShouldClose(window, value); }

bool imgui_window_t::should_close()
    { return glfwWindowShouldClose(window); }

double imgui_window_t::aspect()
    { return double(res_x) / double(res_y); }

double imgui_window_t::inv_aspect()
    { return double(res_y) / double(res_x); }


void imgui_window_t::disable_cursor()
    { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); }

void imgui_window_t::enable_cursor()
    { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); }

int imgui_window_t::key_state(int key)
    { return glfwGetKey(window, key); }

double imgui_window_t::fps()
    { return double(frame) / (frame_ts - initial_ts); };

glm::dvec2 imgui_window_t::cursor_position()
{
    glm::dvec2 pos;
    glfwGetCursorPos(window, &pos.x, &pos.y);
    return pos;
}

imgui_window_t::~imgui_window_t()
{
    glDeleteBuffers(1, &ui_ibo_id);
    glDeleteBuffers(1, &ui_vbo_id);
    glDeleteVertexArrays(1, &ui_vao_id);

    glDetachShader(ui_program_id, ui_vs_id);
    glDeleteShader(ui_vs_id);

    glDetachShader(ui_program_id, ui_fs_id);
    glDeleteShader(ui_fs_id);

    glDeleteProgram(ui_program_id);

    glDeleteTextures(1, &ui_font_tex_id);
    ImGui::GetIO().Fonts->TexID = 0;
    ImGui::Shutdown();

    glfwDestroyWindow(window);
}




#pragma once

#include <memory>
#include "imgui/imgui.h"

struct render_window_t;

// Main class to implement custom interfaces setups IMGUI to render in the current context
struct ui_renderer_t
{
    ui_renderer_t() {}
    virtual ~ui_renderer_t() {}

    // Setups the interface to render in the current rendering context
    static void initialize(const render_window_t& active_window, bool instantCallbacks = true);
    
    static void terminate();                                // Deletes all rendering objects and UI data
    static void render();                                   // Renders the UI
    static void new_frame();                                // Sets up the GUI for a new frame.

	// Contains all the OpenGL objects names and useful fields for callback logic.

	struct render_data_t
    {
        GLFWwindow* window;
        double time;
        bool mousePressed[3];
        float mouseWheel;
        GLuint fontTexture;
        int shaderHandle;
        int vertHandle;
        int fragHandle;
        int attribLocationTex;
        int attribLocationProjMtx;
        unsigned int vao_id;
        unsigned int vbo_id;
        unsigned int ibo_id;
        bool disabled;

        render_data_t()
        {
            window = nullptr;
            time = 0.0f;
            mousePressed[0] = mousePressed[1] = mousePressed[2] = false;
            mouseWheel = 0.0f;
            fontTexture = 0;
            shaderHandle = 0, vertHandle = 0, fragHandle = 0;
            attribLocationTex = 0, attribLocationProjMtx = 0;
            vao_id = 0, vbo_id = 0, ibo_id = 0;
            disabled = false;
        }
    };

    static std::unique_ptr<render_data_t> render_data;
    static void InvalidateDeviceObjects();
    static void CreateFontsTexture();
    static void CreateDeviceObjects();

    static void RenderDrawList(ImDrawData* drawData);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void CharCallback(GLFWwindow* window, unsigned int c);

    static void SetClipboardText(void* user_data, const char * text);
    static const char* GetClipboardText(void* user_data);
};
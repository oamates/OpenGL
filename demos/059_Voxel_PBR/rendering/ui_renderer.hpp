#pragma once

#include <memory>
#include "imgui/imgui.h"

struct RenderWindow;

// Main class to implement custom interfaces setups IMGUI to render in the current context
struct ui_renderer_t
{
    ui_renderer_t() {}
    virtual ~ui_renderer_t() {}

    // Setups the interface to render in the current rendering context
    static void Initialize(const RenderWindow &activeWindow, bool instantCallbacks = true);
    
    static void Terminate();                                // Deletes all rendering objects and UI data
    static void Render();                                   // Renders the UI
    static void NewFrame();                                 // Sets up the GUI for a new frame.

	// Contains all the OpenGL objects names and useful fields for callback logic.

	struct RendererData
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
        int attribLocationPosition;
        int attribLocationUV;
        int attribLocationColor;
        unsigned int vboHandle;
        unsigned int vaoHandle;
        unsigned int elementsHandle;
        bool disabled;

        RendererData()
        {
            window = nullptr;
            time = 0.0f;
            mousePressed[0] = mousePressed[1] = mousePressed[2] = false;
            mouseWheel = 0.0f;
            fontTexture = 0;
            shaderHandle = 0, vertHandle = 0, fragHandle = 0;
            attribLocationTex = 0, attribLocationProjMtx = 0;
            attribLocationPosition = 0, attribLocationUV = 0,
            attribLocationColor = 0;
            vboHandle = 0, vaoHandle = 0, elementsHandle = 0;
            disabled = false;
        }
    };

    static std::unique_ptr<RendererData> renderer;
    static void InvalidateDeviceObjects();
    static void CreateFontsTexture();
    static void CreateDeviceObjects();

    static void RenderDrawList(ImDrawData * drawData);
    static void MouseButtonCallback(GLFWwindow * window, int button, int action, int mods);
    static void ScrollCallback(GLFWwindow * window, double xoffset, double yoffset);
    static void KeyCallback(GLFWwindow * window, int key, int scancode, int action, int mods);
    static void CharCallback(GLFWwindow * window, unsigned int c);

    static void SetClipboardText(void* user_data, const char * text);
    static const char* GetClipboardText(void* user_data);
};
#pragma once

struct GLFWwindow;

namespace FG_3DShaderProg
{
    bool Init(GLFWwindow* w);
    void UpdateCameraInput();
    void RebuildTerrain();
    void Render();
    void RenderUI();
    void Tick(float dt);
    void Shutdown();
}

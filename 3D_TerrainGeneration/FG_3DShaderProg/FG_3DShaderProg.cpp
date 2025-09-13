#include "FG_3DShaderProg.h"

#include "imgui.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader.h"
#include "camera.h"
#include "brute_force.h"
#include "Geomipmapping.h"
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <algorithm>
#include <string>
#include <limits>
#include <map>

enum EAlgorithm
{
    NONE = 0,
    RAW_HEIGHTMAP,
    FAULT_FORMATION,
    MIDPOINT_DISPLACEMENT
};

enum EOptimization
{
    BRUTE_FORCE = 0,
    GEOMIPMAPPING
};

namespace FG_3DShaderProg
{
    static void HelpMarker(const char* desc)
    {
        ImGui::TextDisabled("(?)");
        if (ImGui::BeginItemTooltip())
        {
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

    GLFWwindow* window = nullptr;
    ImGuiIO* IO = nullptr;

    Camera camera(glm::vec3(0.f, 70.f, 140.f));

    float deltaTime = 0.f;	// Time between current frame and last frame
    float lastFrame = 0.f; // Time of last frame

    Shader* terrainShader = nullptr;
    Shader* wireShader = nullptr;

    Brute_Force bruteTerrain;
    Geomipmapping geomipTerrain(3);
    Terrain* terrain = &bruteTerrain;

    bool wireframe = false;

    EAlgorithm algorithm = EAlgorithm::NONE;
    EOptimization optimization = EOptimization::BRUTE_FORCE; //EOptimization::BRUTE_FORCE;

    int size = 128;
    //int size = 50;
    float heightScale = 0.25f;

    int patchSizeUI = 3;
    int k = 1;

    /*std::map<ETileType, char> tilePaths = {
        {ETileType::DIRT, "T_Dirt.png"},
        {ETileType::GRASS, "T_Grass.png"},
        {ETileType::ROCK, "T_Rock.png"},
        {ETileType::SNOW_TIP, "T_snow_mountain.png"}
    };*/

    //RAW_HEIGHTMAP
    char path[256] = "height128.raw";
    //char path[256] = "heightmap_50.raw";

    //FAULT_FORMATION
    int iterations = 100;
    int minDelta = 0;
    int maxDelta = 255;
    float filter = 0.1f;

    //MIDPOINT_DISPLACEMENT
    int n = 7;
    int seed = 777;
    float amplitude = 1.f;
    float factor = 1.5f;

    //LIGHTNING
    struct LightUI
    {
        glm::vec3 position = glm::vec3(0.f, 70.f, 140.f); //glm::vec3(200.0f, 300.0f, 200.0f)
        glm::vec3 ambient = glm::vec3(0.15f); // 0-1
        glm::vec3 diffuse = glm::vec3(0.85f); // 0-1
        glm::vec3 specular = glm::vec3(1.f); // 0-1

        bool  followCamera = false;
        bool blinn = false;
        bool blinnKeyPressed = false;
        bool inverse_normals = false;
    } light;

    struct MaterialUI
    {
        glm::vec3 ambient = glm::vec3(1.f);
        glm::vec3 diffuse = glm::vec3(1.f);
        glm::vec3 specular = glm::vec3(1.f);

        float shininess = 32.0f; // 1-256
    } material;


    bool Init(GLFWwindow* w)
    {
        window = w;
        IO = &ImGui::GetIO();

        lastFrame = static_cast<float>(glfwGetTime());

        glEnable(GL_DEPTH_TEST);

        terrainShader = new Shader("shader.vs", "shader.fs");
        wireShader = new Shader("wireframe.vs", "wireframe.fs");

        if (!terrain->LoadHeightMap(path, size))
        {
            std::cout << "Failed to load heightmap at init: " << path << std::endl;
            return false;
        }

        terrain->SetHeightScale(heightScale);
        terrain->BuildTerrain();

        return true;
    }

    void UpdateCameraInput()
    {
        if (!window || !IO) return;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        //cameraInput
        if (!IO->WantCaptureKeyboard)
        {
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                camera.ProcessKeyboard(FORWARD, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                camera.ProcessKeyboard(BACKWARD, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                camera.ProcessKeyboard(LEFT, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                camera.ProcessKeyboard(RIGHT, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
                camera.ProcessKeyboard(UP, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
                camera.ProcessKeyboard(DOWN, deltaTime);



            if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !light.blinnKeyPressed)
            {
                light.blinn = !light.blinn;
                light.blinnKeyPressed = true;
            }
            if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)
            {
                light.blinnKeyPressed = false;
            }
        }

        // mouse
        if (!IO->WantCaptureMouse && ImGui::IsMouseDown(ImGuiMouseButton_Right))
        {
            //std::cout << "Camera Rotate: " << std::endl;
            ImVec2 d = IO->MouseDelta;
            camera.ProcessMouseMovement(d.x, -d.y);
        }

        if (!IO->WantCaptureMouse && IO->MouseWheel != 0.0f)
        {
            //std::cout << "Camera Zoom: " << std::endl;
            camera.ProcessMouseScroll(IO->MouseWheel * 2.0f);
        }
    }

    void RebuildTerrain()
    {
        switch (optimization)
        {
            case EOptimization::BRUTE_FORCE:
                terrain = &bruteTerrain;
                break;
            case EOptimization::GEOMIPMAPPING:
                terrain = &geomipTerrain;
                geomipTerrain.SetPatchSize(patchSizeUI);
                break;
        }
        switch (algorithm)
        {
            case EAlgorithm::RAW_HEIGHTMAP:
                if (!terrain->LoadHeightMap(path, size))
                {
                    std::cout << "Failed to load heightmap: " << path << std::endl;
                    break;
                }
                terrain->SetHeightScale(heightScale);
                terrain->BuildTerrain();
                break;
            case EAlgorithm::FAULT_FORMATION:
                terrain->GenerateFaultFormation(size, iterations, minDelta, maxDelta, filter);
                terrain->SetHeightScale(heightScale);
                terrain->BuildTerrain();
                break;
            case EAlgorithm::MIDPOINT_DISPLACEMENT:
                terrain->GenerateMidpointDisplacement(n, seed, amplitude, factor);
                terrain->SetHeightScale(heightScale);
                terrain->BuildTerrain();
                break;
                
            default:
                break;
        }
    }

    void Render()
    {
        if (!terrainShader) return;

        int screenWidth = 0;
        int screenHeight = 0;
        glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
        if (screenWidth <= 0 || screenHeight <= 0) return;

        /*glViewport(0, 0, screenWidth, screenHeight);
        glClearColor(0.12f, 0.10f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);*/

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        terrainShader->use();
        terrainShader->setInt("terrainTex",0);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)screenWidth / (float)screenHeight, 0.1f, 2000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model(1.0f);

        terrainShader->setMat4("projection", projection);
        terrainShader->setMat4("view", view);
        terrainShader->setMat4("model", model);

        //Lighting 
        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

        glm::vec3 lightPos = light.followCamera ? camera.Position : light.position;

        terrainShader->setBool("useBlinn", light.blinn);
        terrainShader->setVec3("light.position", lightPos);
        terrainShader->setVec3("light.ambient", light.ambient);
        terrainShader->setVec3("light.diffuse", light.diffuse);
        terrainShader->setVec3("light.specular", light.specular);

        terrainShader->setInt("inverse_normals", light.inverse_normals);

        terrainShader->setVec3("material.ambient", material.ambient);
        terrainShader->setVec3("material.diffuse", material.diffuse);
        terrainShader->setVec3("material.specular", material.specular);
        terrainShader->setFloat("material.shininess", material.shininess);

        terrainShader->setVec3("viewPos", camera.Position);

        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


        terrain->Render(camera);

        if (wireframe)
        {
            if (!wireShader) return;
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            wireShader->use();
            wireShader->setMat4("view", view);
            wireShader->setMat4("projection", projection);
            wireShader->setMat4("model", model);

            terrain->Render(camera);
        }
    }

    void RenderUI()
    {
        ImGui::Begin("Terrain Controls");

        //IMGUI_DEMO_MARKER("Widgets/Basic/SliderInt, SliderFloat");
        ImGui::TextUnformatted("Algorithm:");

        static int chosenOptimization = 0;
        ImGui::RadioButton("Brute Force", &chosenOptimization, 0); ImGui::SameLine();
        ImGui::RadioButton("Geomipmapping", &chosenOptimization, 1);

        static int chosenAlgorithm = 1;

        ImGui::BeginDisabled(chosenOptimization == 1);
        ImGui::RadioButton("RAW Heightmap", &chosenAlgorithm, 1); ImGui::SameLine();
        ImGui::EndDisabled();
        ImGui::RadioButton("Fault Formation", &chosenAlgorithm, 2); ImGui::SameLine();
        ImGui::RadioButton("Midpoint Displacement", &chosenAlgorithm, 3);

        ImGui::Separator();
        ImGui::Checkbox("Wireframe", &wireframe);
        //ImGui::InputFloat("Height scale", &heightScale, 0.01f, 0.1f, "%.3f");
        ImGui::SliderFloat("Height scale", &heightScale, 0.0f, 1.0f, "%.3f");
        ImGui::SameLine(); HelpMarker("CTRL+click to input value.");


        /*ImGui::DragInt("drag int 0..100", &i2, 1, 0, 100, "%d%%", ImGuiSliderFlags_AlwaysClamp);
        ImGui::DragInt("drag int wrap 100..200", &i3, 1, 100, 200, "%d", ImGuiSliderFlags_WrapAround);*/

        switch (chosenOptimization)
        {
            case 0:
                optimization = EOptimization::BRUTE_FORCE;
                break;
            case 1:
                optimization = EOptimization::GEOMIPMAPPING;
                break;
        }

        switch (chosenAlgorithm)
        {
            case 1:
                algorithm = EAlgorithm::RAW_HEIGHTMAP;
                ImGui::SliderInt("Size", &size, 2, 4096, "%d", ImGuiSliderFlags_AlwaysClamp);
                ImGui::InputText("Heightmap file", path, IM_ARRAYSIZE(path));

                if (ImGui::Button("Generate (RAW Heightmap)"))
                    RebuildTerrain();
                break;
            case 2:
                algorithm = EAlgorithm::FAULT_FORMATION;
                if (optimization == EOptimization::BRUTE_FORCE)
                {
                    ImGui::SliderInt("Size", &size, 2, 4096, "%d", ImGuiSliderFlags_AlwaysClamp);
                }
                else
                {
                    size = static_cast<int>(std::pow(2, n) + 1);
                    if (ImGui::SliderInt("n", &n, 1, 11, "%d", ImGuiSliderFlags_AlwaysClamp))
                    {
                        size = std::max(2, static_cast<int>(std::pow(2, n) + 1));
                        if (k > n)
                        {
                            k = n;
                        }
                    }
                    ImGui::SameLine();
                    ImGui::Text(" | Size = 2^n + 1 = % d", size);

                    if (optimization == EOptimization::GEOMIPMAPPING)
                    {
                        patchSizeUI = static_cast<int>(std::pow(2, k) + 1);
                        if (ImGui::SliderInt("k", &k, 1, 11, "%d", ImGuiSliderFlags_AlwaysClamp))
                        {
                            patchSizeUI = std::max(2, static_cast<int>(std::pow(2, k) + 1));
                            if (k > n)
                            {
                                k = n;
                            }
                        }
                        ImGui::SameLine();
                        ImGui::Text(" | PatchSize = 2^k + 1 = % d", patchSizeUI);
                    }
                }
                ImGui::SliderInt("Iterations", &iterations, 1, 10000, "%d", ImGuiSliderFlags_AlwaysClamp);
                ImGui::SliderFloat("Filter", &filter, 0.0f, 1.0f, "%.3f");
                ImGui::SameLine(); HelpMarker("CTRL+click to input value.");
                iterations = std::max(1, iterations);

                if (ImGui::Button("Generate (Fault Formation)"))
                    RebuildTerrain();
                break;
            case 3:
                algorithm = EAlgorithm::MIDPOINT_DISPLACEMENT;
                size = static_cast<int>(std::pow(2, n) + 1);
                if (ImGui::SliderInt("n", &n, 1, 11, "%d", ImGuiSliderFlags_AlwaysClamp))
                {
                    size = std::max(2, static_cast<int>(std::pow(2, n) + 1));
                    if (k > n)
                    {
                        k = n;
                    }
                }
                ImGui::SameLine();
                ImGui::Text(" | Size = 2^n + 1 = % d", size);

                if (optimization == EOptimization::GEOMIPMAPPING)
                {
                    patchSizeUI = static_cast<int>(std::pow(2, k) + 1);
                    if (ImGui::SliderInt("k", &k, 1, 11, "%d", ImGuiSliderFlags_AlwaysClamp))
                    {
                        patchSizeUI = std::max(2, static_cast<int>(std::pow(2, k) + 1));
                        if (k > n)
                        {
                            k = n;
                        }
                    }
                    ImGui::SameLine();
                    ImGui::Text(" | PatchSize = 2^k + 1 = % d", patchSizeUI);
                }

                //ImGui::InputInt("RNG Seed", &seed);
                //ImGui::DragInt("RNG Seed", &seed, 1, 1, INT_MAX, "%d", ImGuiSliderFlags_AlwaysClamp);
                ImGui::SliderInt("RNG Seed", &seed, 1, INT_MAX / 2, "%d", ImGuiSliderFlags_AlwaysClamp); //SliderBehavior Assertion failed: *(const ImS32*)p_min >= IM_S32_MIN / 2 && *(const ImS32*)p_max <= IM_S32_MAX / 2, file imgui_widgets.cpp, line 3254
                //ImGui::InputFloat("Noise Amplitude", &amplitude);
                ImGui::SliderFloat("Noise Amplitude", &amplitude, 0.0f, 1.0f, "%.3f"); //FLT_MAX / 2.0f
                ImGui::SliderFloat("Factor", &factor, 0.0f, 5.0f, "%.3f");
                ImGui::SameLine(); HelpMarker("CTRL+click to input value.");

                if (ImGui::Button("Generate (Midpoint Displacement)"))
                    RebuildTerrain();
                break;
            default:
                break;
        }

        ImGui::SeparatorText("Camera");
        ImGui::Text("WASD/QE move, RMB drag to look");
        ImGui::SliderFloat("FOV", &camera.Zoom, 20.0f, 90.0f, "%.0f deg");
        ImGui::SliderFloat("Speed", &camera.MovementSpeed, 1.0f, 100.0f, "%.1f");

        ImGui::SeparatorText("Debug");
        ImGui::Text("Cam pos: (%.1f, %.1f, %.1f)", camera.Position.x, camera.Position.y, camera.Position.z);
        ImGui::Text("DeltaTime: %.3f ms (%.1f FPS)", deltaTime * 1000.0f, 1.0f / std::max(1e-6f, deltaTime));
        ImGui::Text("Size: %d  HeightScale: %.2f", size, heightScale);

        if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen))
        {
            float posMin = -static_cast<float>(size);
            float posMax = static_cast<float>(size);
            float posYMin = 0.f;
            float posYMax = static_cast<float>(size) * 2.f;

            ImGui::Checkbox("Use Blinn (B)", &light.blinn);
            if (light.blinn)
            {
                ImGui::SameLine();
                ImGui::Text(" | TRUE");
            }
            else
            {
                ImGui::SameLine();
                ImGui::Text(" | FALSE");
            }
            ImGui::Checkbox("Head-light (follow camera)", &light.followCamera);

            ImGui::BeginDisabled(light.followCamera);
            ImGui::SliderFloat("Light Y", &light.position.y, posYMin, posYMax, "%.1f");
            ImGui::SliderFloat2("Light XZ", &light.position.x, posMin, posMax, "%.1f");
            ImGui::EndDisabled();

            ImGui::SeparatorText("Light");
            ImGui::ColorEdit3("L Ambient", &light.ambient.x, ImGuiColorEditFlags_Float);
            ImGui::ColorEdit3("L Diffuse", &light.diffuse.x, ImGuiColorEditFlags_Float);
            ImGui::ColorEdit3("L Specular", &light.specular.x, ImGuiColorEditFlags_Float);

            ImGui::SeparatorText("Material");
            ImGui::ColorEdit3("M Ambient", &material.ambient.x, ImGuiColorEditFlags_Float);
            ImGui::ColorEdit3("M Diffuse", &material.diffuse.x, ImGuiColorEditFlags_Float);
            ImGui::ColorEdit3("M Specular", &material.specular.x, ImGuiColorEditFlags_Float);
            ImGui::SliderFloat("Shininess", &material.shininess, 1.f, 256.f, "%.0f");

            ImGui::Checkbox("Inverse Normals", &light.inverse_normals);

            if (light.inverse_normals)
            {
                ImGui::SameLine();
                ImGui::Text(" | TRUE");
            }
            else
            {
                ImGui::SameLine();
                ImGui::Text(" | FALSE");
            }

            if (ImGui::Button("Reset Lighting"))
            {
                light = LightUI{};
                material = MaterialUI{};
            }
        }

        ImGui::End();
    }

    void Tick(float dt)
    {
        if (dt <= 0.0f)
        {
            float currentFrame = static_cast<float>(glfwGetTime());
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;
        }
        else
        {
            deltaTime = dt;
        }
        UpdateCameraInput();
    }

    void Shutdown()
    {
        delete terrainShader;
        terrainShader = nullptr;
    }
}

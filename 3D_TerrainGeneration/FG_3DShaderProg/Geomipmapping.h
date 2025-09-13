#pragma once

#include "Terrain.h"
#include "camera.h"

struct GeomipPatch
{
    int  LOD;
    glm::vec3 position;
    glm::vec3 Min;
    glm::vec3 Max;
    int startVertex;
};

struct GeomipLevel
{
    int step;
    unsigned int IBO;
    int indexCount;
};

class Geomipmapping : public Terrain
{
public:

    Geomipmapping(int patchSize);
    ~Geomipmapping();

    void BuildTerrain();
    void Render(const Camera& cam);
    void SetPatchSize(int s) { patchSize = s; }

private:

    int patchSize;
    int numPatchX;
    int numPatchZ;

    unsigned int VAO = 0;
    unsigned int VBO = 0;

    std::vector<GeomipPatch> patches;
    std::vector<GeomipLevel> levels;

    void BuildPatches();
    void BuildIndices();
    void BuildVBO();
    int SelectLOD(const GeomipPatch& p, const glm::vec3& camPos);
    void MakeSteps(int patchSize, std::vector<int>& LODsteps);
};

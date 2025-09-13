#include "Geomipmapping.h"
#include <iostream>

Geomipmapping::Geomipmapping(int patchSize)
    : patchSize(patchSize), numPatchX(0), numPatchZ(0)
{

}

Geomipmapping::~Geomipmapping()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    for (auto& L : levels)
    {
        if (L.IBO) glDeleteBuffers(1, &L.IBO);
    }
}

void Geomipmapping::BuildTerrain()
{
    if (m_iSize <= 1 || m_heightData.m_Data.empty())
    {
        std::cout << "Geomipmapping::BuildTerrain(): no height data" << std::endl;
        return;
    }

    if (VAO)
    {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
    if (VBO)
    {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
    for (auto& L : levels)
    {
        if (L.IBO) glDeleteBuffers(1, &L.IBO);
    }

    levels.clear();
    patches.clear();

    BuildPatches();

    BuildIndices();

    BuildVBO();

    /*std::cout << "Geomipmapping::BuildTerrain(): " << "patchSize=" << patchSize
        << " patches=" << (numPatchX * numPatchZ) << " LODs=" << levels.size() << std::endl;*/

    LoadTile(ETileType::DIRT, "T_Dirt.png");
    LoadTile(ETileType::GRASS, "T_Grass.png");
    LoadTile(ETileType::ROCK, "T_Rock.png");
    LoadTile(ETileType::SNOW_TIP, "T_snow_mountain.png");

    SetRegion(ETileType::DIRT, 0, 40, 80);
    SetRegion(ETileType::GRASS, 50, 90, 150);
    SetRegion(ETileType::ROCK, 100, 160, 210);
    SetRegion(ETileType::SNOW_TIP, 170, 220, 255);

    if (!GenerateTextureMap(4096, 8.0f))
    {
        return;
    }
}

void Geomipmapping::Render(const Camera& cam)
{
    if (VAO == 0 && m_iSize > 1 && !m_heightData.m_Data.empty()) BuildTerrain();

    glBindVertexArray(VAO);

    for (auto& p : patches)
    {
        p.LOD = SelectLOD(p, cam.Position);
        const GeomipLevel& L = levels[p.LOD];
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, L.IBO);
        glDrawElementsBaseVertex(GL_TRIANGLES, L.indexCount, GL_UNSIGNED_INT, 0, p.startVertex);
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindVertexArray(0);
}

void Geomipmapping::BuildPatches()
{
    numPatchX = (m_iSize - 1) / (patchSize - 1);
    numPatchZ = (m_iSize - 1) / (patchSize - 1);

    patches.clear();
    int startVertexCounter = 0;

    for (int z = 0; z < numPatchZ; z++)
    {
        for (int x = 0; x < numPatchX; x++)
        {
            GeomipPatch patch;
            int vx0 = x * (patchSize - 1);
            int vz0 = z * (patchSize - 1);
            int vx1 = vx0 + patchSize;
            int vz1 = vz0 + patchSize;

            float minH = 0.f;
            float maxH = 0.f;
            for (int zz = vz0; zz < vz1; zz++)
            {
                for (int xx = vx0; xx < vx1; xx++)
                {
                    float h = GetScaledHeightAtPoint(xx, zz);
                    minH = std::min(minH, h);
                    maxH = std::max(maxH, h);
                }
            }

            patch.position = glm::vec3(vx0, 0, vz0);
            patch.Min = glm::vec3(vx0, minH, vz0);
            patch.Max = glm::vec3(vx1 - 1, maxH, vz1 - 1);
            patch.startVertex = startVertexCounter;

            startVertexCounter += patchSize * patchSize;
            patches.push_back(patch);
        }
    }
}

void Geomipmapping::BuildIndices()
{
    //std::vector<int> steps = { 1,2,4,8};
    std::vector<int> steps;
    MakeSteps(patchSize, steps);

    for (int s : steps)
    {
        std::vector<unsigned> idx;
        for (int z = 0; z < patchSize - 1; z += s)
        {
            for (int x = 0; x < patchSize - 1; x += s)
            {
                unsigned v0 = z * patchSize + x;
                unsigned v1 = z * patchSize + x + s;
                unsigned v2 = (z + s) * patchSize + x;
                unsigned v3 = (z + s) * patchSize + x + s;

                idx.push_back(v0);
                idx.push_back(v2);
                idx.push_back(v1);

                idx.push_back(v1);
                idx.push_back(v2);
                idx.push_back(v3);
            }
        }

        GeomipLevel L;
        L.step = s;
        L.indexCount = idx.size();
        glGenBuffers(1, &L.IBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, L.IBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned), idx.data(), GL_STATIC_DRAW);

        levels.push_back(L);
    }
}

void Geomipmapping::BuildVBO()
{
    std::vector<BFVertex> vertices;

    int stepP = patchSize - 1;
    for (int zPatch = 0; zPatch < numPatchZ; zPatch++)
    {
        for (int xPatch = 0; xPatch < numPatchX; xPatch++)
        {
            int vx0 = xPatch * stepP;
            int vz0 = zPatch * stepP;
            for (int z = 0; z < patchSize; z++)
            {
                for (int x = 0; x < patchSize; x++)
                {
                    glm::vec3 color = glm::vec3(1.0f);

                    int hx = vx0 + x;
                    int hz = vz0 + z;
                    float y = GetScaledHeightAtPoint(hx, hz);
                    glm::vec3 pos = { float(hx), y, float(hz) };

                    glm::vec2 texCoord = glm::vec2(float(hx) / float(m_iSize - 1),
                        float(hz) / float(m_iSize - 1));

                    float hl = GetScaledHeightAtPoint(std::clamp(hx - 1, 0, m_iSize - 1), hz);
                    float hr = GetScaledHeightAtPoint(std::clamp(hx + 1, 0, m_iSize - 1), hz);
                    float hd = GetScaledHeightAtPoint(hx, std::clamp(hz - 1, 0, m_iSize - 1));
                    float hu = GetScaledHeightAtPoint(hx, std::clamp(hz + 1, 0, m_iSize - 1));
                    glm::vec3 dx(2, hr - hl, 0);
                    glm::vec3 dz(0, hu - hd, 2);
                    glm::vec3 normal = glm::normalize(glm::cross(dz, dx));

                    vertices.push_back({ pos, color, texCoord, normal });
                }
            }
        }
    }

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(BFVertex), &vertices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BFVertex), (void*)offsetof(BFVertex, pos));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(BFVertex), (void*)offsetof(BFVertex, color));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(BFVertex), (void*)offsetof(BFVertex, texCoord));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(BFVertex), (void*)offsetof(BFVertex, normal));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);


    vertices.clear();
}

int Geomipmapping::SelectLOD(const GeomipPatch& p, const glm::vec3& camPos)
{
    const float diagonal = static_cast<float>(patchSize - 1) * 1.41421356f; //sqrt 2
    float distance = glm::distance(camPos, 0.5f * (p.Min + p.Max));

    float threshold = 2.f * diagonal;

    for (int lod = 0; lod < levels.size(); lod++) {
        if (distance < threshold) return lod;
        threshold *= 2;
    }
    return (int)levels.size() - 1;

    /*if (distance < 100) return 0;
    if (distance < 200) return 1;
    if (distance < 400) return 2;
    return 3;*/
}

void Geomipmapping::MakeSteps(int patchSize, std::vector<int>& LODsteps)
{
    LODsteps.clear();
    if (patchSize < 3) return;

    int maxLevels = static_cast<int>(floor(log2(patchSize - 1)) + 1);

    const int last = patchSize - 1;
    for (int s = 1; s <= last; s *= 2)
    {
        if (last % s != 0) continue;
        if (s == last) continue;
        LODsteps.push_back(s);
    }
}

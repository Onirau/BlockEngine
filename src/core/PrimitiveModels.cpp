#include "PrimitiveModels.h"
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>
#include <vector>

// Vertex structure matching bgfx requirements
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;
};

static std::unordered_map<PartType, Model> g_models;

// Define vertex layout for bgfx
static bgfx::VertexLayout s_vertexLayout;

void InitVertexLayout() {
    s_vertexLayout.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .end();
}

static glm::vec3 CalculateFaceNormal(const glm::vec3 &v1, const glm::vec3 &v2,
                                     const glm::vec3 &v3) {
    glm::vec3 edge1 = v2 - v1;
    glm::vec3 edge2 = v3 - v1;
    return glm::normalize(glm::cross(edge1, edge2));
}

static Model CreateModelFromMesh(const std::vector<Vertex> &vertices,
                                 const std::vector<uint16_t> &indices) {
    Model model;

    // Create vertex buffer
    const bgfx::Memory *vbMem =
        bgfx::copy(vertices.data(), vertices.size() * sizeof(Vertex));
    model.vbh = bgfx::createVertexBuffer(vbMem, s_vertexLayout);

    // Create index buffer
    const bgfx::Memory *ibMem =
        bgfx::copy(indices.data(), indices.size() * sizeof(uint16_t));
    model.ibh = bgfx::createIndexBuffer(ibMem);

    model.numIndices = static_cast<uint32_t>(indices.size());
    model.transform = glm::mat4(1.0f);

    return model;
}

static Model GenMeshCube(float width, float height, float depth) {
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    float w = width * 0.5f;
    float h = height * 0.5f;
    float d = depth * 0.5f;

    // Cube faces (6 faces, 4 vertices each, 2 triangles per face)
    glm::vec3 positions[24] = {// Front face
                               {-w, -h, d},
                               {w, -h, d},
                               {w, h, d},
                               {-w, h, d},
                               // Back face
                               {w, -h, -d},
                               {-w, -h, -d},
                               {-w, h, -d},
                               {w, h, -d},
                               // Top face
                               {-w, h, d},
                               {w, h, d},
                               {w, h, -d},
                               {-w, h, -d},
                               // Bottom face
                               {-w, -h, -d},
                               {w, -h, -d},
                               {w, -h, d},
                               {-w, -h, d},
                               // Right face
                               {w, -h, d},
                               {w, -h, -d},
                               {w, h, -d},
                               {w, h, d},
                               // Left face
                               {-w, -h, -d},
                               {-w, -h, d},
                               {-w, h, d},
                               {-w, h, -d}};

    glm::vec3 normals[6] = {{0, 0, 1},  {0, 0, -1}, {0, 1, 0},
                            {0, -1, 0}, {1, 0, 0},  {-1, 0, 0}};

    glm::vec2 uvs[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};

    for (int face = 0; face < 6; face++) {
        for (int vert = 0; vert < 4; vert++) {
            Vertex v;
            v.position = positions[face * 4 + vert];
            v.normal = normals[face];
            v.texcoord = uvs[vert];
            vertices.push_back(v);
        }

        uint16_t base = face * 4;
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }

    return CreateModelFromMesh(vertices, indices);
}

static Model GenMeshCylinder(float radius, float height, int slices) {
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    float halfHeight = height * 0.5f;

    // Top cap center
    vertices.push_back({{0, halfHeight, 0}, {0, 1, 0}, {0.5f, 0.5f}});

    // Top cap vertices
    for (int i = 0; i <= slices; i++) {
        float angle = (float)i / slices * 2.0f * 3.14159265f;
        float x = radius * cosf(angle);
        float z = radius * sinf(angle);
        vertices.push_back(
            {{x, halfHeight, z}, {0, 1, 0}, {0.5f + x, 0.5f + z}});
    }

    // Bottom cap center
    uint16_t bottomCenterIdx = vertices.size();
    vertices.push_back({{0, -halfHeight, 0}, {0, -1, 0}, {0.5f, 0.5f}});

    // Bottom cap vertices
    for (int i = 0; i <= slices; i++) {
        float angle = (float)i / slices * 2.0f * 3.14159265f;
        float x = radius * cosf(angle);
        float z = radius * sinf(angle);
        vertices.push_back(
            {{x, -halfHeight, z}, {0, -1, 0}, {0.5f + x, 0.5f + z}});
    }

    // Side vertices
    uint16_t sideStartIdx = vertices.size();
    for (int i = 0; i <= slices; i++) {
        float angle = (float)i / slices * 2.0f * 3.14159265f;
        float x = radius * cosf(angle);
        float z = radius * sinf(angle);
        glm::vec3 normal = glm::normalize(glm::vec3(x, 0, z));
        float u = (float)i / slices;

        vertices.push_back({{x, halfHeight, z}, normal, {u, 0}});
        vertices.push_back({{x, -halfHeight, z}, normal, {u, 1}});
    }

    // Top cap indices
    for (int i = 0; i < slices; i++) {
        indices.push_back(0);
        indices.push_back(i + 1);
        indices.push_back(i + 2);
    }

    // Bottom cap indices
    for (int i = 0; i < slices; i++) {
        indices.push_back(bottomCenterIdx);
        indices.push_back(bottomCenterIdx + i + 2);
        indices.push_back(bottomCenterIdx + i + 1);
    }

    // Side indices
    for (int i = 0; i < slices; i++) {
        uint16_t base = sideStartIdx + i * 2;
        indices.push_back(base);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 2);
        indices.push_back(base + 1);
        indices.push_back(base + 3);
    }

    return CreateModelFromMesh(vertices, indices);
}

static Model GenMeshSphere(float radius, int rings, int slices) {
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    for (int ring = 0; ring <= rings; ring++) {
        float phi = 3.14159265f * ring / rings;
        for (int slice = 0; slice <= slices; slice++) {
            float theta = 2.0f * 3.14159265f * slice / slices;

            float x = radius * sinf(phi) * cosf(theta);
            float y = radius * cosf(phi);
            float z = radius * sinf(phi) * sinf(theta);

            glm::vec3 pos(x, y, z);
            glm::vec3 normal = glm::normalize(pos);
            glm::vec2 uv((float)slice / slices, (float)ring / rings);

            vertices.push_back({pos, normal, uv});
        }
    }

    for (int ring = 0; ring < rings; ring++) {
        for (int slice = 0; slice < slices; slice++) {
            uint16_t curr = ring * (slices + 1) + slice;
            uint16_t next = curr + slices + 1;

            indices.push_back(curr);
            indices.push_back(next);
            indices.push_back(curr + 1);

            indices.push_back(curr + 1);
            indices.push_back(next);
            indices.push_back(next + 1);
        }
    }

    return CreateModelFromMesh(vertices, indices);
}

static Model GenMeshWedge() {
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    glm::vec3 positions[] = {// Bottom face (2 triangles)
                             {-0.5f, -0.5f, -0.5f},
                             {0.5f, -0.5f, -0.5f},
                             {-0.5f, -0.5f, 0.5f},
                             {0.5f, -0.5f, -0.5f},
                             {0.5f, -0.5f, 0.5f},
                             {-0.5f, -0.5f, 0.5f},

                             // Back face (2 triangles)
                             {-0.5f, -0.5f, -0.5f},
                             {-0.5f, 0.5f, -0.5f},
                             {0.5f, 0.5f, -0.5f},
                             {0.5f, -0.5f, -0.5f},
                             {-0.5f, -0.5f, -0.5f},
                             {0.5f, 0.5f, -0.5f},

                             // Left triangle
                             {-0.5f, -0.5f, -0.5f},
                             {-0.5f, -0.5f, 0.5f},
                             {-0.5f, 0.5f, -0.5f},

                             // Right triangle
                             {0.5f, -0.5f, -0.5f},
                             {0.5f, 0.5f, -0.5f},
                             {0.5f, -0.5f, 0.5f},

                             // Top face (2 triangles)
                             {-0.5f, 0.5f, -0.5f},
                             {-0.5f, -0.5f, 0.5f},
                             {0.5f, 0.5f, -0.5f},
                             {0.5f, 0.5f, -0.5f},
                             {-0.5f, -0.5f, 0.5f},
                             {0.5f, -0.5f, 0.5f}};

    glm::vec2 uvs[] = {// Bottom
                       {0, 0},
                       {1, 0},
                       {0, 1},
                       {1, 0},
                       {1, 1},
                       {0, 1},
                       // Back
                       {0, 0},
                       {0, 1},
                       {1, 1},
                       {1, 0},
                       {0, 0},
                       {1, 1},
                       // Left
                       {0, 0},
                       {1, 0},
                       {0, 1},
                       // Right
                       {0, 0},
                       {1, 0},
                       {0, 1},
                       // Top
                       {0, 0},
                       {0, 1},
                       {1, 0},
                       {1, 0},
                       {0, 1},
                       {1, 1}};

    int triangleCount = 8;
    for (int i = 0; i < triangleCount; i++) {
        int baseIdx = i * 3;
        glm::vec3 v1 = positions[baseIdx];
        glm::vec3 v2 = positions[baseIdx + 1];
        glm::vec3 v3 = positions[baseIdx + 2];

        glm::vec3 normal = CalculateFaceNormal(v1, v2, v3);

        for (int j = 0; j < 3; j++) {
            Vertex vert;
            vert.position = positions[baseIdx + j];
            vert.normal = normal;
            vert.texcoord = uvs[baseIdx + j];
            vertices.push_back(vert);
            indices.push_back(baseIdx + j);
        }
    }

    return CreateModelFromMesh(vertices, indices);
}

static Model GenMeshCornerWedge() {
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    glm::vec3 positions[] = {// Bottom face (2 triangles)
                             {-0.5f, -0.5f, -0.5f},
                             {0.5f, -0.5f, -0.5f},
                             {-0.5f, -0.5f, 0.5f},
                             {0.5f, -0.5f, -0.5f},
                             {0.5f, -0.5f, 0.5f},
                             {-0.5f, -0.5f, 0.5f},

                             // Right triangle
                             {-0.5f, -0.5f, -0.5f},
                             {-0.5f, -0.5f, 0.5f},
                             {-0.5f, 0.5f, 0.5f},

                             // Front triangle
                             {-0.5f, 0.5f, 0.5f},
                             {-0.5f, -0.5f, 0.5f},
                             {0.5f, -0.5f, 0.5f},

                             // Back triangle
                             {0.5f, -0.5f, -0.5f},
                             {-0.5f, -0.5f, -0.5f},
                             {-0.5f, 0.5f, 0.5f},

                             // Left triangle
                             {0.5f, -0.5f, -0.5f},
                             {-0.5f, 0.5f, 0.5f},
                             {0.5f, -0.5f, 0.5f}};

    glm::vec2 uvs[] = {// Bottom
                       {0, 0},
                       {1, 0},
                       {0, 1},
                       {1, 0},
                       {1, 1},
                       {0, 1},
                       // Right
                       {0, 0},
                       {1, 0},
                       {0, 1},
                       // Front
                       {0, 0},
                       {0, 1},
                       {1, 1},
                       // Back
                       {0, 0},
                       {1, 0},
                       {0.5f, 1},
                       // Left
                       {1, 0},
                       {0, 1},
                       {1, 1}};

    int triangleCount = 6;
    for (int i = 0; i < triangleCount; i++) {
        int baseIdx = i * 3;
        glm::vec3 v1 = positions[baseIdx];
        glm::vec3 v2 = positions[baseIdx + 1];
        glm::vec3 v3 = positions[baseIdx + 2];

        glm::vec3 normal = CalculateFaceNormal(v1, v2, v3);

        for (int j = 0; j < 3; j++) {
            Vertex vert;
            vert.position = positions[baseIdx + j];
            vert.normal = normal;
            vert.texcoord = uvs[baseIdx + j];
            vertices.push_back(vert);
            indices.push_back(baseIdx + j);
        }
    }

    return CreateModelFromMesh(vertices, indices);
}

void PreparePrimitiveModels() {
    // Initialize vertex layout once
    InitVertexLayout();

    // Generate primitive models
    g_models[PartType::Block] = GenMeshCube(1.0f, 1.0f, 1.0f);
    g_models[PartType::Cylinder] = GenMeshCylinder(0.5f, 1.0f, 16);
    g_models[PartType::Ball] = GenMeshSphere(0.5f, 16, 16);
    g_models[PartType::Wedge] = GenMeshWedge();
    g_models[PartType::CornerWedge] = GenMeshCornerWedge();
}

void UnloadPrimitiveModels() {
    for (auto &[type, model] : g_models) {
        if (bgfx::isValid(model.vbh)) {
            bgfx::destroy(model.vbh);
        }
        if (bgfx::isValid(model.ibh)) {
            bgfx::destroy(model.ibh);
        }
    }
    g_models.clear();
}

Model *GetPrimitiveModel(PartType shape) {
    auto it = g_models.find(shape);
    if (it != g_models.end()) {
        return &it->second;
    }
    return nullptr;
}

const bgfx::VertexLayout &GetVertexLayout() { return s_vertexLayout; }
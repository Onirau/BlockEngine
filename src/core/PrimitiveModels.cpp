#include "PrimitiveModels.h"
#include <raylib.h>
#include <raymath.h>

extern Texture2D g_defaultTexture;

static std::unordered_map<PartType, Model> g_models;

void SetMeshTextureCoords(Mesh *mesh, const Vector2 *texcoords) {
    if (!mesh || !texcoords)
        return;

    if (!mesh->texcoords)
        mesh->texcoords =
            (float *)RL_CALLOC(mesh->vertexCount * 2, sizeof(float));

    for (unsigned int i = 0; i < mesh->vertexCount; i++) {
        mesh->texcoords[i * 2 + 0] = texcoords[i].x;
        mesh->texcoords[i * 2 + 1] = texcoords[i].y;
    }
}

static void CalculateFaceNormal(Vector3 v1, Vector3 v2, Vector3 v3,
                                Vector3 *normal) {
    Vector3 edge1 = Vector3Subtract(v2, v1);
    Vector3 edge2 = Vector3Subtract(v3, v1);
    *normal = Vector3Normalize(Vector3CrossProduct(edge1, edge2));
}

static Mesh GenMeshWedge() {
    Mesh mesh = {0};

    // Each face needs its own vertices for proper per-face normals
    Vector3 vertices[] = {// Bottom face (2 triangles = 6 vertices)
                          {-0.5, -0.5, -0.5},
                          {0.5, -0.5, -0.5},
                          {-0.5, -0.5, 0.5},
                          {0.5, -0.5, -0.5},
                          {0.5, -0.5, 0.5},
                          {-0.5, -0.5, 0.5},

                          // Back face (2 triangles = 6 vertices)
                          {-0.5, -0.5, -0.5},
                          {-0.5, 0.5, -0.5},
                          {0.5, 0.5, -0.5},
                          {0.5, -0.5, -0.5},
                          {-0.5, -0.5, -0.5},
                          {0.5, 0.5, -0.5},

                          // Left triangle (3 vertices)
                          {-0.5, -0.5, -0.5},
                          {-0.5, -0.5, 0.5},
                          {-0.5, 0.5, -0.5},

                          // Right triangle (3 vertices)
                          {0.5, -0.5, -0.5},
                          {0.5, 0.5, -0.5},
                          {0.5, -0.5, 0.5},

                          // Top face (2 triangles = 6 vertices)
                          {-0.5, 0.5, -0.5},
                          {-0.5, -0.5, 0.5},
                          {0.5, 0.5, -0.5},
                          {0.5, 0.5, -0.5},
                          {-0.5, -0.5, 0.5},
                          {0.5, -0.5, 0.5}};

    Vector2 uv[] = {// Bottom
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

    mesh.vertexCount = 24;
    mesh.triangleCount = 8;

    mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
    mesh.normals = (float *)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));

    // Copy vertices and calculate normals
    for (int i = 0; i < mesh.triangleCount; i++) {
        int baseIdx = i * 3;
        Vector3 v1 = vertices[baseIdx];
        Vector3 v2 = vertices[baseIdx + 1];
        Vector3 v3 = vertices[baseIdx + 2];

        Vector3 normal;
        CalculateFaceNormal(v1, v2, v3, &normal);

        for (int j = 0; j < 3; j++) {
            int vertIdx = baseIdx + j;
            mesh.vertices[vertIdx * 3 + 0] = vertices[vertIdx].x;
            mesh.vertices[vertIdx * 3 + 1] = vertices[vertIdx].y;
            mesh.vertices[vertIdx * 3 + 2] = vertices[vertIdx].z;

            mesh.normals[vertIdx * 3 + 0] = normal.x;
            mesh.normals[vertIdx * 3 + 1] = normal.y;
            mesh.normals[vertIdx * 3 + 2] = normal.z;
        }
    }

    SetMeshTextureCoords(&mesh, uv);

    mesh.indices = (unsigned short *)RL_MALLOC(mesh.triangleCount * 3 *
                                               sizeof(unsigned short));
    for (int i = 0; i < mesh.triangleCount * 3; i++) {
        mesh.indices[i] = i;
    }

    UploadMesh(&mesh, false);
    return mesh;
}

static Mesh GenMeshCornerWedge() {
    Mesh mesh = {0};

    Vector3 vertices[] = {// Bottom face (2 triangles = 6 vertices)
                          {-0.5, -0.5, -0.5},
                          {0.5, -0.5, -0.5},
                          {-0.5, -0.5, 0.5},
                          {0.5, -0.5, -0.5},
                          {0.5, -0.5, 0.5},
                          {-0.5, -0.5, 0.5},

                          // Right triangle (3 vertices)
                          {-0.5, -0.5, -0.5},
                          {-0.5, -0.5, 0.5},
                          {-0.5, 0.5, 0.5},

                          // Front triangle (3 vertices)
                          {-0.5, 0.5, 0.5},
                          {-0.5, -0.5, 0.5},
                          {0.5, -0.5, 0.5},

                          // Back triangle (3 vertices)
                          {0.5, -0.5, -0.5},
                          {-0.5, -0.5, -0.5},
                          {-0.5, 0.5, 0.5},

                          // Left triangle (3 vertices)
                          {0.5, -0.5, -0.5},
                          {-0.5, 0.5, 0.5},
                          {0.5, -0.5, 0.5}};

    Vector2 uv[] = {// Bottom
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
                    {0.5, 1},
                    // Left
                    {1, 0},
                    {0, 1},
                    {1, 1}};

    mesh.vertexCount = 18;
    mesh.triangleCount = 6;

    mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
    mesh.normals = (float *)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));

    // Copy vertices and calculate normals
    for (int i = 0; i < mesh.triangleCount; i++) {
        int baseIdx = i * 3;
        Vector3 v1 = vertices[baseIdx];
        Vector3 v2 = vertices[baseIdx + 1];
        Vector3 v3 = vertices[baseIdx + 2];

        Vector3 normal;
        CalculateFaceNormal(v1, v2, v3, &normal);

        for (int j = 0; j < 3; j++) {
            int vertIdx = baseIdx + j;
            mesh.vertices[vertIdx * 3 + 0] = vertices[vertIdx].x;
            mesh.vertices[vertIdx * 3 + 1] = vertices[vertIdx].y;
            mesh.vertices[vertIdx * 3 + 2] = vertices[vertIdx].z;

            mesh.normals[vertIdx * 3 + 0] = normal.x;
            mesh.normals[vertIdx * 3 + 1] = normal.y;
            mesh.normals[vertIdx * 3 + 2] = normal.z;
        }
    }

    SetMeshTextureCoords(&mesh, uv);

    mesh.indices = (unsigned short *)RL_MALLOC(mesh.triangleCount * 3 *
                                               sizeof(unsigned short));
    for (int i = 0; i < mesh.triangleCount * 3; i++) {
        mesh.indices[i] = i;
    }

    UploadMesh(&mesh, false);
    return mesh;
}

void PreparePrimitiveModels() {
    Model block = LoadModelFromMesh(GenMeshCube(1.f, 1.f, 1.f));
    Model cylinder = LoadModelFromMesh(GenMeshCylinder(0.5f, 1.f, 16));
    Model ball = LoadModelFromMesh(GenMeshSphere(0.5, 16, 16));
    Model wedge = LoadModelFromMesh(GenMeshWedge());
    Model cornerWedge = LoadModelFromMesh(GenMeshCornerWedge());

    block.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = g_defaultTexture;
    cylinder.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = g_defaultTexture;
    ball.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = g_defaultTexture;
    wedge.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = g_defaultTexture;
    cornerWedge.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture =
        g_defaultTexture;

    g_models[PartType::Block] = block;
    g_models[PartType::Cylinder] = cylinder;
    g_models[PartType::Ball] = ball;
    g_models[PartType::Wedge] = wedge;
    g_models[PartType::CornerWedge] = cornerWedge;
}

void UnloadPrimitiveModels() {
    for (auto &[_, model] : g_models)
        UnloadModel(model);
    g_models.clear();
}

Model *GetPrimitiveModel(PartType shape) {
    auto it = g_models.find(shape);
    if (it != g_models.end())
        return &it->second;

    return nullptr;
}
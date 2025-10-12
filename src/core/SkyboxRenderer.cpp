#include "SkyboxRenderer.h"
#include <bgfx/bgfx.h>
#include <bx/math.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

Skybox g_skybox;

static bgfx::UniformHandle u_inner;
static bgfx::UniformHandle u_outer;

// Vertex structure for skybox
struct SkyboxVertex {
    float x, y, z;

    static void init() {
        ms_layout.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .end();
    }

    static bgfx::VertexLayout ms_layout;
};

bgfx::VertexLayout SkyboxVertex::ms_layout;

static const char *SKY_VS = R"(
$input a_position
$output v_dir

#include <bgfx_shader.sh>

void main() {
    v_dir = a_position;
    
    // Remove translation from view matrix
    mat3 R = mat3(u_view[0].xyz, u_view[1].xyz, u_view[2].xyz);
    mat4 viewNoTrans = mat4(
        vec4(R[0], 0.0),
        vec4(R[1], 0.0),
        vec4(R[2], 0.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    );
    
    gl_Position = mul(u_proj, mul(viewNoTrans, vec4(a_position, 1.0)));
    gl_Position.z = gl_Position.w; // Force to far plane
}
)";

static const char *SKY_FS = R"(
$input v_dir

#include <bgfx_shader.sh>

uniform vec4 u_innerColor;
uniform vec4 u_outerColor;

void main() {
    vec3 nd = normalize(v_dir);
    float t = clamp(nd.y * 0.5 + 0.5, 0.0, 1.0);
    vec3 col = mix(u_innerColor.rgb, u_outerColor.rgb, t);
    
    // Gamma correction
    gl_FragColor = vec4(pow(col, vec3_splat(1.0/2.2)), 1.0);
}
)";

void LoadSkybox() {
    // Initialize vertex layout
    SkyboxVertex::init();

    // Create cube vertices
    static const SkyboxVertex cubeVertices[] = {
        {-1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f, 1.0f}, {-1.0f, 1.0f, -1.0f},
        {-1.0f, 1.0f, 1.0f},   {1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, 1.0f},
        {1.0f, 1.0f, -1.0f},   {1.0f, 1.0f, 1.0f},
    };

    // Create cube indices
    static const uint16_t cubeIndices[] = {
        0, 1, 2, 1, 3, 2, // -X
        4, 6, 5, 5, 6, 7, // +X
        0, 2, 4, 4, 2, 6, // -Y
        1, 5, 3, 5, 7, 3, // +Y
        0, 4, 1, 4, 5, 1, // -Z
        2, 3, 6, 3, 7, 6, // +Z
    };

    // Create vertex buffer
    const bgfx::Memory *vbMem = bgfx::copy(cubeVertices, sizeof(cubeVertices));
    g_skybox.vbh = bgfx::createVertexBuffer(vbMem, SkyboxVertex::ms_layout);

    // Create index buffer
    const bgfx::Memory *ibMem = bgfx::copy(cubeIndices, sizeof(cubeIndices));
    g_skybox.ibh = bgfx::createIndexBuffer(ibMem);

    // Note: In production, you'd compile these shaders with shaderc
    // For now, assuming pre-compiled shaders or using runtime compilation
    // You'll need to compile these with bgfx's shaderc tool
    bgfx::ShaderHandle vsh =
        bgfx::createShader(bgfx::makeRef(SKY_VS, strlen(SKY_VS)));
    bgfx::ShaderHandle fsh =
        bgfx::createShader(bgfx::makeRef(SKY_FS, strlen(SKY_FS)));
    g_skybox.program = bgfx::createProgram(vsh, fsh, true);

    // Create uniforms
    u_inner = bgfx::createUniform("u_innerColor", bgfx::UniformType::Vec4);
    u_outer = bgfx::createUniform("u_outerColor", bgfx::UniformType::Vec4);
}

void UnloadSkybox() {
    if (bgfx::isValid(g_skybox.vbh)) {
        bgfx::destroy(g_skybox.vbh);
        g_skybox.vbh = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(g_skybox.ibh)) {
        bgfx::destroy(g_skybox.ibh);
        g_skybox.ibh = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(g_skybox.program)) {
        bgfx::destroy(g_skybox.program);
        g_skybox.program = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(u_inner)) {
        bgfx::destroy(u_inner);
    }
    if (bgfx::isValid(u_outer)) {
        bgfx::destroy(u_outer);
    }
}

void DrawSkybox() {
    float inner[4] = {0.25f, 0.55f, 0.65f, 1.0f};
    float outer[4] = {0.05f, 0.15f, 0.45f, 1.0f};

    // Set uniforms
    bgfx::setUniform(u_inner, inner);
    bgfx::setUniform(u_outer, outer);

    // Set render state
    // Write RGB only, depth test LEQUAL, no depth write, no culling
    uint64_t state = 0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_DEPTH_TEST_LEQUAL |
                     BGFX_STATE_CULL_CW // or CCW depending on your winding
        ;

    bgfx::setState(state);

    // Set vertex and index buffers
    bgfx::setVertexBuffer(0, g_skybox.vbh);
    bgfx::setIndexBuffer(g_skybox.ibh);

    // Submit draw call
    // View ID 0 is typically the main view
    bgfx::submit(0, g_skybox.program);
}
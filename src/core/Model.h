

#include <bgfx/bgfx.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Model {
    bgfx::VertexBufferHandle vbh;
    bgfx::IndexBufferHandle ibh;
    uint32_t numIndices;
    glm::mat4 transform;

    Model()
        : vbh(BGFX_INVALID_HANDLE), ibh(BGFX_INVALID_HANDLE), numIndices(0),
          transform(glm::mat4(1.0f)) {}
};
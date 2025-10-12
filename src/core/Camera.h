#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Camera {
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;
    float fov;
    float aspect;
    float nearPlane;
    float farPlane;

    glm::mat4 getViewMatrix() const {
        return glm::lookAt(position, target, up);
    }

    glm::mat4 getProjectionMatrix() const {
        return glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
    }
};
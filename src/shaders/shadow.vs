#version 330

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexTexCoord;

uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 lightSpaceMatrix;

out vec2 fragTexCoord;
out vec3 fragNormal;
out vec4 fragPosLightSpace;

void main() {
    fragTexCoord = vertexTexCoord;
    fragNormal = normalize(vec3(matModel * vec4(vertexNormal, 0.0)));
    fragPosLightSpace = lightSpaceMatrix * matModel * vec4(vertexPosition, 1.0);
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
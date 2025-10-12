#version 330

in vec2 fragTexCoord;
in vec3 fragNormal;
in vec4 fragPosLightSpace;

uniform sampler2D texture0;
uniform sampler2D shadowMap;
uniform vec4 colDiffuse;
uniform vec3 lightDir;

out vec4 finalColor;

float ShadowCalculation(vec4 fragPosLight) {
    vec3 projCoords = fragPosLight.xyz / fragPosLight.w;
    projCoords = projCoords * 0.5 + 0.5;
    if(projCoords.z > 1.0) return 0.0;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float bias = 0.005;
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    return shadow;
}

void main() {
    vec4 texelColor = texture(texture0, fragTexCoord);
    float diff = max(dot(fragNormal, normalize(-lightDir)), 0.0);
    float shadow = ShadowCalculation(fragPosLightSpace);
    vec3 lighting = (0.3 + (1.0 - shadow) * diff * 0.7) * texelColor.rgb * colDiffuse.rgb;
    finalColor = vec4(lighting, texelColor.a * colDiffuse.a);
}
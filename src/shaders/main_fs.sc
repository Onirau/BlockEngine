$input v_normal, v_position

#include <bgfx_shader.sh>

uniform vec4 u_color;

void main()
{
    vec3 normal = normalize(v_normal);
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    
    float diffuse = max(dot(normal, lightDir), 0.0);
    vec3 ambient = vec3(0.3, 0.3, 0.3);
    
    vec3 finalColor = u_color.rgb * (ambient + diffuse * 0.7);
    
    gl_FragColor = vec4(finalColor, u_color.a);
}
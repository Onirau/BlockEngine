$input a_position, a_normal
$output v_normal, v_position

#include <bgfx_shader.sh>

uniform mat4 u_modelViewProj;
uniform mat4 u_model;

void main()
{
    v_position = mul(u_model, vec4(a_position, 1.0)).xyz;
    v_normal = mul(u_model, vec4(a_normal, 0.0)).xyz;
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
}
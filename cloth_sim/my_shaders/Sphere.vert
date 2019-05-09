#version 330

uniform mat4 u_model;
uniform mat4 u_view_projection;

in vec3 in_position;
in vec3 in_normal;
in vec4 in_tangent;
in vec2 in_uv;

out vec4 v_position;
out vec4 v_normal;
out vec2 v_uv;
out vec4 v_tangent;

void main() {
    v_position = vec4(in_position, 1.0);
    v_position = u_model * v_position;
    v_normal = normalize(u_model * vec4(in_normal, 0.0));
//    v_uv = in_uv;
//    v_tangent = normalize(u_model * in_tangent);
    // gl_Position = v_position;
    gl_Position = u_view_projection * v_position;
}

#version 330

uniform vec3 u_cam_pos;
uniform vec3 u_light_pos;
uniform vec3 u_light_intensity;

uniform vec4 u_color;

uniform sampler2D u_texture_2;
uniform vec2 u_texture_2_size;

uniform float u_normal_scaling;
uniform float u_height_scaling;

in vec4 v_position;
in vec4 v_normal;
in vec4 v_tangent;
in vec2 v_uv;

out vec4 out_color;

float h(vec2 uv) {
  // You may want to use this helper function...
    return texture(u_texture_2, uv).r;
  // return 0.0;
}

void main() {
  // YOUR CODE HERE
    vec3 b = cross(v_normal.xyz, v_tangent.xyz);
    mat3x3 TBN = mat3x3(v_tangent.xyz, b, v_normal.xyz);
    // float height_scaling = 1.0;
    float d_u = (h(vec2(v_uv.x + 1.0 / u_texture_2_size.x, v_uv.y)) - h(v_uv)) * u_normal_scaling * u_height_scaling;
    float d_v = (h(vec2(v_uv.x, v_uv.y + 1.0 / u_texture_2_size.y)) - h(v_uv)) * u_normal_scaling * u_height_scaling;
    vec3 n0 = vec3(-d_u, -d_v, 1.0);
    vec3 nd = TBN * n0;
    
    vec4 r = vec4(u_light_pos, 1.0) - v_position;
    float k_a = 0.2;
    float k_d = 0.8;
    float k_s = 1.0;
    vec4 I_a = vec4(0.0, 0.0, 0.0, 0.0);
    int p = 3;
    vec4 l = normalize(r);
    vec4 m = normalize(vec4(u_cam_pos, 1.0) - v_position);
    out_color = k_d * (vec4(u_light_intensity, 1.0) / (length(r) * (length(r)))) * max(dot(vec4(nd, 0.0), l), 0.0);
    out_color += k_s * (vec4(u_light_intensity, 1.0) / (length(r) * (length(r)))) *
    pow(max(dot(vec4(nd, 0.0), normalize(m + l)), 0.0), p);
    out_color += k_a * I_a;
    out_color.a = 1;
}

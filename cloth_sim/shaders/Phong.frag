#version 330

uniform vec4 u_color;
uniform vec3 u_cam_pos;
uniform vec3 u_light_pos;
uniform vec3 u_light_intensity;

in vec4 v_position;
in vec4 v_normal;
in vec2 v_uv;

out vec4 out_color;

void main() {
  // YOUR CODE HERE
    vec4 r = vec4(u_light_pos, 1.0) - v_position;
    float k_a = 0.5;
    float k_d = 0.8;
    float k_s = 1.0;
    vec4 I_a = vec4(1.0, 0.0, 1.0, 0.0);
    int p = 3;
    vec4 l = normalize(r);
    vec4 m = normalize(vec4(u_cam_pos, 1.0) - v_position);
    vec4 diffuse = k_d * (vec4(u_light_intensity, 1.0) / (length(r) * (length(r)))) * max(dot(v_normal, l), 0.0);
    vec4 specular = k_s * (vec4(u_light_intensity, 1.0) / (length(r) * (length(r)))) *
    pow(max(dot(v_normal, normalize(m + l)), 0.0), p);
    vec4 ambient = k_a * I_a;
    out_color = diffuse + specular + ambient;
    
  // (Placeholder code. You will want to replace it.)
  // out_color = (vec4(1, 1, 1, 0) + v_normal) / 2;
  out_color.a = 1;
}


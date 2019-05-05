#version 330

uniform vec4 u_color;

in vec4 v_position;
in vec4 v_normal;
in vec2 v_uv;
out vec4 out_color;

void main() {
//    out_color = vec4(texture(u_information_texture, v_uv));
//    out_color.a = 1;
  out_color = u_color;
}

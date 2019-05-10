#version 330

uniform sampler2D u_texture;

in vec3 out_point_normal;
in vec2 out_uv;

out vec4 outColor;

void main()
{
//    outColor = vec4((vec3(1.0, 1.0, 1.0) + out_point_normal) / 2.0, 1.0);
    outColor = vec4(texture(u_texture, out_uv));
    outColor.a = 1;
//     outColor = vec4(1.0, 0.0, 0.0, 0.3);
}

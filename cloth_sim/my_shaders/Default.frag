#version 330

in vec3 out_point_normal;

out vec4 outColor;

void main()
{
    outColor = vec4((vec3(1.0, 1.0, 1.0) + out_point_normal) / 2.0, 1.0);
}

#version 330

uniform mat4 u_view_projection;
uniform int pause;
uniform samplerBuffer u_points;
uniform samplerBuffer u_pinned;
uniform vec3 u_sphere_origin;
uniform float u_radius;
uniform int u_exist_sphere;
uniform float u_friction_sphere;
uniform float u_friction_plane;
uniform int u_exist_plane;
uniform vec3 u_point_plane;
uniform vec3 u_normal_plane;
uniform float u_thickness;

in vec3 position;
in int pinned;
in vec3 last_position;
in vec3 point_normal;
in vec2 uv;

out vec3 outPosition;
out vec3 out_last_position;
out vec3 out_point_normal;
out vec2 out_uv;

vec3 collide_sphere(vec3 p) {
    // vec3 p_ = p - vec3(0.000005, 0.0, 0.0);
    vec3 direction = normalize(p - u_sphere_origin);
    float len = distance(p, u_sphere_origin);
    vec3 new_position = p;
    if (len <= u_radius) {
        vec3 correction = u_sphere_origin + u_radius * direction - last_position;
        new_position = last_position + (1.0 - u_friction_sphere) * correction;
        if (isnan(correction.x)) new_position = last_position;
        // if (isnan(new_position.x)) new_position = u_sphere_origin + u_radius * direction;
    }
    return new_position;
}

vec3 collide_plane(vec3 p) {
    vec3 vector_new = p - u_point_plane;
    vec3 vector_last = last_position - u_point_plane;
    vec3 new_position = p;
    if (dot(vector_new, u_normal_plane) * dot(vector_last, u_normal_plane) <= 0.0) {
        vec3 unit = normalize(u_normal_plane);
        vec3 tangent = p - dot(unit, vector_new) * unit;
        vec3 vector = vec3(0.0, 0.0, 0.0);
        if (dot(vector_last, u_normal_plane) < 0) {
            vector = tangent - u_normal_plane * 0.0002 - last_position;
        }
        else {
            vector = tangent + u_normal_plane * 0.0002 - last_position;
        }
        new_position = last_position + (1.0 - u_friction_plane) * vector;
    }
    return new_position + vec3(0.0, 0.000001, 0.0);
}

vec3 collide_self(vec3 self_position) {
    int num = 0;
    vec3 correction = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < 2500; i ++) {
        vec3 p = texelFetch(u_points, i).rgb;
        if (p != self_position) {
            float dist = distance(p, self_position);
            if (dist < 2 * u_thickness) {
                vec3 direction = normalize(self_position - p);
                correction += (2 * u_thickness - dist) * direction;
                num ++;
            }
        }
    }
    vec3 new_position = self_position;
    if (num > 0) {
        correction = correction / float(num) / 10.0;
        new_position += correction;
    }
    return new_position;
}

void main() {
    out_last_position = position;
    out_point_normal = point_normal;
    out_uv = uv;
    if (pause == 0 && pinned == 0) {
        outPosition = collide_self(position);
//        outPosition = position;
        
        if (u_exist_sphere == 1) {
            outPosition = collide_sphere(outPosition);
        }
        if (u_exist_plane == 1) {
            outPosition = collide_plane(outPosition);
        }
    }
    else {
        outPosition = position;
    }

    gl_Position = u_view_projection * vec4(outPosition, 1.0);
}

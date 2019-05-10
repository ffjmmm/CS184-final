#version 330

uniform mat4 u_model;
uniform mat4 u_view_projection;
uniform vec3 u_gravity;
uniform int pause;
uniform float u_delta_t;
uniform float u_damping;
uniform float u_mass;
uniform samplerBuffer u_points;
uniform samplerBuffer u_pinned;
uniform float u_ks;
uniform vec3 u_sphere_origin;
uniform float u_radius;
uniform int u_exist_sphere;
uniform float u_friction_sphere;
uniform float u_friction_plane;
uniform int u_exist_plane;
uniform vec3 u_point_plane;
uniform vec3 u_normal_plane;
uniform vec3 u_wind;
uniform float u_thickness;
uniform int u_flag;

layout(location = 0) in vec3 position;
layout(location = 1) in int pinned;
layout(location = 2) in vec3 last_position;
layout(location = 3) in vec4 spring_structural;
layout(location = 4) in vec4 spring_shearing;
layout(location = 5) in vec4 spring_bending;
layout(location = 6) in vec3 point_normal;
layout(location = 7) in vec2 uv;

out vec3 outPosition;
out vec3 out_last_position;
out vec3 out_point_normal;
out vec2 out_uv;

vec3 spring_force(int k, float relax_length, int type) {
    vec3 force = vec3(0.0, 0.0, 0.0);
    if (k == -1) {
        return force;
    }
    vec3 p = texelFetch(u_points, k).rgb;
    vec3 direction = normalize(p - position);
    float length = distance(p, position);
    force = u_ks * (length - relax_length) * direction;
    if (type == 2) {
        force = 0.2 * force;
    }
    return force;
}

vec3 constrain_changes(int k, float relax_length, vec3 new_position) {
    vec3 position_change = vec3(0, 0, 0);
    if (k == -1) {
        return position_change;
    }
    vec3 p = texelFetch(u_points, k).rgb;
    float length = distance(p, new_position);
    if (length > relax_length * 1.1) {
        vec3 direction = normalize(p - new_position);
        float len = length - relax_length * 1.1;
        if (texelFetch(u_pinned, k).r == 1) {
            position_change = len * direction;
        }
        else {
            position_change = (len / 2.0) * direction;
        }
    }
    return position_change;
}

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
        correction = correction / float(num) / 15.0;
        new_position += correction;
    }
    return new_position;
}

void main() {
    out_last_position = position;
    out_point_normal = point_normal;
    out_uv = uv;
    if (u_damping == 0.0 && pause == 0 && u_delta_t == 0.0) {
        outPosition = position;
    }
    else {
        if (pause == 0 && pinned == 0) {
            vec3 force = vec3(0.0, 0.0, 0.0);
            force += u_mass * u_gravity;
            if (length(u_wind) > 0.0) {
                vec3 wind_direction = normalize(u_wind);
                force += u_mass * dot(point_normal, wind_direction) * u_wind;
            }
            
            force += spring_force(int(spring_structural.x), 0.02, 0);
            force += spring_force(int(spring_structural.y), 0.02, 0);
            force += spring_force(int(spring_structural.z), 0.02, 0);
            force += spring_force(int(spring_structural.w), 0.02, 0);
            force += spring_force(int(spring_shearing.x), 0.028284, 1);
            force += spring_force(int(spring_shearing.y), 0.028284, 1);
            force += spring_force(int(spring_shearing.z), 0.028284, 1);
            force += spring_force(int(spring_shearing.w), 0.028284, 1);
            force += spring_force(int(spring_bending.x), 0.04, 2);
            force += spring_force(int(spring_bending.y), 0.04, 2);
            force += spring_force(int(spring_bending.z), 0.04, 2);
            force += spring_force(int(spring_bending.w), 0.04, 2);
            
            vec3 a = force / u_mass;
            outPosition = position + u_damping * (position - last_position) + u_delta_t * u_delta_t * a;
                
            if (u_exist_sphere == 1) {
                outPosition = collide_sphere(outPosition);
            }
            if (u_exist_plane == 1) {
                outPosition = collide_plane(outPosition);
            }
            
            outPosition += constrain_changes(int(spring_bending.x), 0.04, outPosition);
            outPosition += constrain_changes(int(spring_bending.y), 0.04, outPosition);
            outPosition += constrain_changes(int(spring_bending.z), 0.04, outPosition);
            outPosition += constrain_changes(int(spring_bending.w), 0.04, outPosition);
            outPosition += constrain_changes(int(spring_shearing.x), 0.028284, outPosition);
            outPosition += constrain_changes(int(spring_shearing.y), 0.028284, outPosition);
            outPosition += constrain_changes(int(spring_shearing.z), 0.028284, outPosition);
            outPosition += constrain_changes(int(spring_shearing.w), 0.028284, outPosition);
            outPosition += constrain_changes(int(spring_structural.x), 0.02, outPosition);
            outPosition += constrain_changes(int(spring_structural.y), 0.02, outPosition);
            outPosition += constrain_changes(int(spring_structural.z), 0.02, outPosition);
            outPosition += constrain_changes(int(spring_structural.w), 0.02, outPosition);
        }
        else {
            outPosition = position;
        }
    }
//    if (u_flag == 0) gl_Position = u_view_projection * vec4(last_position, 1.0);
//    else gl_Position = u_view_projection * vec4(outPosition, 1.0);
    gl_Position = u_view_projection * vec4(outPosition, 1.0);
//    gl_Position = vec4(0.0, 0.0, 0.0, 0.0);
}

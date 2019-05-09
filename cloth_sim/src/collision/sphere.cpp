#include <nanogui/nanogui.h>

#include "../clothMesh.h"
#include "../misc/sphere_drawing.h"
#include "sphere.h"

using namespace nanogui;
using namespace CGL;

void Sphere::collide(PointMass &pm) {
  // TODO (Part 3): Handle collisions with spheres.
    Vector3D direction = pm.position - origin;
    direction.normalize();
    double length = (pm.position - origin).norm();
    if (length <= radius) {
        Vector3D vector = origin + direction * radius - pm.last_position;
        pm.position = pm.last_position + (1.0 - friction) * vector;
    }
}

void Sphere::render(GLShader &shader) {
  // We decrease the radius here so flat triangles don't behave strangely
  // and intersect with the sphere when rendered
  m_sphere_mesh.draw_sphere(shader, origin, radius * 0.92);
}

MatrixXf Sphere::get_normals() {
    return m_sphere_mesh.get_normals();
}

MatrixXf Sphere::get_position() {
    return m_sphere_mesh.get_positions();
}

Vector3D Sphere::get_origin() {
    return origin;
}

double Sphere::get_radius() {
    return radius;
}

#include "iostream"
#include <nanogui/nanogui.h>

#include "../clothMesh.h"
#include "../clothSimulator.h"
#include "plane.h"

using namespace std;
using namespace CGL;

#define SURFACE_OFFSET 0.0001

void Plane::collide(PointMass &pm) {
  // TODO (Part 3): Handle collisions with planes.
    Vector3D vector_new = pm.position - point;
    Vector3D vector_last = pm.last_position - point;
    if (dot(vector_new, normal) * dot(vector_last, normal) <= 0) {
        Vector3D unit = normal.unit();
        Vector3D tangent = pm.position - dot(unit, vector_new) * unit;
        Vector3D vector;
        if (dot(vector_last, normal) < 0) {
            vector = tangent - normal * SURFACE_OFFSET - pm.last_position;
        }
        else {
            vector = tangent + normal * SURFACE_OFFSET - pm.last_position;
        }
        pm.position = pm.last_position + (1.0 - friction) * vector;
    }
}

void Plane::render(GLShader &shader) {
  nanogui::Color color(0.7f, 0.7f, 0.7f, 1.0f);

  Vector3f sPoint(point.x, point.y, point.z);
  Vector3f sNormal(normal.x, normal.y, normal.z);
  Vector3f sParallel(normal.y - normal.z, normal.z - normal.x,
                     normal.x - normal.y);
  sParallel.normalize();
  Vector3f sCross = sNormal.cross(sParallel);

  MatrixXf positions(3, 4);
  MatrixXf normals(3, 4);

  positions.col(0) << sPoint + 2 * (sCross + sParallel);
  positions.col(1) << sPoint + 2 * (sCross - sParallel);
  positions.col(2) << sPoint + 2 * (-sCross + sParallel);
  positions.col(3) << sPoint + 2 * (-sCross - sParallel);

  normals.col(0) << sNormal;
  normals.col(1) << sNormal;
  normals.col(2) << sNormal;
  normals.col(3) << sNormal;

  if (shader.uniform("u_color", false) != -1) {
    shader.setUniform("u_color", color);
  }
  shader.uploadAttrib("in_position", positions);
  if (shader.attrib("in_normal", false) != -1) {
    shader.uploadAttrib("in_normal", normals);
  }

  shader.drawArray(GL_TRIANGLE_STRIP, 0, 4);
}

MatrixXf Plane::get_position() {
    return MatrixXf(1, 1);
}

MatrixXf Plane::get_normals() {
    return MatrixXf(1, 1);
}

Vector3D Plane::get_origin() {
    return Vector3D(0.0, 0.0, 0.0);
}

double Plane::get_radius() {
    return 0.0;
}

double Plane::get_friction() {
    return friction;
}

int Plane::get_type() {
    return 2;
}

Vector3D Plane::get_point() {
    return point;
}

Vector3D Plane::get_normal() {
    return normal;
}

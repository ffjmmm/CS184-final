#ifndef COLLISIONOBJECT_PLANE_H
#define COLLISIONOBJECT_PLANE_H

#include <nanogui/nanogui.h>

#include "../clothMesh.h"
#include "collisionObject.h"

using namespace nanogui;
using namespace CGL;
using namespace std;

struct Plane : public CollisionObject {
public:
  Plane(const Vector3D &point, const Vector3D &normal, double friction)
      : point(point), normal(normal.unit()), friction(friction) {}

  void render(GLShader &shader);
  void collide(PointMass &pm);
    MatrixXf get_position();
    MatrixXf get_normals();
    Vector3D get_origin();
    double get_radius();
    double get_friction();
    int get_type();
    Vector3D get_point();
    Vector3D get_normal();
    bool get_controllable() {
        return controllable;
    }
    bool get_useTexture() {
        return false;
    }
    
    bool controllable;
    
  Vector3D point;
  Vector3D normal;

  double friction;
};

#endif /* COLLISIONOBJECT_PLANE_H */

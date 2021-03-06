#ifndef COLLISIONOBJECT_SPHERE_H
#define COLLISIONOBJECT_SPHERE_H

#include "../clothMesh.h"
#include "../misc/sphere_drawing.h"
#include "collisionObject.h"

using namespace CGL;
using namespace std;

struct Sphere : public CollisionObject {
public:
  Sphere(const Vector3D &origin, double radius, double friction, int num_lat = 40, int num_lon = 40)
      : origin(origin), radius(radius), radius2(radius * radius),
        friction(friction), m_sphere_mesh(Misc::SphereMesh(num_lat, num_lon)) {}

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
        return use_texture;
    }
    
    bool use_texture;
    bool controllable;
    
private:
  Vector3D origin;
  double radius;
  double radius2;

  double friction;
  
  Misc::SphereMesh m_sphere_mesh;
};

#endif /* COLLISIONOBJECT_SPHERE_H */

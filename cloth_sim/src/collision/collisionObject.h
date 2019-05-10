#ifndef COLLISIONOBJECT
#define COLLISIONOBJECT

#include <nanogui/nanogui.h>

#include "../clothMesh.h"

using namespace CGL;
using namespace std;
using namespace nanogui;

class CollisionObject {
public:
  virtual void render(GLShader &shader) = 0;
  virtual void collide(PointMass &pm) = 0;
    virtual MatrixXf get_position() = 0;
    virtual MatrixXf get_normals() = 0;
    virtual Vector3D get_origin() = 0;
    virtual double get_radius() = 0;
    virtual double get_friction() = 0;
    virtual int get_type() = 0;
    virtual Vector3D get_point() = 0;
    virtual Vector3D get_normal() = 0;
    virtual bool get_controllable() = 0;
    virtual bool get_useTexture() = 0;
    
private:
  double friction;
};

#endif /* COLLISIONOBJECT */

#ifndef POINTMASS_H
#define POINTMASS_H

#include "CGL/CGL.h"
#include "CGL/misc.h"
#include "CGL/vector3D.h"
#include <vector>

using namespace CGL;
using namespace std;

// Forward declarations
class Halfedge;

struct PointMass {
  PointMass(Vector3D position, bool pinned)
      : pinned(pinned), start_position(position), position(position),
        last_position(position) {}

  Vector3D normal();
  Vector3D velocity(double delta_t) {
    return (position - last_position) / delta_t;
  }

  // static values
  bool pinned;
  Vector3D start_position;

  // dynamic values
  Vector3D position;
  Vector3D last_position;
  Vector3D forces;
    
    vector<int> index_spring_STRUCTURAL;
    vector<int> index_spring_SHEARING;
    vector<int> index_spring_BENDING;
    int index_feedback;
    
  // mesh reference
  Halfedge *halfedge;
};

#endif /* POINTMASS_H */

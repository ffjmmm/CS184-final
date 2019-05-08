#ifndef CLOTH_H
#define CLOTH_H

#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <string>

#include "CGL/CGL.h"
#include "CGL/misc.h"
#include "clothMesh.h"
#include "collision/collisionObject.h"
#include "spring.h"

using namespace CGL;
using namespace std;

enum e_orientation { HORIZONTAL = 0, VERTICAL = 1 };

struct ClothParameters {
  ClothParameters() {}
  ClothParameters(bool enable_structural_constraints,
                  bool enable_shearing_constraints,
                  bool enable_bending_constraints, double damping,
                  double density, double ks)
      : enable_structural_constraints(enable_structural_constraints),
        enable_shearing_constraints(enable_shearing_constraints),
        enable_bending_constraints(enable_bending_constraints),
        damping(damping), density(density), ks(ks) {}
  ~ClothParameters() {}

  // Global simulation parameters

  bool enable_structural_constraints;
  bool enable_shearing_constraints;
  bool enable_bending_constraints;

  double damping;

  // Mass-spring parameters
  double density;
  double ks;
};

struct Cloth {
  Cloth() {}
  Cloth(double width, double height, int num_width_points,
        int num_height_points, float thickness);
  ~Cloth();

  void buildGrid();
    void initTransFormBuffer(string project_root);
  void simulate(double frames_per_sec, double simulation_steps, ClothParameters *cp,
                Vector3D gravity, vector<CollisionObject *> *collision_objects,
                Vector3D wind, Matrix4f model, Matrix4f viewProjection, bool pause);

  void reset();
    void buildClothMesh();

  void build_spatial_map();
  void self_collide(PointMass &pm, double simulation_steps);
  float hash_position(Vector3D pos);

  // Cloth properties
  double width;
  double height;
  int num_width_points;
  int num_height_points;
  double thickness;
  e_orientation orientation;

  // Cloth components
  vector<PointMass> point_masses;
  vector<vector<int>> pinned;
  vector<Spring> springs;
  ClothMesh *clothMesh;

  // Spatial hashing
  unordered_map<float, vector<PointMass *> *> map;
    
    string m_project_root;
    
    int len = 19;
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint program;
    GLint uniGravity;
    GLint uniKs;
    GLint uniPoints;
    GLfloat points[2500 * 3];
    GLint points_pinned[2500];
    GLint uniPinned;
    GLint uniPoints_x;
    GLint uniPoints_y;
    GLint uniPoints_z;
    GLint uniMass;
    GLint uniDamping;
    GLint uniDeltaT;
    GLint uniPause;
    GLint uniTime;
    GLint uniModel;
    GLint uniViewProjection;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLuint points_buffer;
    GLuint pinned_buffer;
    GLuint points_texture;
    GLuint pinned_texture;
    GLint springStructuralAttrib;
    GLint springShearingAttrib;
    GLint springBendingAttrib;
    GLint posAttrib;
    GLint pinAttrib;
    GLint lastPosAttrib;
    GLuint tbo;
    GLfloat dataPos[2500 * 19];
    GLfloat feedback[14502 * 6 * 2];
    GLuint indices[14502 * 2];
//    GLfloat dataPos[14502 * 2 * 7];
//    GLfloat feedback[14502 * 2 * 6];
};

#endif /* CLOTH_H */

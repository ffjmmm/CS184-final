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
    void initTransFormBuffer(string project_root, vector<CollisionObject *> *objects);
  void simulate(double frames_per_sec, double simulation_steps, ClothParameters *cp,
                Vector3D gravity, vector<CollisionObject *> *collision_objects,
                Vector3D wind, Matrix4f model, Matrix4f viewProjection, bool pause, int time);

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
    
    int len = 24;
    int num_points_sphere = 28800;
    GLuint my_texture;
    GLuint vertexShader_sphere;
    GLuint fragmentShader_sphere;
    GLuint program_sphere;
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint program;
    GLint uniGravity;
    GLint uniKs;
    GLint uniPoints;
    GLfloat points[2500 * 3];
    GLint points_pinned[2500];
    GLint uniFriction_sphere;
    GLint uniSphere_origin;
    GLint uniRadius;
    GLint uniWind;
    GLint uniThickness;
    GLint uniTexture;
    GLint uniExist_sphere;
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
    GLint uniModel_sphere;
    GLint uniViewProjection;
    GLint uniViewProjection_sphere;
    GLuint vao;
    GLuint vbo;
    GLuint vbo_sphere;
    GLuint ebo;
    GLuint points_buffer;
    GLuint pinned_buffer;
    GLuint points_texture;
    GLuint pinned_texture;
    GLint springStructuralAttrib;
    GLint springShearingAttrib;
    GLint springBendingAttrib;
    GLint normalAttrib;
    GLint posAttrib;
    GLint pinAttrib;
    GLint lastPosAttrib;
    GLint uvAttrib;
    GLint posAttrib_sphere;
    GLuint tbo;
    GLfloat points_sphere[28800 * 6];
    GLfloat dataPos[2500 * 24];
    GLfloat feedback[14502 * 6 * 2];
    GLfloat feedback_tri[4802 * 6 * 3];
    GLuint indices[14502 * 2];
    GLuint indices_tri[4802 * 3];
    Matrix4f model_sphere;
    Vector3D sphere_origin;
    GLfloat sphere_radius = 0;
    GLint exist_sphere = 0;
    GLfloat friction_sphere;
    Vector3D point_normal;
    Vector3D wind_accleration = Vector3D(0.0, 0.0, -5.0);
};

#endif /* CLOTH_H */

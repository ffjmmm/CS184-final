#include <iostream>
#include <math.h>
#include <random>
#include <vector>
#include <cstdio>
#include <fstream>

#include "cloth.h"
#include "clothSimulator.h"
#include "collision/plane.h"
#include "collision/sphere.h"
#include "misc/stb_image.h"

using namespace std;

Cloth::Cloth(double width, double height, int num_width_points,
             int num_height_points, float thickness) {
  this->width = width;
  this->height = height;
  this->num_width_points = num_width_points;
  this->num_height_points = num_height_points;
  this->thickness = thickness;

  buildGrid();
  buildClothMesh();
}

Cloth::~Cloth() {
  point_masses.clear();
  springs.clear();

  if (clothMesh) {
    delete clothMesh;
  }
    
    glDeleteProgram(program);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);
    
    glDeleteBuffers(1, &tbo);
    glDeleteBuffers(1, &vbo);
    
    glDeleteVertexArrays(1, &vao);
    
}


void Cloth::buildGrid() {
    // TODO (Part 1): Build a grid of masses and springs.
    double width_unit = width / num_width_points;
    double height_unit = height / num_height_points;
    
    for (int h = 0; h < num_height_points; h ++) {
        for (int w = 0; w < num_width_points; w ++) {
            double x = width_unit * w;
            double y = height_unit * h;
            
            bool pin = false;
            for (int i = 0; i < pinned.size(); i ++) {
                if (pinned[i][0] == w && pinned[i][1] == h) {
                    pin = true;
                    break;
                }
            }
            if (orientation == 0) {
                PointMass p = PointMass(Vector3D(x, 1.0, y), pin);
                point_masses.push_back(p);
            }
            else {
                double z = ((double)rand() / (double)RAND_MAX) * 0.002 - 0.001;
                PointMass p = PointMass(Vector3D(x, y, z), pin);
                point_masses.push_back(p);
            }
        }
    }
    
    for (int h = 0; h < num_height_points; h ++) {
        for (int w = 0; w < num_width_points; w ++) {
            int index = h * num_width_points + w;
            // Structural
            if (w - 1 >= 0) {
                int left_index = h * num_width_points + (w - 1);
                Spring spring = Spring(&point_masses[index], &point_masses[left_index], STRUCTURAL);
                springs.push_back(spring);
            }
            if (h - 1 >= 0) {
                int top_index = (h - 1) * num_width_points + w;
                Spring spring = Spring(&point_masses[index], &point_masses[top_index], STRUCTURAL);
                springs.push_back(spring);
            }
            // Shearing
            if (h - 1 >= 0 && w - 1 >= 0) {
                int upper_left_index = (h - 1) * num_width_points + (w - 1);
                Spring spring = Spring(&point_masses[index], &point_masses[upper_left_index], SHEARING);
                springs.push_back(spring);
            }
            if (h - 1 >= 0 && w + 1 < num_width_points) {
                int upper_right_index = (h - 1) * num_width_points + (w + 1);
                Spring spring = Spring(&point_masses[index], &point_masses[upper_right_index], SHEARING);
                springs.push_back(spring);
            }
            // Bending
            if (w - 2 >= 0) {
                int left_two_index = h * num_width_points + (w - 2);
                Spring spring = Spring(&point_masses[index], &point_masses[left_two_index], BENDING);
                springs.push_back(spring);
            }
            if (h - 2 >= 0) {
                int top_two_index = (h - 2) * num_width_points + w;
                Spring spring = Spring(&point_masses[index], &point_masses[top_two_index], BENDING);
                springs.push_back(spring);
            }
        }
    }
    
}

void Cloth::initTransFormBuffer(string project_root, vector<CollisionObject *> *objects, Vector3D _wind) {
    m_project_root = project_root;
    wind_accleration = _wind;
    if (wind_accleration.norm2() > 0.0) {
        wind_flag = 1;
    }
    
    freopen("indices", "r", stdin);
    for (int i = 0; i < 14502 * 2; i ++) {
        scanf("%d", &indices[i]);
    }
    fclose(stdin);
    freopen("indices_tri", "r", stdin);
    for (int i = 0; i < 4802 * 3; i ++) {
        scanf("%d", &indices_tri[i]);
    }
    printf("Indices Data Read completely!\n");
    // printf("%d %d %d %d \n", indices[100], indices[101], indices[102], indices[103]);
    
    auto file_to_string = [](const std::string &filename) -> std::string {
        if (filename.empty())
            return "";
        std::ifstream t(filename);
        return std::string((std::istreambuf_iterator<char>(t)),
                           std::istreambuf_iterator<char>());
    };
    
    string vert_shader_str = file_to_string(m_project_root + "/my_shaders/Simulation.vert");
    string frag_shader_str = file_to_string(m_project_root + "/my_shaders/Simulation.frag");
    string vert_shader_str_sphere = file_to_string(m_project_root + "/my_shaders/Sphere.vert");
    string frag_shader_str_sphere = file_to_string(m_project_root + "/my_shaders/Sphere.frag");
    string vert_shader_str_correction = file_to_string(m_project_root + "/my_shaders/Correction.vert");
    string frag_shader_str_correction = file_to_string(m_project_root + "/my_shaders/Correction.frag");
    string vert_shader_str_plane = file_to_string(m_project_root + "/my_shaders/Plane.vert");
    string frag_shader_str_plane = file_to_string(m_project_root + "/my_shaders/Plane.frag");
    
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // glShaderSource(vertexShader, 1, &vertexShaderSrc, nullptr);
    const char *vert_shader_const = vert_shader_str.c_str();
    glShaderSource(vertexShader, 1, &vert_shader_const, nullptr);
    glCompileShader(vertexShader);
    
    GLint status;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
    
    if (status != GL_TRUE) {
        char buffer[512];
        std::cerr << "Error while compiling ";
        std::cerr << "vertex shader";
        std::cerr << vert_shader_str << std::endl << std::endl;
        glGetShaderInfoLog(vertexShader, 512, nullptr, buffer);
        std::cerr << "Error: " << std::endl << buffer << std::endl;
        throw std::runtime_error("Shader compilation failed!");
    }
    
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    // glShaderSource(fragmentShader, 1, &fragmentShaderSrc, nullptr);
    const char *frag_shader_const = frag_shader_str.c_str();
    glShaderSource(fragmentShader, 1, &frag_shader_const, nullptr);
    glCompileShader(fragmentShader);
    
    vertexShader_sphere = glCreateShader(GL_VERTEX_SHADER);
    const char *vert_shader_const_sphere = vert_shader_str_sphere.c_str();
    glShaderSource(vertexShader_sphere, 1, &vert_shader_const_sphere, nullptr);
    glCompileShader(vertexShader_sphere);
    
    fragmentShader_sphere = glCreateShader(GL_FRAGMENT_SHADER);
    const char *frag_shader_const_sphere = frag_shader_str_sphere.c_str();
    glShaderSource(fragmentShader_sphere, 1, &frag_shader_const_sphere, nullptr);
    glCompileShader(fragmentShader_sphere);
    
    vertexShader_plane = glCreateShader(GL_VERTEX_SHADER);
    const char *vert_shader_const_plane = vert_shader_str_plane.c_str();
    glShaderSource(vertexShader_plane, 1, &vert_shader_const_plane, nullptr);
    glCompileShader(vertexShader_plane);
    
    fragmentShader_plane = glCreateShader(GL_FRAGMENT_SHADER);
    const char *frag_shader_const_plane = frag_shader_str_plane.c_str();
    glShaderSource(fragmentShader_plane, 1, &frag_shader_const_plane, nullptr);
    glCompileShader(fragmentShader_plane);
    
    vertexShader_correction = glCreateShader(GL_VERTEX_SHADER);
    const char *vert_shader_const_correction = vert_shader_str_correction.c_str();
    glShaderSource(vertexShader_correction, 1, &vert_shader_const_correction, nullptr);
    glCompileShader(vertexShader_correction);
    
    glGetShaderiv(vertexShader_correction, GL_COMPILE_STATUS, &status);
    
    if (status != GL_TRUE) {
        char buffer[512];
        std::cerr << "Error while compiling ";
        std::cerr << "vertex shader";
        glGetShaderInfoLog(vertexShader_correction, 512, nullptr, buffer);
        std::cerr << "Error: " << std::endl << buffer << std::endl;
        throw std::runtime_error("Shader compilation failed!");
    }
    
    fragmentShader_correction = glCreateShader(GL_FRAGMENT_SHADER);
    const char *frag_shader_const_correction = frag_shader_str_correction.c_str();
    glShaderSource(fragmentShader_correction, 1, &frag_shader_const_correction, nullptr);
    glCompileShader(fragmentShader_correction);
    
    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    
    const GLchar* feedbackVaryings[] = {"outPosition", "out_last_position"};
    glTransformFeedbackVaryings(program, 2, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);
    
    glLinkProgram(program);
    uniViewProjection = glGetUniformLocation(program, "u_view_projection");
    uniModel = glGetUniformLocation(program, "u_model");
    uniPause = glGetUniformLocation(program, "pause");
    uniGravity = glGetUniformLocation(program, "u_gravity");
    uniDeltaT = glGetUniformLocation(program, "u_delta_t");
    uniDamping = glGetUniformLocation(program, "u_damping");
    uniMass = glGetUniformLocation(program, "u_mass");
    uniKs = glGetUniformLocation(program, "u_ks");
    uniPoints = glGetUniformLocation(program, "u_points");
    uniPinned = glGetUniformLocation(program, "u_pinned");
    uniSphere_origin = glGetUniformLocation(program, "u_sphere_origin");
    uniRadius = glGetUniformLocation(program, "u_radius");
    uniExist_sphere = glGetUniformLocation(program, "u_exist_sphere");
    uniFriction_sphere = glGetUniformLocation(program, "u_friction_sphere");
    uniFriction_plane = glGetUniformLocation(program, "u_friction_plane");
    uniTexture = glGetUniformLocation(program, "u_texture");
    uniWind = glGetUniformLocation(program, "u_wind");
    uniExist_plane = glGetUniformLocation(program, "u_exist_plane");
    uniPoint_plane = glGetUniformLocation(program, "u_point_plane");
    uniNormal_plane = glGetUniformLocation(program, "u_normal_plane");
    uniThickness = glGetUniformLocation(program, "u_thickness");
    uniFlag = glGetUniformLocation(program, "u_flag");
    
    for (int i = 0; i < springs.size(); i ++) {
        for (int k = 0; k < point_masses.size(); k ++) {
            if (springs[i].pm_a -> position == point_masses[k].position) {
                springs[i].index_a = k;
                if (springs[i].spring_type == BENDING) {
                    point_masses[k].index_spring_BENDING.push_back(i);
                }
                if (springs[i].spring_type == STRUCTURAL) {
                    point_masses[k].index_spring_STRUCTURAL.push_back(i);
                }
                if (springs[i].spring_type == SHEARING) {
                    point_masses[k].index_spring_SHEARING.push_back(i);
                }
                point_masses[k].index_feedback = 2 * i;
            }
            if (springs[i].pm_b -> position == point_masses[k].position) {
                springs[i].index_b = k;
                if (springs[i].spring_type == BENDING) {
                    point_masses[k].index_spring_BENDING.push_back(i);
                }
                if (springs[i].spring_type == STRUCTURAL) {
                    point_masses[k].index_spring_STRUCTURAL.push_back(i);
                }
                if (springs[i].spring_type == SHEARING) {
                    point_masses[k].index_spring_SHEARING.push_back(i);
                }
                point_masses[k].index_feedback = 2 * i + 1;
            }
        }
    }
    // int num[2500] = {0};
    for (int i = 0; i < clothMesh -> triangles.size(); i ++) {
        Triangle *tri = clothMesh -> triangles[i];
        int num_pm1 = 0, num_pm2 = 0, num_pm3 = 0;
        for (int k = 0; k < point_masses.size(); k ++) {
            if (tri -> pm1 -> position == point_masses[k].position) {
                point_masses[k].index_feedback_tri = 3 * i;
                point_masses[k].texture_uv = tri -> uv1;
                num_pm1 = k;
//                num[k] ++;
            }
            if (tri -> pm2 -> position == point_masses[k].position) {
                point_masses[k].index_feedback_tri = 3 * i + 1;
                point_masses[k].texture_uv = tri -> uv2;
                num_pm2 = k;
//                num[k] ++;
            }
            if (tri -> pm3 -> position == point_masses[k].position) {
                point_masses[k].index_feedback_tri = 3 * i + 2;
                point_masses[k].texture_uv = tri -> uv3;
                num_pm3 = k;
//                num[k] ++;
            }
        }
    }
    /*
    int max = 0;
    for (int i = 0; i < 2500; i ++) {
        max = num[i] > max ? num[i] : max;
        printf("%d : %d \n", i, num[i]);
    }
    printf("MAX = %d\n", max);
    */
    // 012: position 3: pinned 456: last_position
    // STRUCTURAL No.: 789(10) rest_len = 0.02
    // SHEARING No.:(11)(12)(13)(14) rest_len = 0.028284
    // BENDING No." (15)(16)(17)(18) rest_len = 0.04
    // Normal: (19)(20)(21)
    // texture uv: (22)(23)
    
    for (int i = 0; i < point_masses.size(); i ++) {
        points[3 * i] = point_masses[i].position.x;
        points[3 * i + 1] = point_masses[i].position.y;
        points[3 * i + 2] = point_masses[i].position.z;
        points_pinned[i] = (int)point_masses[i].pinned;
        dataPos[len * i] = point_masses[i].position.x;
        dataPos[len * i + 1] = point_masses[i].position.y;
        dataPos[len * i + 2] = point_masses[i].position.z;
        dataPos[len * i + 3] = (int)point_masses[i].pinned;
        dataPos[len * i + 4] = point_masses[i].last_position.x;
        dataPos[len * i + 5] = point_masses[i].last_position.y;
        dataPos[len * i + 6] = point_masses[i].last_position.z;
//        dataPos_selfCollision[len_selfCollision * i] = point_masses[i].position.x;
//        dataPos_selfCollision[len_selfCollision * i + 1] = point_masses[i].position.y;
//        dataPos_selfCollision[len_selfCollision * i + 2] = point_masses[i].position.z;
//        dataPos_selfCollision[len_selfCollision * i + 3] = (int)point_masses[i].pinned;
//        dataPos_selfCollision[len_selfCollision * i + 4] = point_masses[i].last_position.x;
//        dataPos_selfCollision[len_selfCollision * i + 5] = point_masses[i].last_position.y;
//        dataPos_selfCollision[len_selfCollision * i + 6] = point_masses[i].last_position.z;
        point_normal = point_masses[i].normal();
        dataPos[len * i + 19] = point_normal.x;
        dataPos[len * i + 20] = point_normal.y;
        dataPos[len * i + 21] = point_normal.z;
        dataPos[len * i + 22] = point_masses[i].texture_uv.x;
        dataPos[len * i + 23] = point_masses[i].texture_uv.y;
//        dataPos_selfCollision[len_selfCollision * i + 19] = point_masses[i].texture_uv.x;
//        dataPos_selfCollision[len_selfCollision * i + 20] = point_masses[i].texture_uv.y;
        for (int k = 0; k < 12; k ++) {
            dataPos[len * i + 7 + k] = -1;
//            dataPos_selfCollision[len_selfCollision * i + 7 + k] = -1;
        }
        for (int k = 0; k < point_masses[i].index_spring_STRUCTURAL.size(); k ++) {
            int another_point_index;
            if (springs[point_masses[i].index_spring_STRUCTURAL[k]].index_a == i) {
                another_point_index = springs[point_masses[i].index_spring_STRUCTURAL[k]].index_b;
            }
            else {
                another_point_index = springs[point_masses[i].index_spring_STRUCTURAL[k]].index_a;
            }
            dataPos[len * i + 7 + k] = another_point_index;
//            dataPos_selfCollision[len_selfCollision * i + 7 + k] = another_point_index;
        }
        for (int k = 0; k < point_masses[i].index_spring_SHEARING.size(); k ++) {
            int another_point_index;
            if (springs[point_masses[i].index_spring_SHEARING[k]].index_a == i) {
                another_point_index = springs[point_masses[i].index_spring_SHEARING[k]].index_b;
            }
            else {
                another_point_index = springs[point_masses[i].index_spring_SHEARING[k]].index_a;
            }
            dataPos[len * i + 11 + k] = another_point_index;
//            dataPos_selfCollision[len_selfCollision * i + 11 + k] = another_point_index;
        }
        for (int k = 0; k < point_masses[i].index_spring_BENDING.size(); k ++) {
            int another_point_index;
            if (springs[point_masses[i].index_spring_BENDING[k]].index_a == i) {
                another_point_index = springs[point_masses[i].index_spring_BENDING[k]].index_b;
            }
            else {
                another_point_index = springs[point_masses[i].index_spring_BENDING[k]].index_a;
            }
            dataPos[len * i + 15 + k] = another_point_index;
//            dataPos_selfCollision[len_selfCollision * i + 15 + k] = another_point_index;
        }
    }
    
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(dataPos), dataPos, GL_STREAM_DRAW);
    
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    posAttrib = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, len * sizeof(GL_FLOAT), 0);

    pinAttrib = glGetAttribLocation(program, "pinned");
    glEnableVertexAttribArray(pinAttrib);
    glVertexAttribPointer(pinAttrib, 1, GL_INT, GL_FALSE, len * sizeof(GL_FLOAT), (void*)(3 * sizeof(GLfloat)));
    
    lastPosAttrib = glGetAttribLocation(program, "last_position");
    glEnableVertexAttribArray(lastPosAttrib);
    glVertexAttribPointer(lastPosAttrib, 3, GL_FLOAT, GL_FALSE, len * sizeof(GL_FLOAT), (void*)(4 * sizeof(GLfloat)));
    
    
    springStructuralAttrib = glGetAttribLocation(program, "spring_structural");
    glEnableVertexAttribArray(springStructuralAttrib);
    glVertexAttribPointer(springStructuralAttrib, 4, GL_FLOAT, GL_FALSE, len * sizeof(GL_FLOAT), (void*)(7 * sizeof(GLfloat)));
    
    springShearingAttrib = glGetAttribLocation(program, "spring_shearing");
    glEnableVertexAttribArray(springShearingAttrib);
    glVertexAttribPointer(springShearingAttrib, 4, GL_FLOAT, GL_FALSE, len * sizeof(GL_FLOAT), (void*)(11 * sizeof(GLfloat)));
    
    springBendingAttrib = glGetAttribLocation(program, "spring_bending");
    glEnableVertexAttribArray(springBendingAttrib);
    glVertexAttribPointer(springBendingAttrib, 4, GL_FLOAT, GL_FALSE, len * sizeof(GL_FLOAT), (void*)(15 * sizeof(GLfloat)));
    
    normalAttrib = glGetAttribLocation(program, "point_normal");
    glEnableVertexAttribArray(normalAttrib);
    glVertexAttribPointer(normalAttrib, 3, GL_FLOAT, GL_FALSE, len * sizeof(GL_FLOAT), (void*)(19 * sizeof(GLfloat)));
    
//    uvAttrib = glGetAttribLocation(program, "uv");
//    glEnableVertexAttribArray(uvAttrib);
//    glVertexAttribPointer(uvAttrib, 2, GL_FLOAT, GL_FALSE, len * sizeof(GL_FLOAT), (void*)(22 * sizeof(GLfloat)));
    
    glGenBuffers(1, &tbo);
    glBindBuffer(GL_ARRAY_BUFFER, tbo);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(feedback), nullptr, GL_STATIC_READ);
    glBufferData(GL_ARRAY_BUFFER, sizeof(feedback_tri), nullptr, GL_STATIC_READ);
    
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    
    glGenBuffers(1, &points_buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, points_buffer);
    glBufferData(GL_TEXTURE_BUFFER, sizeof(points), points, GL_STREAM_READ);

    glGenTextures(1, &points_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_BUFFER, points_texture);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, points_buffer);
    
    glGenBuffers(1, &pinned_buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, pinned_buffer);
    glBufferData(GL_TEXTURE_BUFFER, sizeof(points_pinned), points_pinned, GL_STATIC_READ);
    
    glGenTextures(1, &pinned_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_BUFFER, pinned_texture);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, pinned_buffer);
    
    glGenTextures(1, &my_texture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, my_texture);
    int img_x, img_y, img_n;
    unsigned char* img_data = stbi_load((m_project_root + "/textures/texture_3.png").c_str(), &img_x, &img_y, &img_n, 3);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_x, img_y, 0, GL_RGB, GL_UNSIGNED_BYTE, img_data);
    stbi_image_free(img_data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_tri), indices_tri, GL_STATIC_DRAW);
    
    
//    glGenBuffers(1, &vbo_correction);
//    glBindBuffer(GL_ARRAY_BUFFER, vbo_correction);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(dataPos), dataPos, GL_STREAM_DRAW);

    program_correction = glCreateProgram();
    glAttachShader(program_correction, vertexShader_correction);
    glAttachShader(program_correction, fragmentShader_correction);
    
    glTransformFeedbackVaryings(program_correction, 2, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);
    
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, len * sizeof(GL_FLOAT), 0);
    glBindAttribLocation(program_correction, 7, "position");
    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 1, GL_INT, GL_FALSE, len * sizeof(GL_FLOAT), (void*)(3 * sizeof(GLfloat)));
    glBindAttribLocation(program_correction, 8, "pinned");
    glEnableVertexAttribArray(9);
    glVertexAttribPointer(9, 3, GL_FLOAT, GL_FALSE, len * sizeof(GL_FLOAT), (void*)(4 * sizeof(GLfloat)));
    glBindAttribLocation(program_correction, 9, "last_position");
    glEnableVertexAttribArray(10);
    glVertexAttribPointer(10, 3, GL_FLOAT, GL_FALSE, len * sizeof(GL_FLOAT), (void*)(19 * sizeof(GLfloat)));
    glBindAttribLocation(program_correction, 10, "point_normal");
    glEnableVertexAttribArray(11);
    glVertexAttribPointer(11, 2, GL_FLOAT, GL_FALSE, len * sizeof(GL_FLOAT), (void*)(22 * sizeof(GLfloat)));
    glBindAttribLocation(program_correction, 11, "uv");
    glLinkProgram(program_correction);
    
    uniViewProjection_correction = glGetUniformLocation(program_correction, "u_view_projection");
    uniPause_correction = glGetUniformLocation(program_correction, "pause");
    uniPoints_correction = glGetUniformLocation(program_correction, "u_points");
    uniPinned_correction = glGetUniformLocation(program_correction, "u_pinned");
    uniSphere_origin_correction = glGetUniformLocation(program_correction, "u_sphere_origin");
    uniRadius_correction = glGetUniformLocation(program_correction, "u_radius");
    uniExist_sphere_correction = glGetUniformLocation(program_correction, "u_exist_sphere");
    uniFriction_sphere_correction = glGetUniformLocation(program_correction, "u_friction_sphere");
    uniFriction_plane_correction = glGetUniformLocation(program_correction, "u_friction_plane");
    uniTexture_correction = glGetUniformLocation(program_correction, "u_texture");
    uniExist_plane_correction = glGetUniformLocation(program_correction, "u_exist_plane");
    uniPoint_plane_correction = glGetUniformLocation(program_correction, "u_point_plane");
    uniNormal_plane_correction = glGetUniformLocation(program_correction, "u_normal_plane");
    uniThickness_correction = glGetUniformLocation(program_correction, "u_thickness");
    uniUseTexture_correction = glGetUniformLocation(program_correction, "u_use_texture");
    
    for (CollisionObject *co : *objects) {
        //type: 1.Sphere 2.Plane
        int type = co -> get_type();
        if (type == 1) {
            MatrixXf position = co -> get_position();
            MatrixXf normals = co -> get_normals();
            sphere_origin = co -> get_origin();
            sphere_radius = co -> get_radius();
            friction_sphere = co -> get_friction();
            sphere_controllable = co -> get_controllable();
            exist_sphere = 1;
            model_sphere << sphere_radius, 0, 0, sphere_origin.x, 0, sphere_radius, 0, sphere_origin.y, 0, 0, sphere_radius, sphere_origin.z, 0, 0, 0, 1;
            for (int i = 0; i < num_points_sphere; i ++) {
                points_sphere[6 * i] = position.col(i)[0];
                points_sphere[6 * i + 1] = position.col(i)[1];
                points_sphere[6 * i + 2] = position.col(i)[2];
                points_sphere[6 * i + 3] = normals.col(i)[0];
                points_sphere[6 * i + 4] = normals.col(i)[1];
                points_sphere[6 * i + 5] = normals.col(i)[2];
            }
        }
        else {
            exist_plane = 1;
            friction_plane = co -> get_friction();
            point_plane = co -> get_point();
            normal_plane = co -> get_normal();
            Vector3D parallel_plane = Vector3D(normal_plane.y - normal_plane.z, normal_plane.z - normal_plane.x, normal_plane.x - normal_plane.y);
            parallel_plane.normalize();
            Vector3D cross_plane = cross(normal_plane, parallel_plane);
            points_plane[0] = (point_plane + 2 * (cross_plane + parallel_plane)).x;
            points_plane[1] = (point_plane + 2 * (cross_plane + parallel_plane)).y;
            points_plane[2] = (point_plane + 2 * (cross_plane + parallel_plane)).z;
            points_plane[3] = normal_plane.x;
            points_plane[4] = normal_plane.y;
            points_plane[5] = normal_plane.z;
            points_plane[6] = (point_plane + 2 * (cross_plane - parallel_plane)).x;
            points_plane[7] = (point_plane + 2 * (cross_plane - parallel_plane)).y;
            points_plane[8] = (point_plane + 2 * (cross_plane - parallel_plane)).z;
            points_plane[9] = normal_plane.x;
            points_plane[10] = normal_plane.y;
            points_plane[11] = normal_plane.z;
            points_plane[12] = (point_plane + 2 * (-cross_plane + parallel_plane)).x;
            points_plane[13] = (point_plane + 2 * (-cross_plane + parallel_plane)).y;
            points_plane[14] = (point_plane + 2 * (-cross_plane + parallel_plane)).z;
            points_plane[15] = normal_plane.x;
            points_plane[16] = normal_plane.y;
            points_plane[17] = normal_plane.z;
            points_plane[18] = (point_plane + 2 * (-cross_plane - parallel_plane)).x;
            points_plane[19] = (point_plane + 2 * (-cross_plane - parallel_plane)).y;
            points_plane[20] = (point_plane + 2 * (-cross_plane - parallel_plane)).z;
            points_plane[21] = normal_plane.x;
            points_plane[22] = normal_plane.y;
            points_plane[23] = normal_plane.z;
        }
    }
    
    glGenBuffers(1, &vbo_sphere);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points_sphere), points_sphere, GL_STATIC_DRAW);
    
    program_sphere = glCreateProgram();
    glAttachShader(program_sphere, vertexShader_sphere);
    glAttachShader(program_sphere, fragmentShader_sphere);
    
    int vertexAttribIndex = 12;
    glEnableVertexAttribArray(vertexAttribIndex);
    glVertexAttribPointer(vertexAttribIndex, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), 0);
    glBindAttribLocation(program_sphere, vertexAttribIndex, "in_position");
    vertexAttribIndex ++;
    glEnableVertexAttribArray(vertexAttribIndex);
    glVertexAttribPointer(vertexAttribIndex, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), (void*)(3 * sizeof(GLfloat)));
    glBindAttribLocation(program_sphere, vertexAttribIndex, "in_normal");
    glLinkProgram(program_sphere);
    
    uniViewProjection_sphere = glGetUniformLocation(program_sphere, "u_view_projection");
    uniModel_sphere = glGetUniformLocation(program_sphere, "u_model");
    
    
    glGenBuffers(1, &vbo_plane);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_plane);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points_plane), points_plane, GL_STATIC_DRAW);
    
    program_plane = glCreateProgram();
    glAttachShader(program_plane, vertexShader_plane);
    glAttachShader(program_plane, fragmentShader_plane);
    
    vertexAttribIndex ++;
    glEnableVertexAttribArray(vertexAttribIndex);
    glVertexAttribPointer(vertexAttribIndex, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), 0);
    glBindAttribLocation(program_plane, vertexAttribIndex, "in_position");
    vertexAttribIndex ++;
    glEnableVertexAttribArray(vertexAttribIndex);
    glVertexAttribPointer(vertexAttribIndex, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), (void*)(3 * sizeof(GLfloat)));
    glBindAttribLocation(program_plane, vertexAttribIndex, "in_normal");
    glLinkProgram(program_plane);
    
    uniViewProjection_plane = glGetUniformLocation(program_plane, "u_view_projection");
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
}

void Cloth::simulate(double frames_per_sec, double simulation_steps, ClothParameters *cp,
                     Vector3D gravity, vector<CollisionObject *> *collision_objects, Vector3D wind,
                     Matrix4f model, Matrix4f viewProjection, bool pause, int time, Vector3D object_move) {
    double mass = width * height * cp->density / num_width_points / num_height_points;
    double delta_t = 1.0f / frames_per_sec / simulation_steps;
    
    if (wind_flag) {
        wind_accleration.z += sin(2 * PI * time / 10.0) > 0 ?  2.0 : -2.0;
    }
    
    if (exist_sphere && !pause) {
        if (sphere_controllable) sphere_origin += object_move * 0.001;
        else sphere_origin.z += sin(2 * PI * sphere_time / 800.0) > 0 ? 0.003 : -0.003;
        sphere_time ++;
    }
    
    glUseProgram(program);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glUniformMatrix4fv(uniModel, 1, GL_FALSE, model.data());
    glUniformMatrix4fv(uniViewProjection, 1, GL_FALSE, viewProjection.data());
    glUniform1i(uniPause, (int)pause);
    glUniform3f(uniGravity, gravity.x, gravity.y, gravity.z);
    glUniform3f(uniWind, wind_accleration.x, wind_accleration.y, wind_accleration.z);
    glUniform3f(uniSphere_origin, sphere_origin.x, sphere_origin.y, sphere_origin.z);
    glUniform3f(uniPoint_plane, point_plane.x, point_plane.y, point_plane.z);
    glUniform3f(uniNormal_plane, normal_plane.x, normal_plane.y, normal_plane.z);
    glUniform1f(uniRadius, sphere_radius * 1.03);
    glUniform1f(uniDeltaT, delta_t);
    glUniform1f(uniDamping, 1.0 - cp -> damping / 100.0);
    glUniform1f(uniMass, mass);
    glUniform1f(uniKs, cp -> ks);
    glUniform1i(uniPoints, 0);
    glUniform1i(uniPinned, 1);
    glUniform1i(uniTexture, 2);
//    glUniform1i(uniFlag, 0);
//    glUniform1i(uniFlag, time % 2);
    glUniform1i(uniExist_sphere, exist_sphere);
    glUniform1i(uniExist_plane, exist_plane);
    glUniform1f(uniFriction_sphere, friction_sphere);
    glUniform1f(uniFriction_plane, friction_plane);
    glUniform1f(uniThickness, thickness);
    
//    glBeginTransformFeedback(GL_LINES);
//    glDrawElements(GL_LINES, 14502 * 2, GL_UNSIGNED_INT, 0);
//    glEndTransformFeedback();
    
//    glBeginTransformFeedback(GL_TRIANGLES);
//    glDrawElements(GL_TRIANGLES, 4802 * 3, GL_UNSIGNED_INT, 0);
//    glEndTransformFeedback();

    glBeginTransformFeedback(GL_POINTS);
    glDrawElements(GL_POINTS, 4802 * 3, GL_UNSIGNED_INT, 0);
    glEndTransformFeedback();
    
//    glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(feedback), feedback);
    glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(feedback_tri), feedback_tri);
    
    for (int i = 0; i < point_masses.size(); i ++) {
//        dataPos[len * i] = feedback[6 * point_masses[i].index_feedback];
//        dataPos[len * i + 1] = feedback[6 * point_masses[i].index_feedback + 1];
//        dataPos[len * i + 2] = feedback[6 * point_masses[i].index_feedback + 2];
//        dataPos[len * i + 4] = feedback[6 * point_masses[i].index_feedback + 3];
//        dataPos[len * i + 5] = feedback[6 * point_masses[i].index_feedback + 4];
//        dataPos[len * i + 6] = feedback[6 * point_masses[i].index_feedback + 5];
        dataPos[len * i] = feedback_tri[6 * point_masses[i].index_feedback_tri];
        dataPos[len * i + 1] = feedback_tri[6 * point_masses[i].index_feedback_tri + 1];
        dataPos[len * i + 2] = feedback_tri[6 * point_masses[i].index_feedback_tri + 2];
//        if (time % 2 == 0) {
            dataPos[len * i + 4] = feedback_tri[6 * point_masses[i].index_feedback_tri + 3];
            dataPos[len * i + 5] = feedback_tri[6 * point_masses[i].index_feedback_tri + 4];
            dataPos[len * i + 6] = feedback_tri[6 * point_masses[i].index_feedback_tri + 5];
////        }
        point_masses[i].position = Vector3D(dataPos[len * i], dataPos[len * i + 1], dataPos[len * i + 2]);
        points[3 * i] = dataPos[len * i];
        points[3 * i + 1] = dataPos[len * i + 1];
        points[3 * i + 2] = dataPos[len * i + 2];
    }
    
    for (int i = 0; i < point_masses.size(); i ++) {
        point_normal = point_masses[i].normal();
        dataPos[len * i + 19] = point_normal.x;
        dataPos[len * i + 20] = point_normal.y;
        dataPos[len * i + 21] = point_normal.z;
    }
//    printf("1 : %f %f %f ~ %f %f %f\n", dataPos[0], dataPos[1], dataPos[2], dataPos[4], dataPos[5], dataPos[6]);
    
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(dataPos), dataPos);
    
    glBindBuffer(GL_TEXTURE_BUFFER, points_buffer);
    glBufferSubData(GL_TEXTURE_BUFFER, 0, sizeof(points), points);
    glBindTexture(GL_TEXTURE_BUFFER, points_texture);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, points_buffer);
    
    glUseProgram(program_correction);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
//    glBindBuffer(GL_ARRAY_BUFFER, vbo_correction);
    glUniformMatrix4fv(uniViewProjection_correction, 1, GL_FALSE, viewProjection.data());
    glUniform1i(uniPause_correction, (int)pause);
    glUniform3f(uniSphere_origin_correction, sphere_origin.x, sphere_origin.y, sphere_origin.z);
    glUniform3f(uniPoint_plane_correction, point_plane.x, point_plane.y, point_plane.z);
    glUniform3f(uniNormal_plane_correction, normal_plane.x, normal_plane.y, normal_plane.z);
    glUniform1f(uniRadius_correction, sphere_radius * 1.03);
    glUniform1i(uniPoints_correction, 0);
    glUniform1i(uniPinned_correction, 1);
    glUniform1i(uniTexture_correction, 2);
    glUniform1i(uniExist_sphere_correction, exist_sphere);
    glUniform1i(uniExist_plane_correction, exist_plane);
    glUniform1f(uniFriction_sphere_correction, friction_sphere);
    glUniform1f(uniFriction_plane_correction, friction_plane);
    glUniform1f(uniThickness_correction, thickness);
    glUniform1i(uniUseTexture_correction, (int)use_texture);
    
    glBeginTransformFeedback(GL_TRIANGLES);
    glDrawElements(GL_TRIANGLES, 4802 * 3, GL_UNSIGNED_INT, 0);
    glEndTransformFeedback();
    
    glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(feedback_tri), feedback_tri);
    
    for (int i = 0; i < point_masses.size(); i ++) {
        dataPos[len * i] = feedback_tri[6 * point_masses[i].index_feedback_tri];
        dataPos[len * i + 1] = feedback_tri[6 * point_masses[i].index_feedback_tri + 1];
        dataPos[len * i + 2] = feedback_tri[6 * point_masses[i].index_feedback_tri + 2];
//        dataPos[len * i + 4] = feedback_tri[6 * point_masses[i].index_feedback_tri + 3];
//        dataPos[len * i + 5] = feedback_tri[6 * point_masses[i].index_feedback_tri + 4];
//        dataPos[len * i + 6] = feedback_tri[6 * point_masses[i].index_feedback_tri + 5];
        point_masses[i].position = Vector3D(dataPos[len * i], dataPos[len * i + 1], dataPos[len * i + 2]);
        points[3 * i] = dataPos[len * i];
        points[3 * i + 1] = dataPos[len * i + 1];
        points[3 * i + 2] = dataPos[len * i + 2];
    }
    
    for (int i = 0; i < point_masses.size(); i ++) {
        point_normal = point_masses[i].normal();
        dataPos[len * i + 19] = point_normal.x;
        dataPos[len * i + 20] = point_normal.y;
        dataPos[len * i + 21] = point_normal.z;
    }
    
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(dataPos), dataPos);
    
//    printf("2 : %f %f %f ~ %f %f %f\n", dataPos[0], dataPos[1], dataPos[2], dataPos[4], dataPos[5], dataPos[6]);
    
    glBindBuffer(GL_TEXTURE_BUFFER, points_buffer);
    glBufferSubData(GL_TEXTURE_BUFFER, 0, sizeof(points), points);
    glBindTexture(GL_TEXTURE_BUFFER, points_texture);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, points_buffer);
 
    if (exist_sphere) {
        glUseProgram(program_sphere);
        model_sphere << sphere_radius, 0, 0, sphere_origin.x, 0, sphere_radius, 0, sphere_origin.y, 0, 0, sphere_radius, sphere_origin.z, 0, 0, 0, 1;
        glUniformMatrix4fv(uniModel_sphere, 1, GL_FALSE, model_sphere.data());
        glUniformMatrix4fv(uniViewProjection_sphere, 1, GL_FALSE, viewProjection.data());
        glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere);
        glDrawArrays(GL_TRIANGLES, 0, num_points_sphere * 3);
    }
    
    if (exist_plane) {
        glUseProgram(program_plane);
        glUniformMatrix4fv(uniViewProjection_plane, 1, GL_FALSE, viewProjection.data());
        glBindBuffer(GL_ARRAY_BUFFER, vbo_plane);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    /*
  // TODO (Part 2): Compute total force acting on each point mass.
    Vector3D external_force = Vector3D(0.0, 0.0, 0.0);
    for (int i = 0; i < external_accelerations.size(); i ++) {
        external_force += external_accelerations[i] * mass;
    }
    for (int i = 0; i < point_masses.size(); i ++) {
        point_masses[i].forces = external_force;
    }
    
    if (wind.norm()) {
        for (int i = 0; i < clothMesh -> triangles.size(); i ++) {
            Vector3D pos1 = clothMesh -> triangles[i] -> pm1 -> position;
            Vector3D pos2 = clothMesh -> triangles[i] -> pm2 -> position;
            Vector3D pos3 = clothMesh -> triangles[i] -> pm3 -> position;
            Vector3D v1 = pos2 - pos1;
            Vector3D v2 = pos3 - pos1;
            Vector3D normal = cross(v1, v2);
            normal.normalize();
            Vector3D windDirection = wind / wind.norm();
            Vector3D force = wind * dot(normal, windDirection) * mass;
            clothMesh -> triangles[i] -> pm1 -> forces += force;
            clothMesh -> triangles[i] -> pm2 -> forces += force;
            clothMesh -> triangles[i] -> pm3 -> forces += force;
        }
    }
    
    for (int i = 0; i < springs.size(); i ++) {
        Spring spring = springs[i];
        Vector3D direction = spring.pm_b -> position - spring.pm_a -> position;
        direction.normalize();
        double length = (spring.pm_b -> position - spring.pm_a -> position).norm();
        if ((spring.spring_type == STRUCTURAL && cp -> enable_structural_constraints) ||
            (spring.spring_type == BENDING && cp -> enable_bending_constraints) ||
            (spring.spring_type == SHEARING && cp -> enable_shearing_constraints)) {
            Vector3D force = cp -> ks * (length - spring.rest_length) * direction;
            if (spring.spring_type == BENDING) {
                force = force * 0.2;
            }
            spring.pm_a -> forces += force;
            spring.pm_b -> forces += -force;
        }
    }
    
  // TODO (Part 2): Use Verlet integration to compute new point mass positions
    for (int i = 0; i < point_masses.size(); i ++) {
        if (point_masses[i].pinned) continue;
        Vector3D a = point_masses[i].forces / mass;
        Vector3D new_postion = point_masses[i].position + (1 - cp -> damping / 100.0) * (point_masses[i].position - point_masses[i].last_position) + a * delta_t * delta_t;
        point_masses[i].last_position = point_masses[i].position;
        point_masses[i].position = new_postion;
    }
    
  // TODO (Part 4): Handle self-collisions.
    build_spatial_map();
    for (int i = 0; i < point_masses.size(); i ++) {
        self_collide(point_masses[i], simulation_steps);
     }
    
  // TODO (Part 3): Handle collisions with other primitives.
    for (int i = 0; i < point_masses.size(); i ++) {
        for (CollisionObject *object : *collision_objects) {
            object -> collide(point_masses[i]);
        }
    }
  // TODO (Part 2): Constrain the changes to be such that the spring does not change
  // in length more than 10% per timestep [Provot 1995].
    for (int i = 0; i < springs.size(); i ++) {
        Spring spring = springs[i];
        Vector3D direction = spring.pm_b -> position - spring.pm_a -> position;
        direction.normalize();
        double length = (spring.pm_b -> position - spring.pm_a -> position).norm();
        if (length > spring.rest_length * 1.1) {
            double len = length - spring.rest_length * 1.1;
            if (spring.pm_a -> pinned && !spring.pm_b -> pinned) {
                spring.pm_b -> position -= direction * len;
            }
            if (!spring.pm_a -> pinned && spring.pm_b -> pinned) {
                spring.pm_a -> position += direction * len;
            }
            if (!spring.pm_a -> pinned && !spring.pm_b -> pinned) {
                spring.pm_a -> position += direction * (len / 2);
                spring.pm_b -> position -= direction * (len / 2);
            }
        }
    }
     */
}

void Cloth::build_spatial_map() {
  for (const auto &entry : map) {
    delete(entry.second);
  }
  map.clear();

  // TODO (Part 4): Build a spatial map out of all of the point masses.
    for (int i = 0; i < point_masses.size(); i ++) {
        double h = hash_position(point_masses[i].position);
        if (map.find(h) == map.end()) {
            map[h] = new vector<PointMass *>;
        }
        map[h] -> push_back(&point_masses[i]);
    }

}

void Cloth::self_collide(PointMass &pm, double simulation_steps) {
  // TODO (Part 4): Handle self-collision for a given point mass.
    int num = 0;
    Vector3D correction_vector = Vector3D(0, 0, 0);
    double hash = hash_position(pm.position);
    vector<PointMass *> *points = map[hash];
    for (PointMass *p : *points) {
        if (p -> position == pm.position) continue;
        double distance = (p -> position - pm.position).norm();
        if (distance < 2 * thickness) {
            Vector3D direction = pm.position - p -> position;
            direction.normalize();
            correction_vector += direction * (2 * thickness - distance);
            num ++;
        }
    }
    if (num) {
        correction_vector = correction_vector / (double)num / simulation_steps;
        pm.position += correction_vector;
    }
}

float Cloth::hash_position(Vector3D pos) {
  // TODO (Part 4): Hash a 3D position into a unique float identifier that represents membership in some 3D box volume.
    double w = 3.0 * width / num_width_points;
    double h = 3.0 * height / num_height_points;
    double t = max(w, h);
    
    double x = (pos.x - fmod(pos.x, w)) / w;
    double y = (pos.y - fmod(pos.y, h)) / h;
    double z = (pos.z - fmod(pos.z, t)) / t;
    return pow(x, 5) + pow(y, 7) + pow(z, 11);
}

///////////////////////////////////////////////////////
/// YOU DO NOT NEED TO REFER TO ANY CODE BELOW THIS ///
///////////////////////////////////////////////////////

void Cloth::reset() {
  PointMass *pm = &point_masses[0];
  for (int i = 0; i < point_masses.size(); i++) {
    pm->position = pm->start_position;
    pm->last_position = pm->start_position;
    pm++;
  }
}

void Cloth::buildClothMesh() {
  if (point_masses.size() == 0) return;

  ClothMesh *clothMesh = new ClothMesh();
  vector<Triangle *> triangles;

  // Create vector of triangles
  for (int y = 0; y < num_height_points - 1; y++) {
    for (int x = 0; x < num_width_points - 1; x++) {
      PointMass *pm = &point_masses[y * num_width_points + x];
      // Get neighboring point masses:
      /*                      *
       * pm_A -------- pm_B   *
       *             /        *
       *  |         /   |     *
       *  |        /    |     *
       *  |       /     |     *
       *  |      /      |     *
       *  |     /       |     *
       *  |    /        |     *
       *      /               *
       * pm_C -------- pm_D   *
       *                      *
       */
      
      float u_min = x;
      u_min /= num_width_points - 1;
      float u_max = x + 1;
      u_max /= num_width_points - 1;
      float v_min = y;
      v_min /= num_height_points - 1;
      float v_max = y + 1;
      v_max /= num_height_points - 1;
      
      PointMass *pm_A = pm                       ;
      PointMass *pm_B = pm                    + 1;
      PointMass *pm_C = pm + num_width_points    ;
      PointMass *pm_D = pm + num_width_points + 1;
      
      Vector3D uv_A = Vector3D(u_min, v_min, 0);
      Vector3D uv_B = Vector3D(u_max, v_min, 0);
      Vector3D uv_C = Vector3D(u_min, v_max, 0);
      Vector3D uv_D = Vector3D(u_max, v_max, 0);
      
      
      // Both triangles defined by vertices in counter-clockwise orientation
      triangles.push_back(new Triangle(pm_A, pm_C, pm_B, 
                                       uv_A, uv_C, uv_B));
      triangles.push_back(new Triangle(pm_B, pm_C, pm_D, 
                                       uv_B, uv_C, uv_D));
    }
  }

  // For each triangle in row-order, create 3 edges and 3 internal halfedges
  for (int i = 0; i < triangles.size(); i++) {
    Triangle *t = triangles[i];

    // Allocate new halfedges on heap
    Halfedge *h1 = new Halfedge();
    Halfedge *h2 = new Halfedge();
    Halfedge *h3 = new Halfedge();

    // Allocate new edges on heap
    Edge *e1 = new Edge();
    Edge *e2 = new Edge();
    Edge *e3 = new Edge();

    // Assign a halfedge pointer to the triangle
    t->halfedge = h1;

    // Assign halfedge pointers to point masses
    t->pm1->halfedge = h1;
    t->pm2->halfedge = h2;
    t->pm3->halfedge = h3;

    // Update all halfedge pointers
    h1->edge = e1;
    h1->next = h2;
    h1->pm = t->pm1;
    h1->triangle = t;

    h2->edge = e2;
    h2->next = h3;
    h2->pm = t->pm2;
    h2->triangle = t;

    h3->edge = e3;
    h3->next = h1;
    h3->pm = t->pm3;
    h3->triangle = t;
  }

  // Go back through the cloth mesh and link triangles together using halfedge
  // twin pointers

  // Convenient variables for math
  int num_height_tris = (num_height_points - 1) * 2;
  int num_width_tris = (num_width_points - 1) * 2;

  bool topLeft = true;
  for (int i = 0; i < triangles.size(); i++) {
    Triangle *t = triangles[i];

    if (topLeft) {
      // Get left triangle, if it exists
      if (i % num_width_tris != 0) { // Not a left-most triangle
        Triangle *temp = triangles[i - 1];
        t->pm1->halfedge->twin = temp->pm3->halfedge;
      } else {
        t->pm1->halfedge->twin = nullptr;
      }

      // Get triangle above, if it exists
      if (i >= num_width_tris) { // Not a top-most triangle
        Triangle *temp = triangles[i - num_width_tris + 1];
        t->pm3->halfedge->twin = temp->pm2->halfedge;
      } else {
        t->pm3->halfedge->twin = nullptr;
      }

      // Get triangle to bottom right; guaranteed to exist
      Triangle *temp = triangles[i + 1];
      t->pm2->halfedge->twin = temp->pm1->halfedge;
    } else {
      // Get right triangle, if it exists
      if (i % num_width_tris != num_width_tris - 1) { // Not a right-most triangle
        Triangle *temp = triangles[i + 1];
        t->pm3->halfedge->twin = temp->pm1->halfedge;
      } else {
        t->pm3->halfedge->twin = nullptr;
      }

      // Get triangle below, if it exists
      if (i + num_width_tris - 1 < 1.0f * num_width_tris * num_height_tris / 2.0f) { // Not a bottom-most triangle
        Triangle *temp = triangles[i + num_width_tris - 1];
        t->pm2->halfedge->twin = temp->pm3->halfedge;
      } else {
        t->pm2->halfedge->twin = nullptr;
      }

      // Get triangle to top left; guaranteed to exist
      Triangle *temp = triangles[i - 1];
      t->pm1->halfedge->twin = temp->pm2->halfedge;
    }

    topLeft = !topLeft;
  }

  clothMesh->triangles = triangles;
  this->clothMesh = clothMesh;
}

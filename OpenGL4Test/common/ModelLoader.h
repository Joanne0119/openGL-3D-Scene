// ModelLoader.h
#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include "tiny_obj_loader.h"

struct VertexAll {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;
};

class ModelLoader {
public:
    ModelLoader();
    ~ModelLoader();

    bool LoadModel(const std::string& path);
    void Render();

private:
    GLuint VAO, VBO;
    std::vector<VertexAll> vertices;

    void SetupMesh();
};

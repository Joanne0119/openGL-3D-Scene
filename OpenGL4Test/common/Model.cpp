#include "Model.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

// STBI 用於載入紋理圖片
//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


Model::~Model() {
    Cleanup();
}

bool Model::LoadModel(const std::string& filepath) {
    // 清理之前的資源
    Cleanup();
    
    // 取得檔案目錄
    directory = GetDirectory(filepath);
    
    // TinyObjLoader 變數
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> objMaterials;
    std::string warn, err;
    
    // 載入 OBJ 檔案
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &objMaterials, &warn, &err,
                               filepath.c_str(), directory.c_str());
    
    if (!warn.empty()) {
        std::cout << "Warning: " << warn << std::endl;
    }
    
    if (!err.empty()) {
        std::cerr << "Error: " << err << std::endl;
        return false;
    }
    
    if (!ret) {
        std::cerr << "Failed to load model: " << filepath << std::endl;
        return false;
    }
    
    // 檢查是否有頂點資料
    if (attrib.vertices.empty()) {
        std::cerr << "No vertices found in OBJ file!" << std::endl;
        return false;
    }
    
    // 處理材質
    ProcessMaterials(objMaterials);
    
    // 處理每個形狀（網格）
    for (const auto& shape : shapes) {
        ProcessMesh(attrib, shape, objMaterials);
    }
    
    std::cout << "Successfully loaded model: " << filepath << std::endl;
    std::cout << "Meshes: " << meshes.size() << ", Materials: " << materials.size() << std::endl;
    
    return true;
}

void Model::ProcessMaterials(const std::vector<tinyobj::material_t>& objMaterials) {
    materials.reserve(objMaterials.size());
    
    for (const auto& objMat : objMaterials) {
        Material mat;
        mat.name = objMat.name;
        
        // 複製顏色屬性
        for (int i = 0; i < 3; i++) {
            mat.ambient[i] = objMat.ambient[i];
            mat.diffuse[i] = objMat.diffuse[i];
            mat.specular[i] = objMat.specular[i];
        }
        mat.shininess = objMat.shininess;
        
        // 處理透明度
        mat.alpha = objMat.dissolve;  // TinyObjLoader 中的 dissolve 就是透明度
        if (mat.alpha <= 0.0f) mat.alpha = 1.0f;  // 預設為不透明
        
        std::cout << "Material: " << mat.name << ", Alpha: " << mat.alpha << std::endl;
        
        
        // 載入紋理
        if (!objMat.diffuse_texname.empty()) {
            mat.diffuseTexPath = directory + "/" + objMat.diffuse_texname;
            mat.diffuseTexture = LoadTexture(mat.diffuseTexPath);
        }
        
        if (!objMat.normal_texname.empty()) {
            mat.normalTexPath = directory + "/" + objMat.normal_texname;
            mat.normalTexture = LoadTexture(mat.normalTexPath);
        }
        
        if (!objMat.specular_texname.empty()) {
            mat.specularTexPath = directory + "/" + objMat.specular_texname;
            mat.specularTexture = LoadTexture(mat.specularTexPath);
        }
        
        materials.push_back(mat);
    }
}

void Model::ProcessMesh(const tinyobj::attrib_t& attrib,
                       const tinyobj::shape_t& shape,
                       const std::vector<tinyobj::material_t>& objMaterials) {
    Mesh mesh;
    std::unordered_map<std::string, unsigned int> uniqueVertices;

    std::cout << "  Processing mesh with " << shape.mesh.indices.size() / 3 << " faces" << std::endl; // 顯示面數

    // 處理每個面
    for (size_t f = 0; f < shape.mesh.indices.size(); f += 3) { // 一次處理三個索引（一個三角形）
        for (int i = 0; i < 3; ++i) { // 每個頂點
            const auto& index = shape.mesh.indices[f + i];

            Vertex vertex;

            // 位置
            if (index.vertex_index >= 0) {
                vertex.position[0] = attrib.vertices[3 * index.vertex_index + 0];
                vertex.position[1] = attrib.vertices[3 * index.vertex_index + 1];
                vertex.position[2] = attrib.vertices[3 * index.vertex_index + 2];
            } else {
                std::cerr << "    Warning: Vertex position index is negative!" << std::endl;
                vertex.position[0] = vertex.position[1] = vertex.position[2] = 0.0f;
            }

            // 法向量
            if (index.normal_index >= 0) {
                vertex.normal[0] = attrib.normals[3 * index.normal_index + 0];
                vertex.normal[1] = attrib.normals[3 * index.normal_index + 1];
                vertex.normal[2] = attrib.normals[3 * index.normal_index + 2];
            } else {
                // 如果沒有法向量，設為預設值
                std::cerr << "    Warning: Normal index is negative or missing!" << std::endl;
                vertex.normal[0] = 0.0f;
                vertex.normal[1] = 1.0f;
                vertex.normal[2] = 0.0f;
            }

            // 紋理坐標
            if (index.texcoord_index >= 0) {
                vertex.texCoords[0] = attrib.texcoords[2 * index.texcoord_index + 0];
                vertex.texCoords[1] = attrib.texcoords[2 * index.texcoord_index + 1];
            } else {
                std::cerr << "    Warning: TexCoord index is negative or missing!" << std::endl;
                vertex.texCoords[0] = 0.0f;
                vertex.texCoords[1] = 0.0f;
            }

            // 建立唯一頂點標識符
            std::ostringstream oss;
            oss << index.vertex_index << "_" << index.normal_index << "_" << index.texcoord_index;
            std::string vertexKey = oss.str();

            // 檢查是否為重複頂點
            if (uniqueVertices.find(vertexKey) == uniqueVertices.end()) {
                uniqueVertices[vertexKey] = static_cast<unsigned int>(mesh.vertices.size());
                mesh.vertices.push_back(vertex);

            }

            mesh.indices.push_back(uniqueVertices[vertexKey]);
        }
    }

    // 設定材質索引
    if (!shape.mesh.material_ids.empty() && shape.mesh.material_ids[0] >= 0) {
        mesh.materialIndex = shape.mesh.material_ids[0];
        std::cout << "  Mesh material index: " << mesh.materialIndex << std::endl;
    } else {
        mesh.materialIndex = -1; // 沒有材質
        std::cout << "  Mesh has no material index" << std::endl;
    }

    // 設置 OpenGL 緩衝區
    SetupMesh(mesh);

    meshes.push_back(mesh);
}

void Model::SetupMesh(Mesh& mesh) {
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);
    
    glBindVertexArray(mesh.VAO);
    
    // 頂點緩衝區
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex),
                 mesh.vertices.data(), GL_STATIC_DRAW);
    
    // 索引緩衝區
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int),
                 mesh.indices.data(), GL_STATIC_DRAW);
    
    // 頂點屬性
    // 位置
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 法向量
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                         (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    
    // 紋理坐標
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                         (void*)offsetof(Vertex, texCoords));
    glEnableVertexAttribArray(3);
    
    glBindVertexArray(0);
}

GLuint Model::LoadTexture(const std::string& path) {
    // 檢查檔案是否存在
    std::ifstream file(path);
    if (!file) {
        std::cout << "Texture file not found: " << path << std::endl;
        return 0;
    }
    file.close();
    
    // 清除之前的 OpenGL 錯誤
    while (glGetError() != GL_NO_ERROR);
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    
    // 檢查 glGenTextures 是否成功
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cout << "Error generating texture: " << error << std::endl;
        return 0;
    }
    
    stbi_set_flip_vertically_on_load(true);
    
    int width, height, nrComponents;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    
    if (data && width > 0 && height > 0) {
        GLenum format;
        GLenum internalFormat;
        
        if (nrComponents == 1) {
            format = GL_RED;
            internalFormat = GL_RED;
        }
        else if (nrComponents == 3) {
            format = GL_RGB;
            internalFormat = GL_RGB8;
        }
        else if (nrComponents == 4) {
            format = GL_RGBA;
            internalFormat = GL_RGBA8;
        } else {
            std::cout << "Unsupported texture format: " << nrComponents << " components in " << path << std::endl;
            stbi_image_free(data);
            glDeleteTextures(1, &textureID);
            return 0;
        }
        
        // 綁定紋理前確保沒有其他紋理綁定
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        // 檢查綁定是否成功
        error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cout << "Error binding texture: " << error << std::endl;
            stbi_image_free(data);
            glDeleteTextures(1, &textureID);
            return 0;
        }
        
        // 設置紋理參數 (在上傳數據前設置)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // 上傳紋理數據
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        
        // 檢查紋理上傳是否成功
        error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cout << "Failed to upload texture data: " << error << " for " << path << std::endl;
            stbi_image_free(data);
            glDeleteTextures(1, &textureID);
            return 0;
        }
        
        // 生成 mipmap
        glGenerateMipmap(GL_TEXTURE_2D);
        
        // 檢查 mipmap 生成是否成功
        error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cout << "Error generating mipmap: " << error << std::endl;
            // 不返回錯誤，因為紋理本身已經上傳成功
        }
        
        // 解綁紋理
        glBindTexture(GL_TEXTURE_2D, 0);
        
        stbi_image_free(data);
        std::cout << "Successfully loaded texture: " << path << " (ID: " << textureID << ", " << width << "x" << height << ", " << nrComponents << " components)" << std::endl;
        
        return textureID;
        
    } else {
        std::cout << "Failed to load texture data: " << path;
        if (data == nullptr) {
            std::cout << " - STBI error: " << stbi_failure_reason();
        }
        std::cout << std::endl;
        
        if (data) stbi_image_free(data);
        glDeleteTextures(1, &textureID);
        return 0;
    }
}

void Model::Render(GLuint shaderProgram) {
    // 確保 shader 程式是當前使用的
    glUseProgram(shaderProgram);
    // 如果有透明物體，需要啟用混合
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (size_t i = 0; i < meshes.size(); i++) {
        const Mesh& mesh = meshes[i];

        std::cout << "  Rendering mesh " << i << " (Material Index: " << mesh.materialIndex << ")" << std::endl;

        // 重置紋理單元 (確保乾淨的狀態)
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE3);  // 透明度貼圖
        glBindTexture(GL_TEXTURE_2D, 0);


        // 設置預設值 (如果沒有材質)
        glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasDiffuseTexture"), 0);
        glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasNormalTexture"), 0);
        glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasSpecularTexture"), 0);
        glUniform1f(glGetUniformLocation(shaderProgram, "uMaterial.alpha"), 1.0f);

        // 綁定材質
        if (mesh.materialIndex >= 0 && mesh.materialIndex < materials.size()) {
            const Material& material = materials[mesh.materialIndex];

            std::cout << "  Applying material: " << material.name << std::endl;

            // 設定材質屬性 uniform 變數
            GLint ambientLoc = glGetUniformLocation(shaderProgram, "uMaterial.ambient");
            GLint diffuseLoc = glGetUniformLocation(shaderProgram, "uMaterial.diffuse");
            GLint specularLoc = glGetUniformLocation(shaderProgram, "uMaterial.specular");
            GLint shininessLoc = glGetUniformLocation(shaderProgram, "uMaterial.shininess");
            GLint alphaLoc = glGetUniformLocation(shaderProgram, "uMaterial.alpha");

            if (ambientLoc != -1) {
                glUniform4f(ambientLoc, material.ambient[0], material.ambient[1], material.ambient[2], 1.0f);
            }
            if (diffuseLoc != -1) {
                glUniform4f(diffuseLoc, material.diffuse[0], material.diffuse[1], material.diffuse[2], 1.0f);
            }

            if (specularLoc != -1) {
                glUniform4f(specularLoc, material.specular[0], material.specular[1], material.specular[2], 1.0f);
            }

            if (shininessLoc != -1) {
                glUniform1f(shininessLoc, material.shininess);
            }
            if (alphaLoc != -1) {
                glUniform1f(alphaLoc, material.alpha);  // 設定材質透明度
            }


            // 綁定漫反射紋理
            if (material.diffuseTexture != 0) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, material.diffuseTexture);
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.diffuseTexture"), 0);
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasDiffuseTexture"), 1); // 重要！
            } else {
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasDiffuseTexture"), 0);
                std::cout << "    No diffuse texture" << std::endl;
            }

            // 綁定法線貼圖
            if (material.normalTexture != 0) {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, material.normalTexture);
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.normalTexture"), 1);
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasNormalTexture"), 1);
            } else {
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasNormalTexture"), 0);
                std::cout << "    No normal texture" << std::endl;
            }

            // 綁定鏡面反射貼圖
            if (material.specularTexture != 0) {
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, material.specularTexture);
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.specularTexture"), 2);
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasSpecularTexture"), 1);
            } else {
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasSpecularTexture"), 0);
                std::cout << "    No specular texture" << std::endl;
            }
            // 綁定透明度貼圖
            if (material.alphaTexture != 0) {
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, material.alphaTexture);
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.alphaTexture"), 3);
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasAlphaTexture"), 1);
                std::cout << "    Using alpha texture" << std::endl;
            }

        } else {
            std::cout << "  Mesh has no material assigned" << std::endl;
        }

        // 渲染網格
        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // 檢查 OpenGL 錯誤
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "  OpenGL error during rendering: " << error << std::endl;
        }
    }

    // 清理紋理綁定
    for (int i = 0; i < 4; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void Model::Cleanup() {
    for (auto& mesh : meshes) {
        if (mesh.VAO != 0) glDeleteVertexArrays(1, &mesh.VAO);
        if (mesh.VBO != 0) glDeleteBuffers(1, &mesh.VBO);
        if (mesh.EBO != 0) glDeleteBuffers(1, &mesh.EBO);
    }
    
    for (auto& material : materials) {
        if (material.diffuseTexture != 0) glDeleteTextures(1, &material.diffuseTexture);
        if (material.normalTexture != 0) glDeleteTextures(1, &material.normalTexture);
        if (material.specularTexture != 0) glDeleteTextures(1, &material.specularTexture);
        if (material.alphaTexture != 0) glDeleteTextures(1, &material.alphaTexture);
    }
    
    meshes.clear();
    materials.clear();
}

const Material& Model::GetMaterial(size_t index) const {
    if (index >= materials.size()) {
        throw std::out_of_range("Material index out of range");
    }
    return materials[index];
}

std::string Model::GetDirectory(const std::string& filepath) {
    size_t pos = filepath.find_last_of('/');
    if (pos == std::string::npos) {
        pos = filepath.find_last_of('\\');
    }
    
    if (pos != std::string::npos) {
        return filepath.substr(0, pos);
    }
    
    return ".";
}

void Model::setAutoRotate()
{
    if( !_bautoRotate ) _bautoRotate = true;
}

void Model::update(float dt)
{
    updateCameraFollow();
    
    float _boundaryLeft = -20.0f;
    float _boundaryRight = 20.0f;
    float _boundaryTop = 20.0f;
    float _boundaryBottom = -20.0f;
    bool shouldMove = _bautoRotate;
    if (shouldMove) {
        // 移動位置
       glm::vec3 delta = _direction * _speed * dt;
       _position += delta;

       // 判斷是否碰到邊界 → 如果碰到了就轉彎（右轉 90 度）
        if (_position.x > _boundaryRight && _direction.x > 0) {
            _position.x = _boundaryRight;
            _direction = glm::vec3(0.0f, 0.0f, -1.0f);
            _targetAngle = glm::radians(180.0f); // 往 -Z
        }
        else if (_position.z < _boundaryBottom && _direction.z < 0) {
            _position.z = _boundaryBottom;
            _direction = glm::vec3(-1.0f, 0.0f, 0.0f);
            _targetAngle = glm::radians(270.0f); // 往 -X
        }
        else if (_position.x < _boundaryLeft && _direction.x < 0) {
            _position.x = _boundaryLeft;
            _direction = glm::vec3(0.0f, 0.0f, 1.0f);
            _targetAngle = glm::radians(0.0f);   // 往 +Z
        }
        else if (_position.z > _boundaryTop && _direction.z > 0) {
            _position.z = _boundaryTop;
            _direction = glm::vec3(1.0f, 0.0f, 0.0f);
            _targetAngle = glm::radians(90.0f);  // 往 +X
        }
        
        float angleDiff = _targetAngle - _currentAngle;

        // 保證角度在 -π 到 π 範圍內，避免繞遠路
        if (angleDiff > glm::pi<float>()) angleDiff -= glm::two_pi<float>();
        if (angleDiff < -glm::pi<float>()) angleDiff += glm::two_pi<float>();

        // 根據旋轉速度與 dt 計算要轉多少
        float maxStep = _rotationSpeed * dt;
        if (glm::abs(angleDiff) < maxStep) {
            _currentAngle = _targetAngle; // 到了就貼齊
        } else {
            _currentAngle += glm::sign(angleDiff) * maxStep;
        }

        _modelMatrix = glm::translate(glm::mat4(1.0f), _position);
        _modelMatrix = glm::rotate(_modelMatrix, _currentAngle, glm::vec3(0.0f, 1.0f, 0.0f));

       std::cout << "=== MODEL TRACK MOVEMENT ===" << std::endl;
       std::cout << "Position: (" << _position.x << ", " << _position.y << ", " << _position.z << ")" << std::endl;
       std::cout << "Direction: (" << _direction.x << ", " << _direction.y << ", " << _direction.z << ")" << std::endl;
       std::cout << "============================" << std::endl;
    }
}
void Model::setFollowCamera(bool follow, const glm::vec3& offset, bool followRotation = true, float rotationOffset = 0.0f) {
    _followCamera = follow;
    _cameraOffset = offset;
    _followCameraRotation = followRotation;
    _rotationOffset = rotationOffset;
    
    if (follow) {
        std::cout << "Model now following camera with offset: ("
                  << offset.x << ", " << offset.y << ", " << offset.z << ")";
        if (followRotation) {
            std::cout << " and rotation with offset: " << glm::degrees(rotationOffset) << " degrees";
        }
        std::cout << std::endl;
    } else {
        std::cout << "Model stopped following camera" << std::endl;
    }
}

void Model::updateCameraFollow() {
    if (!_followCamera) return;
    
    // 獲取攝影機位置
    glm::vec3 cameraPos = _cameraPos;
    
    // 獲取攝影機的前方、右方、上方向量
    glm::mat4 viewMatrix = _viewMatrix;
    glm::vec3 cameraForward = -glm::vec3(viewMatrix[0][2], viewMatrix[1][2], viewMatrix[2][2]);
    glm::vec3 cameraRight = glm::vec3(viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]);
    glm::vec3 cameraUp = glm::vec3(viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]);
    
    // 計算物件應該在的位置
    // offset.x = 右方偏移, offset.y = 上方偏移, offset.z = 前方偏移
    glm::vec3 targetPosition = cameraPos +
                              cameraRight * _cameraOffset.x +
                              cameraUp * _cameraOffset.y +
                              cameraForward * _cameraOffset.z;
    
    // 更新物件位置
    _position = targetPosition;
    
    float targetAngle = _currentAngle;
    
    if (_followCameraRotation) {
            // 方法1：根據攝影機前方向量計算 Y 軸旋轉角度
            targetAngle = atan2(cameraForward.x, cameraForward.z) + _rotationOffset;
            
            // 或者使用方法2：更精確的四元數轉換
            // glm::quat cameraRotation = glm::conjugate(glm::quat_cast(viewMatrix));
            // glm::vec3 eulerAngles = glm::eulerAngles(cameraRotation);
            // targetAngle = eulerAngles.y + _rotationOffset;
        }
    
    // 更新模型矩陣 - 只有位移，保持原有的旋轉
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), _position);
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), targetAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    _modelMatrix = translation * rotation;
    
    if (_followCameraRotation) {
        _currentAngle = targetAngle;
    }
    
    static int frameCount = 0;
    frameCount++;
    if (frameCount % 60 == 0) {
        std::cout << "Following model - Position: (" << _position.x << ", " << _position.y << ", " << _position.z << ")";
        if (_followCameraRotation) {
            std::cout << ", Rotation: " << glm::degrees(targetAngle) << " degrees";
        }
        std::cout << std::endl;
    }
}

void Model::setCameraPos(const glm::vec3& pos) {
    _cameraPos = pos;
     std::cout << "Model received camera pos: (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
}

void Model::setViewMatrix(const glm::mat4& viewMatrix) {
    _viewMatrix = viewMatrix;
    // Debug 輸出 - 只打印一次避免過多輸出
    static int debugCount = 0;
    if (debugCount < 5) {
        std::cout << "Model received view matrix update " << debugCount << std::endl;
        debugCount++;
    }
}



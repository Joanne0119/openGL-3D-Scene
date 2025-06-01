// ModelManager.h
#pragma once
#include "OBJLoader.h"
#include "Transform.h"
#include <GL/glew.h>
#include <vector>
#include <string>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <iostream>
#include <filesystem>


// 材質結構
struct Material {
    GLuint diffuseTexture;      // 漫反射紋理
    GLuint normalTexture;       // 法線貼圖
    GLuint specularTexture;     // 鏡面反射紋理
    
    glm::vec3 ambient;          // 環境光顏色
    glm::vec3 diffuse;          // 漫反射顏色
    glm::vec3 specular;         // 鏡面反射顏色
    float shininess;            // 光澤度
    float alpha;                // 透明度
    
    std::string name;
    
    Material() : diffuseTexture(0), normalTexture(0), specularTexture(0),
                 ambient(0.2f, 0.2f, 0.2f), diffuse(0.8f, 0.8f, 0.8f),
                 specular(1.0f, 1.0f, 1.0f), shininess(32.0f), alpha(1.0f) {}
    
    void cleanup() {
        if (diffuseTexture != 0) {
            glDeleteTextures(1, &diffuseTexture);
            diffuseTexture = 0;
        }
        if (normalTexture != 0) {
            glDeleteTextures(1, &normalTexture);
            normalTexture = 0;
        }
        if (specularTexture != 0) {
            glDeleteTextures(1, &specularTexture);
            specularTexture = 0;
        }
    }
};

// 子模型結構（用於多材質模型）
struct SubModel {
    GLuint VAO;
    GLuint VBO;
    int vertexCount;
    std::string materialName;
    
    SubModel() : VAO(0), VBO(0), vertexCount(0) {}
    
    void cleanup() {
        if (VAO != 0) {
            glDeleteVertexArrays(1, &VAO);
            VAO = 0;
        }
        if (VBO != 0) {
            glDeleteBuffers(1, &VBO);
            VBO = 0;
        }
    }
};

struct Model {
    std::vector<SubModel> subModels;  // 支援多材質的子模型
    std::string name;
    Transform transform;
    
    // 向後兼容的屬性（單一材質模型使用）
    GLuint VAO;
    GLuint VBO;
    int vertexCount;
    std::string materialName;
    
    Model() : VAO(0), VBO(0), vertexCount(0) {}
    
    void cleanup() {
        // 清理子模型
        for (auto& subModel : subModels) {
            subModel.cleanup();
        }
        subModels.clear();
        
        // 清理主模型
        if (VAO != 0) {
            glDeleteVertexArrays(1, &VAO);
            VAO = 0;
        }
        if (VBO != 0) {
            glDeleteBuffers(1, &VBO);
            VBO = 0;
        }
    }
    
    glm::mat4 getModelMatrix() const {
        return transform.getMatrix();
    }
    
    bool hasMultipleMaterials() const {
        return !subModels.empty();
    }
};

class ModelManager {
private:
    std::map<std::string, Model> models;
    std::map<std::string, Material> materials;
    OBJLoader loader;
    
    // 從文件載入紋理的輔助函數
    GLuint loadTextureFromFile(const std::string& filepath);
    
    // 創建默認紋理的輔助函數
    GLuint createDefaultTexture(glm::vec3 color = glm::vec3(1.0f));
    
    // 從 MTL 材質創建 Material 的輔助函數
    void createMaterialFromMTL(const MTLMaterial& mtlMaterial, const std::string& basePath);
    
    // 創建子模型的輔助函數
    void createSubModel(const std::vector<Face>& faces,
                       const std::string& materialName,
                       SubModel& subModel,
                       const OBJLoader& objLoader);
    void bindMaterial(const std::string& materialName, GLuint shaderProgram);
    
    void debugTextureCoordinates(const std::vector<TexCoord>& texCoords);

public:
    void printCurrentDirectory();
    
    // 載入模型（增強版，支援 MTL 材質）
    bool loadModel(const std::string& modelName, const std::string& filepath) {
        if (loader.loadOBJ(filepath)) {
            Model model;
            model.name = modelName;
            
            // 檢查是否有 MTL 材質
            auto materialNames = loader.getMaterialNames();
            
            if (!materialNames.empty()) {
                // 有材質的情況 - 按材質分組載入
                std::cout << "載入多材質模型: " << modelName << std::endl;
                
                // 先從 MTL 載入所有材質
                const MTLLoader& mtlLoader = loader.getMTLLoader();
                std::string basePath = mtlLoader.getBasePath();
                
                for (const std::string& matName : materialNames) {
                    const MTLMaterial* mtlMat = loader.getMaterial(matName);
                    if (mtlMat) {
                        createMaterialFromMTL(*mtlMat, basePath);
                    }
                }
                
                // 按材質創建子模型
                auto facesByMaterial = loader.getFacesByMaterial();
                for (const auto& pair : facesByMaterial) {
                    const std::string& materialName = pair.first;
                    const std::vector<Face>& faces = pair.second;
                    
                    if (!faces.empty()) {
                        SubModel subModel;
                        subModel.materialName = materialName;
                        createSubModel(faces, materialName, subModel, loader);
                        model.subModels.push_back(subModel);
                        
                        std::cout << "創建子模型，材質: " << materialName
                                  << "，面數: " << faces.size() << std::endl;
                    }
                }
            } else {
                // 沒有材質的情況 - 使用原有方式
                std::cout << "載入單一材質模型: " << modelName << std::endl;
                
                std::vector<float> vertexData = loader.getVertexData();
                int vertexCount = loader.getVertexCount();
                
                model.vertexCount = vertexCount;
                
                // 建立VAO和VBO
                glGenVertexArrays(1, &model.VAO);
                glGenBuffers(1, &model.VBO);
                
                glBindVertexArray(model.VAO);
                glBindBuffer(GL_ARRAY_BUFFER, model.VBO);
                glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float),
                             vertexData.data(), GL_STATIC_DRAW);
                
                // 設定頂點屬性
                // location=0: aPos (vec3)
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0);
                
                // location=1: aColor (vec3) - 跳過，因為OBJ通常沒有頂點顏色
                // 我們在shader中使用固定顏色或材質顏色
                
                // location=2: aNormal (vec3)
                glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
                glEnableVertexAttribArray(2);
                
                // location=3: aTex (vec2)
                glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
                glEnableVertexAttribArray(3);
                
                glBindVertexArray(0);
            }
            
            models[modelName] = model;
            return true;
        }
        return false;
    }
    
    // 創建新材質
    bool createMaterial(const std::string& materialName) {
        if (materials.find(materialName) != materials.end()) {
            return false; // 材質已存在
        }
        
        Material material;
        material.name = materialName;
        // 創建默認白色紋理
        material.diffuseTexture = createDefaultTexture(glm::vec3(1.0f, 1.0f, 1.0f));
        
        materials[materialName] = material;
        return true;
    }
    
    // 載入漫反射紋理
    bool loadDiffuseTexture(const std::string& materialName, const std::string& texturePath) {
        auto it = materials.find(materialName);
        if (it == materials.end()) {
            return false;
        }
        
        std::cout << "loadDiffuseTexture: " << texturePath << std::endl;
        
        GLuint texture = loadTextureFromFile(texturePath);
        if (texture != 0) {
            // 清理舊紋理
            if (it->second.diffuseTexture != 0) {
                glDeleteTextures(1, &it->second.diffuseTexture);
            }
            it->second.diffuseTexture = texture;
            return true;
        }
        return false;
    }
    
    // 載入法線貼圖
    bool loadNormalTexture(const std::string& materialName, const std::string& texturePath) {
        auto it = materials.find(materialName);
        if (it == materials.end()) {
            return false;
        }
        std::cout << "loadNormalTexture: " << texturePath << std::endl;
        
        GLuint texture = loadTextureFromFile(texturePath);
        if (texture != 0) {
            if (it->second.normalTexture != 0) {
                glDeleteTextures(1, &it->second.normalTexture);
            }
            it->second.normalTexture = texture;
            return true;
        }
        return false;
    }
    
    // 載入鏡面反射紋理
    bool loadSpecularTexture(const std::string& materialName, const std::string& texturePath) {
        auto it = materials.find(materialName);
        if (it == materials.end()) {
            return false;
        }
        
        std::cout << "loadSpecularTexture: " << texturePath << std::endl;
        
        GLuint texture = loadTextureFromFile(texturePath);
        if (texture != 0) {
            if (it->second.specularTexture != 0) {
                glDeleteTextures(1, &it->second.specularTexture);
            }
            it->second.specularTexture = texture;
            return true;
        }
        return false;
    }
    
    // 設定材質屬性
    void setMaterialAmbient(const std::string& materialName, float r, float g, float b) {
        auto it = materials.find(materialName);
        if (it != materials.end()) {
            it->second.ambient = glm::vec3(r, g, b);
        }
    }
    
    void setMaterialDiffuse(const std::string& materialName, float r, float g, float b) {
        auto it = materials.find(materialName);
        if (it != materials.end()) {
            it->second.diffuse = glm::vec3(r, g, b);
        }
    }
    
    void setMaterialSpecular(const std::string& materialName, float r, float g, float b) {
        auto it = materials.find(materialName);
        if (it != materials.end()) {
            it->second.specular = glm::vec3(r, g, b);
        }
    }
    
    void setMaterialShininess(const std::string& materialName, float shininess) {
        auto it = materials.find(materialName);
        if (it != materials.end()) {
            it->second.shininess = shininess;
        }
    }
    
    void setMaterialAlpha(const std::string& materialName, float alpha) {
        auto it = materials.find(materialName);
        if (it != materials.end()) {
            it->second.alpha = alpha;
        }
    }
    
    // 將材質指派給模型
    void assignMaterialToModel(const std::string& modelName, const std::string& materialName) {
        auto modelIt = models.find(modelName);
        auto materialIt = materials.find(materialName);
        
        if (modelIt != models.end() && materialIt != materials.end()) {
            modelIt->second.materialName = materialName;
        }
    }
    
    // 應用材質到shader（需要在渲染時調用）
    void applyMaterial(const std::string& materialName, GLuint shaderProgram) {
        auto it = materials.find(materialName);
        if (it == materials.end()) {
            return;
        }
        
        const Material& material = it->second;
        
        // 設定材質屬性uniform
        GLint ambientLoc = glGetUniformLocation(shaderProgram, "material.ambient");
        GLint diffuseLoc = glGetUniformLocation(shaderProgram, "material.diffuse");
        GLint specularLoc = glGetUniformLocation(shaderProgram, "material.specular");
        GLint shininessLoc = glGetUniformLocation(shaderProgram, "material.shininess");
        GLint alphaLoc = glGetUniformLocation(shaderProgram, "material.alpha");
        
        if (ambientLoc != -1) glUniform3fv(ambientLoc, 1, glm::value_ptr(material.ambient));
        if (diffuseLoc != -1) glUniform3fv(diffuseLoc, 1, glm::value_ptr(material.diffuse));
        if (specularLoc != -1) glUniform3fv(specularLoc, 1, glm::value_ptr(material.specular));
        if (shininessLoc != -1) glUniform1f(shininessLoc, material.shininess);
        if (alphaLoc != -1) glUniform1f(alphaLoc, material.alpha);
        
        // 綁定紋理
        if (material.diffuseTexture != 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, material.diffuseTexture);
            GLint diffuseTexLoc = glGetUniformLocation(shaderProgram, "material.diffuseTexture");
            if (diffuseTexLoc != -1) glUniform1i(diffuseTexLoc, 0);
        }
        
        if (material.normalTexture != 0) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, material.normalTexture);
            GLint normalTexLoc = glGetUniformLocation(shaderProgram, "material.normalTexture");
            if (normalTexLoc != -1) glUniform1i(normalTexLoc, 1);
        }
        
        if (material.specularTexture != 0) {
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, material.specularTexture);
            GLint specularTexLoc = glGetUniformLocation(shaderProgram, "material.specularTexture");
            if (specularTexLoc != -1) glUniform1i(specularTexLoc, 2);
        }
    }
    
    // 增強的渲染函數（支援多材質模型）
    void renderModelWithMaterial(const std::string& modelName, GLuint shaderProgram, GLint modelMatrixLocation) {
        auto it = models.find(modelName);
        if (it == models.end()) {
            return;
        }
        
        const Model& model = it->second;
        
        // 應用模型矩陣
        glm::mat4 modelMatrix = model.getModelMatrix();
        glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        
        if (model.hasMultipleMaterials()) {
            // 渲染多材質模型
            for (const auto& subModel : model.subModels) {
                // 應用子模型的材質
                if (!subModel.materialName.empty()) {
                    applyMaterial(subModel.materialName, shaderProgram);
                }
                
                // 渲染子模型
                glBindVertexArray(subModel.VAO);
                glDrawArrays(GL_TRIANGLES, 0, subModel.vertexCount);
                glBindVertexArray(0);
            }
        } else {
            // 渲染單一材質模型
            if (!model.materialName.empty()) {
                applyMaterial(model.materialName, shaderProgram);
            }
            
            glBindVertexArray(model.VAO);
            glDrawArrays(GL_TRIANGLES, 0, model.vertexCount);
            glBindVertexArray(0);
        }
    }
    
    void setModelTransform(const std::string& modelName, const Transform& transform) {
        auto it = models.find(modelName);
        if (it != models.end()) {
            it->second.transform = transform;
        }
    }
    
    // 設定模型大小（統一縮放）
    void setModelScale(const std::string& modelName, float scale) {
        auto it = models.find(modelName);
        if (it != models.end()) {
            it->second.transform.setScale(scale);
        }
    }
    
    // 設定模型大小（各軸不同）
    void setModelScale(const std::string& modelName, float x, float y, float z) {
        auto it = models.find(modelName);
        if (it != models.end()) {
            it->second.transform.setScale(x, y, z);
        }
    }
    
    // 設定模型位置
    void setModelPosition(const std::string& modelName, float x, float y, float z) {
        auto it = models.find(modelName);
        if (it != models.end()) {
            it->second.transform.setPosition(x, y, z);
        }
    }
    
    // 設定模型旋轉
    void setModelRotation(const std::string& modelName, float x, float y, float z) {
        auto it = models.find(modelName);
        if (it != models.end()) {
            it->second.transform.setRotation(x, y, z);
        }
    }
    
    // 繪製指定模型（向後兼容）
    void renderModel(const std::string& modelName, GLuint shaderProgram, GLint modelMatrixLocation) {
        renderModelWithMaterial(modelName, shaderProgram, modelMatrixLocation);
    }
    
    // 取得模型的變換資訊
    Transform* getModelTransform(const std::string& modelName) {
        auto it = models.find(modelName);
        if (it != models.end()) {
            return &it->second.transform;
        }
        return nullptr;
    }
    
    // 取得模型資訊
    Model* getModel(const std::string& modelName) {
        auto it = models.find(modelName);
        if (it != models.end()) {
            return &it->second;
        }
        return nullptr;
    }
    
    // 取得材質資訊
    Material* getMaterial(const std::string& materialName) {
        auto it = materials.find(materialName);
        if (it != materials.end()) {
            return &it->second;
        }
        return nullptr;
    }
    
    // 檢查模型是否存在
    bool hasModel(const std::string& modelName) {
        return models.find(modelName) != models.end();
    }
    
    // 檢查材質是否存在
    bool hasMaterial(const std::string& materialName) {
        return materials.find(materialName) != materials.end();
    }
    
    // 移除模型
    void removeModel(const std::string& modelName) {
        auto it = models.find(modelName);
        if (it != models.end()) {
            it->second.cleanup();
            models.erase(it);
        }
    }
    
    void removeMaterial(const std::string& materialName) {
        auto it = materials.find(materialName);
        if (it != materials.end()) {
            it->second.cleanup();
            materials.erase(it);
        }
    }
    
    void cleanup() {
        for (auto& pair : models) {
            pair.second.cleanup();
        }
        models.clear();
        
        for (auto& pair : materials) {
            pair.second.cleanup();
        }
        materials.clear();
    }
    
    // 取得名稱列表
    std::vector<std::string> getModelNames() {
        std::vector<std::string> names;
        for (const auto& pair : models) {
            names.push_back(pair.first);
        }
        return names;
    }
    
    std::vector<std::string> getMaterialNames() {
        std::vector<std::string> names;
        for (const auto& pair : materials) {
            names.push_back(pair.first);
        }
        return names;
    }
    
    // 列印模型和材質資訊
    void printModelInfo(const std::string& modelName) {
        auto it = models.find(modelName);
        if (it != models.end()) {
            const Model& model = it->second;
            std::cout << "模型: " << model.name << std::endl;
            
            if (model.hasMultipleMaterials()) {
                std::cout << "  多材質模型，子模型數: " << model.subModels.size() << std::endl;
                for (size_t i = 0; i < model.subModels.size(); ++i) {
                    const SubModel& subModel = model.subModels[i];
                    std::cout << "    子模型 " << i << ": 材質=" << subModel.materialName
                              << ", 頂點數=" << subModel.vertexCount << std::endl;
                }
            } else {
                std::cout << "  單一材質模型，頂點數: " << model.vertexCount << std::endl;
                std::cout << "  材質: " << (model.materialName.empty() ? "無" : model.materialName) << std::endl;
            }
        }
    }
};

// 全域模型管理器
extern ModelManager g_modelManager;

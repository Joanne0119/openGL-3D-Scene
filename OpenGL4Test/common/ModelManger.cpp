// ModelManager.cpp
#include "ModelManager.h"
#include <iostream>
//
//#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"

bool fileExists(const std::string& filepath) {
    std::ifstream file(filepath);
    return file.good();
}

void ModelManager::printCurrentDirectory(){
    std::cout << "\n=== Current Working Directory ===" << std::endl;
    std::cout << "Current path: " << std::filesystem::current_path() << std::endl;

    std::cout << "\n=== Testing Roughness File Specifically ===" << std::endl;
    std::string roughnessFile = "Texture/elephant_toy_roughness.jpg";

    // Test 1: filesystem::exists
    if (std::filesystem::exists(roughnessFile)) {
        std::cout << "✅ filesystem::exists = TRUE" << std::endl;
        std::cout << "File size: " << std::filesystem::file_size(roughnessFile) << " bytes" << std::endl;
    } else {
        std::cout << "❌ filesystem::exists = FALSE" << std::endl;
    }

    // Test 2: ifstream (your fileExists function)
    std::ifstream testFile(roughnessFile);
    if (testFile.good()) {
        std::cout << "✅ ifstream access = TRUE" << std::endl;
    } else {
        std::cout << "❌ ifstream access = FALSE" << std::endl;
    }
    testFile.close();

    // Test 3: Try loading with ModelManager
    std::cout << "\n=== Testing with ModelManager ===" << std::endl;
    g_modelManager.createMaterial("testMaterial");
    bool result = g_modelManager.loadNormalTexture("testMaterial", roughnessFile);
    std::cout << "ModelManager result: " << (result ? "SUCCESS" : "FAILED") << std::endl;
}

GLuint ModelManager::loadTextureFromFile(const std::string& filepath) {
    // 1. 先檢查檔案是否存在
    if (!fileExists(filepath)) {
        std::cerr << "Texture file does not exist: " << filepath << std::endl;
        std::cerr << "Current working directory: " << std::filesystem::current_path() << std::endl;
        
        // 嘗試常見的路徑變化
        std::vector<std::string> possiblePaths = {
            filepath,
            "../" + filepath,
            "./" + filepath,
            "assets/" + filepath,
            "resources/" + filepath
        };
        
        std::cout << "Trying alternative paths:" << std::endl;
        for (const auto& path : possiblePaths) {
            std::cout << "  Checking: " << path;
            if (fileExists(path)) {
                std::cout << " [FOUND]" << std::endl;
                return loadTextureFromFile(path); // 遞歸呼叫找到的路徑
            } else {
                std::cout << " [NOT FOUND]" << std::endl;
            }
        }
        
        return createDefaultTexture(); // 返回默認紋理
    }
    
    GLuint textureID = 0;
    
    // 使用 stb_image 載入圖片
    int width, height, channels;
    
    // stb_image 預設 Y 軸是翻轉的，設定為不翻轉（與 OpenGL 坐標系統一致）
    stbi_set_flip_vertically_on_load(true);
    
    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);
    
    if (!data) {
        std::cerr << "stb_image failed to load texture: " << filepath << std::endl;
        std::cerr << "stb_image error: " << stbi_failure_reason() << std::endl;
        return createDefaultTexture();
    }
    
    // 生成 OpenGL 紋理
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // 根據通道數決定格式
    GLenum format;
    GLenum internalFormat;
    
    switch (channels) {
        case 1:
            format = GL_RED;
            internalFormat = GL_RED;
            break;
        case 2:
            format = GL_RG;
            internalFormat = GL_RG;
            break;
        case 3:
            format = GL_RGB;
            internalFormat = GL_RGB;
            break;
        case 4:
            format = GL_RGBA;
            internalFormat = GL_RGBA;
            break;
        default:
            std::cerr << "Unsupported number of channels: " << channels << std::endl;
            stbi_image_free(data);
            glDeleteTextures(1, &textureID);
            return createDefaultTexture();
    }
    
    // 上傳紋理數據到 GPU
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    
    // 生成 mipmap
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // 設定紋理參數
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // 設定各向異性過濾（如果支援的話）
    if (GLEW_EXT_texture_filter_anisotropic) {
        GLfloat maxAnisotropy;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // 釋放 stb_image 的記憶體
    stbi_image_free(data);
    
    std::cout << "Successfully loaded texture: " << filepath
              << " (ID: " << textureID
              << ", Size: " << width << "x" << height
              << ", Channels: " << channels << ")" << std::endl;
    
    return textureID;
}

GLuint ModelManager::createDefaultTexture(glm::vec3 color) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // 創建 1x1 像素的純色紋理
    unsigned char pixels[3] = {
        static_cast<unsigned char>(color.r * 255),
        static_cast<unsigned char>(color.g * 255),
        static_cast<unsigned char>(color.b * 255)
    };
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    
    // 設定紋理參數
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    return textureID;
}

void ModelManager::createMaterialFromMTL(const MTLMaterial& mtlMaterial, const std::string& basePath) {
    // 檢查材質是否已存在
    if (materials.find(mtlMaterial.name) != materials.end()) {
        std::cout << "材質 " << mtlMaterial.name << " 已存在，跳過" << std::endl;
        return;
    }
    
    Material material;
    material.name = mtlMaterial.name;
    
    // 設定材質顏色屬性
    material.ambient = mtlMaterial.Ka;
    material.diffuse = mtlMaterial.Kd;
    material.specular = mtlMaterial.Ks;
    material.shininess = mtlMaterial.Ns;
    material.alpha = mtlMaterial.d;
    
    std::cout << "創建材質: " << material.name << std::endl;
    std::cout << "  環境光: (" << material.ambient.x << ", " << material.ambient.y << ", " << material.ambient.z << ")" << std::endl;
    std::cout << "  漫反射: (" << material.diffuse.x << ", " << material.diffuse.y << ", " << material.diffuse.z << ")" << std::endl;
    std::cout << "  鏡面反射: (" << material.specular.x << ", " << material.specular.y << ", " << material.specular.z << ")" << std::endl;
    std::cout << "  光澤度: " << material.shininess << std::endl;
    std::cout << "  透明度: " << material.alpha << std::endl;
    
    // 載入紋理（如果有的話）
    
    // 1. 漫反射紋理 (map_Kd)
    if (!mtlMaterial.map_Kd.empty()) {
        std::string texturePath = basePath + mtlMaterial.map_Kd;
        std::cout << "  嘗試載入漫反射紋理: " << texturePath << std::endl;
        
        GLuint diffuseTexture = loadTextureFromFile(texturePath);
        if (diffuseTexture != 0) {
            material.diffuseTexture = diffuseTexture;
            std::cout << "  ✅ 成功載入漫反射紋理: " << texturePath << std::endl;
        } else {
            std::cout << "  ❌ 載入漫反射紋理失敗: " << texturePath << std::endl;
            material.diffuseTexture = createDefaultTexture(material.diffuse);
        }
    } else {
        // 如果沒有紋理，創建基於材質顏色的默認紋理
        material.diffuseTexture = createDefaultTexture(material.diffuse);
    }
    
    // 2. 法線貼圖 (map_bump)
    if (!mtlMaterial.map_bump.empty()) {
        std::string normalPath = basePath + mtlMaterial.map_bump;
        std::cout << "  嘗試載入法線貼圖: " << normalPath << std::endl;
        
        GLuint normalTexture = loadTextureFromFile(normalPath);
        if (normalTexture != 0) {
            material.normalTexture = normalTexture;
            std::cout << "  ✅ 成功載入法線貼圖: " << normalPath << std::endl;
        } else {
            std::cout << "  ❌ 載入法線貼圖失敗: " << normalPath << std::endl;
        }
    }
    
    // 3. 鏡面反射紋理 (map_Ks)
    if (!mtlMaterial.map_Ks.empty()) {
        std::string specularPath = basePath + mtlMaterial.map_Ks;
        std::cout << "  嘗試載入鏡面反射紋理: " << specularPath << std::endl;
        
        GLuint specularTexture = loadTextureFromFile(specularPath);
        if (specularTexture != 0) {
            material.specularTexture = specularTexture;
            std::cout << "  ✅ 成功載入鏡面反射紋理: " << specularPath << std::endl;
        } else {
            std::cout << "  ❌ 載入鏡面反射紋理失敗: " << specularPath << std::endl;
        }
    }
    
    // 4. 其他可能的紋理映射
    // 環境光紋理 (map_Ka) - 通常不常用，可選實作
    if (!mtlMaterial.map_Ka.empty()) {
        std::string ambientPath = basePath + mtlMaterial.map_Ka;
        std::cout << "  發現環境光紋理但跳過: " << ambientPath << std::endl;
        // 可以根據需要實作環境光紋理載入
    }
    
    // 光澤度紋理 (map_Ns) - 可選實作
    if (!mtlMaterial.map_Ns.empty()) {
        std::string shininessPath = basePath + mtlMaterial.map_Ns;
        std::cout << "  發現光澤度紋理但跳過: " << shininessPath << std::endl;
        // 可以根據需要實作光澤度紋理載入
    }
    
    // 透明度紋理 (map_d) - 可選實作
    if (!mtlMaterial.map_d.empty()) {
        std::string alphaPath = basePath + mtlMaterial.map_d;
        std::cout << "  發現透明度紋理但跳過: " << alphaPath << std::endl;
        // 可以根據需要實作透明度紋理載入
    }
    
    // 將材質添加到管理器
    materials[material.name] = material;
    std::cout << "  ✅ 材質 " << material.name << " 創建完成" << std::endl;
}

void ModelManager::createSubModel(const std::vector<Face>& faces,
                                  const std::string& materialName,
                                  SubModel& subModel,
                                  const OBJLoader& objLoader) {
    if (faces.empty()) {
        std::cerr << "錯誤: 嘗試創建空的子模型" << std::endl;
        return;
    }
    
    std::vector<float> vertexData;
    
    // 取得 OBJ 載入器的資料
    const std::vector<Vertex>& vertices = objLoader.getVertices();
    const std::vector<Normal>& normals = objLoader.getNormals();
    const std::vector<TexCoord>& texCoords = objLoader.getTexCoords();
    
    // 處理每個面
    for (const auto& face : faces) {
        // 每個面應該是三角形（3個頂點）
        // 按照 [position, normal, texcoord] 的順序添加
        for (int i = 0; i < 3; ++i) {
            // 取得索引（注意：OBJ 檔案索引從1開始，所以要減1）
            int vertexIndex = face.v[i] - 1;
            int normalIndex = face.vn[i] - 1;
            int texCoordIndex = face.vt[i] - 1;
            
            // 檢查索引是否有效
            if (vertexIndex >= 0 && vertexIndex < vertices.size()) {
                const Vertex& vertex = vertices[vertexIndex];
                // 位置 (x, y, z)
                vertexData.push_back(vertex.x);
                vertexData.push_back(vertex.y);
                vertexData.push_back(vertex.z);
            } else {
                // 如果索引無效，使用預設值
                vertexData.push_back(0.0f);
                vertexData.push_back(0.0f);
                vertexData.push_back(0.0f);
            }
            
            // 法線 (nx, ny, nz)
            if (normalIndex >= 0 && normalIndex < normals.size()) {
                const Normal& normal = normals[normalIndex];
                vertexData.push_back(normal.x);
                vertexData.push_back(normal.y);
                vertexData.push_back(normal.z);
            } else {
                // 如果沒有法線，使用預設法線 (0, 0, 1)
                vertexData.push_back(0.0f);
                vertexData.push_back(0.0f);
                vertexData.push_back(1.0f);
            }
            
            // 紋理座標 (u, v)
            if (texCoordIndex >= 0 && texCoordIndex < texCoords.size()) {
                const TexCoord& texCoord = texCoords[texCoordIndex];
                vertexData.push_back(texCoord.u);
                vertexData.push_back(texCoord.v);
                
                std::cout << "紋理座標: (" << texCoord.u << ", " << texCoord.v << ")" << std::endl;
            } else {
                vertexData.push_back(0.0f);
                vertexData.push_back(0.0f);
                std::cout << "使用默認紋理座標: (0.0, 0.0)" << std::endl;
            }
        }
    }
    
    subModel.vertexCount = faces.size() * 3; // 每個面3個頂點
    subModel.materialName = materialName;
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(3);
    
    // 創建OpenGL緩衝區
    glGenVertexArrays(1, &subModel.VAO);
    glGenBuffers(1, &subModel.VBO);
    
    glBindVertexArray(subModel.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, subModel.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float),
                 vertexData.data(), GL_STATIC_DRAW);
    
    // 設定頂點屬性
    // location=0: aPos (vec3)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // location=1: aColor - 跳過
    
    // location=2: aNormal (vec3)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    // location=3: aTex (vec2)
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(3);
    
    glBindVertexArray(0);
    
    std::cout << "子模型創建完成 - 材質: " << materialName
              << ", 頂點數: " << subModel.vertexCount
              << ", 面數: " << faces.size() << std::endl;
}

void ModelManager::bindMaterial(const std::string& materialName, GLuint shaderProgram) {
    auto it = materials.find(materialName);
    if (it == materials.end()) {
        std::cerr << "警告: 材質不存在: " << materialName << std::endl;
        return;
    }
    
    const Material& mat = it->second;
    
    // 设置材质属性
    glUniform4fv(glGetUniformLocation(shaderProgram, "uMaterial.ambient"), 1,
                 glm::value_ptr(glm::vec4(mat.ambient, 1.0f)));
    glUniform4fv(glGetUniformLocation(shaderProgram, "uMaterial.diffuse"), 1,
                 glm::value_ptr(glm::vec4(mat.diffuse, 1.0f)));
    glUniform4fv(glGetUniformLocation(shaderProgram, "uMaterial.specular"), 1,
                 glm::value_ptr(glm::vec4(mat.specular, 1.0f)));
    glUniform1f(glGetUniformLocation(shaderProgram, "uMaterial.shininess"), mat.shininess);
    
    // 绑定并设置纹理
    bool hasDiffuseTexture = (mat.diffuseTexture != 0);
    bool hasNormalTexture = (mat.normalTexture != 0);
    bool hasSpecularTexture = (mat.specularTexture != 0);
    
    // 设置纹理存在标志
    glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasDiffuseTexture"), hasDiffuseTexture);
    glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasNormalTexture"), hasNormalTexture);
    glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasSpecularTexture"), hasSpecularTexture);
    
    // 绑定漫反射纹理
    if (hasDiffuseTexture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mat.diffuseTexture);
        glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.diffuseTexture"), 0);
        
        std::cout << "绑定漫反射紋理 ID: " << mat.diffuseTexture << std::endl;
    }
    
    // 绑定法线纹理
    if (hasNormalTexture) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mat.normalTexture);
        glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.normalTexture"), 1);
        
        std::cout << "绑定法線紋理 ID: " << mat.normalTexture << std::endl;
    }
    
    // 绑定镜面纹理
    if (hasSpecularTexture) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, mat.specularTexture);
        glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.specularTexture"), 2);
        
        std::cout << "绑定镜面紋理 ID: " << mat.specularTexture << std::endl;
    }
    
    // 检查 OpenGL 错误
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL 錯誤在材質绑定後: " << error << std::endl;
    }
}

void ModelManager::debugTextureCoordinates(const std::vector<TexCoord>& texCoords) {
    if (texCoords.empty()) {
        std::cout << "❌ 沒有紋理座標數據" << std::endl;
        return;
    }
    
    float minU = texCoords[0].u, maxU = texCoords[0].u;
    float minV = texCoords[0].v, maxV = texCoords[0].v;
    
    for (const auto& tc : texCoords) {
        minU = std::min(minU, tc.u);
        maxU = std::max(maxU, tc.u);
        minV = std::min(minV, tc.v);
        maxV = std::max(maxV, tc.v);
    }
    
    std::cout << "紋理座標範圍: U[" << minU << " - " << maxU << "], V[" << minV << " - " << maxV << "]" << std::endl;
    std::cout << "紋理座標總數: " << texCoords.size() << std::endl;
}

// 全域模型管理器實例
ModelManager g_modelManager;

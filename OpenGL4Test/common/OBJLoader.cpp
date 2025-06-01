// OBJLoader.cpp 
#include "OBJLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>

// 原有的結構體實作保持不變
Vertex::Vertex() : x(0), y(0), z(0) {}
Vertex::Vertex(float x, float y, float z) : x(x), y(y), z(z) {}

Normal::Normal() : x(0), y(0), z(0) {}
Normal::Normal(float x, float y, float z) : x(x), y(y), z(z) {}

TexCoord::TexCoord() : u(0), v(0) {}
TexCoord::TexCoord(float u, float v) : u(u), v(v) {}

Face::Face() {
    for(int i = 0; i < 3; i++) {
        v[i] = vt[i] = vn[i] = -1;
    }
}

std::string OBJLoader::getDirectoryFromPath(const std::string& filepath) {
    size_t lastSlash = filepath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        return filepath.substr(0, lastSlash + 1);
    }
    return "./";
}

void OBJLoader::parseFace(const std::string& faceData, Face& face) {
    std::istringstream iss(faceData);
    std::string vertex;
    int index = 0;
    
    while (iss >> vertex && index < 3) {
        std::istringstream vertexStream(vertex);
        std::string component;
        int componentIndex = 0;
        
        while (std::getline(vertexStream, component, '/') && componentIndex < 3) {
            if (!component.empty()) {
                int value = std::stoi(component) - 1; // OBJ索引從1開始，轉換為0開始
                
                if (componentIndex == 0) {
                    face.v[index] = value;
                } else if (componentIndex == 1) {
                    face.vt[index] = value;
                } else if (componentIndex == 2) {
                    face.vn[index] = value;
                }
            }
            componentIndex++;
        }
        index++;
    }
    
    // 設定面的材質
    face.materialName = currentMaterial;
}

bool OBJLoader::loadOBJ(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "無法開啟檔案: " << filename << std::endl;
        return false;
    }
    
    // 設定基礎路徑
    basePath = getDirectoryFromPath(filename);
    
    // 清空之前的資料
    vertices.clear();
    normals.clear();
    texCoords.clear();
    faces.clear();
    currentMaterial.clear();
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue; // 跳過空行和註解
        }
        
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        
        if (prefix == "v") {
            // 頂點座標
            float x, y, z;
            iss >> x >> y >> z;
            vertices.emplace_back(x, y, z);
            
        } else if (prefix == "vn") {
            // 法向量
            float x, y, z;
            iss >> x >> y >> z;
            normals.emplace_back(x, y, z);
            
        } else if (prefix == "vt") {
            // 紋理座標
            float u, v;
            iss >> u >> v;
            texCoords.emplace_back(u, v);
            
        } else if (prefix == "f") {
            // 面資料
            Face face;
            std::string remaining;
            std::getline(iss, remaining);
            parseFace(remaining, face);
            faces.push_back(face);
            
        } else if (prefix == "mtllib") {
            // MTL 檔案參考
            std::string mtlFile;
            iss >> mtlFile;
            std::string mtlPath = basePath + mtlFile;
            
            std::cout << "載入 MTL 檔案: " << mtlPath << std::endl;
            if (!mtlLoader.loadMTL(mtlPath)) {
                std::cerr << "警告: 無法載入 MTL 檔案: " << mtlPath << std::endl;
            }
            
        } else if (prefix == "usemtl") {
            // 使用材質
            std::string materialName;
            iss >> materialName;
            currentMaterial = materialName;
            std::cout << "切換到材質: " << materialName << std::endl;
        }
    }
    
    file.close();
    
    std::cout << "OBJ 載入完成: " << std::endl;
    std::cout << "頂點數: " << vertices.size() << std::endl;
    std::cout << "法向量數: " << normals.size() << std::endl;
    std::cout << "紋理座標數: " << texCoords.size() << std::endl;
    std::cout << "面數: " << faces.size() << std::endl;
    std::cout << "材質數: " << mtlLoader.getMaterialCount() << std::endl;
    
    return true;
}

std::vector<float> OBJLoader::getVertexData() {
    std::vector<float> data;
    
    for (const auto& face : faces) {
        for (int i = 0; i < 3; i++) {
            // 頂點座標
            if (face.v[i] >= 0 && face.v[i] < vertices.size()) {
                data.push_back(vertices[face.v[i]].x);
                data.push_back(vertices[face.v[i]].y);
                data.push_back(vertices[face.v[i]].z);
            }
            
            // 法向量
            if (face.vn[i] >= 0 && face.vn[i] < normals.size()) {
                data.push_back(normals[face.vn[i]].x);
                data.push_back(normals[face.vn[i]].y);
                data.push_back(normals[face.vn[i]].z);
            } else {
                data.push_back(0.0f);
                data.push_back(1.0f);
                data.push_back(0.0f);
            }
            
            // 紋理座標
            if (face.vt[i] >= 0 && face.vt[i] < texCoords.size()) {
                data.push_back(texCoords[face.vt[i]].u);
                data.push_back(texCoords[face.vt[i]].v);
            } else {
                data.push_back(0.0f);
                data.push_back(0.0f);
            }
        }
    }
    
    return data;
}

int OBJLoader::getVertexCount() {
    return faces.size() * 3;
}

const std::vector<Vertex>& OBJLoader::getVertices() const {
    return vertices;
}

const std::vector<Normal>& OBJLoader::getNormals() const {
    return normals;
}

const std::vector<TexCoord>& OBJLoader::getTexCoords() const {
    return texCoords;
}

const std::vector<Face>& OBJLoader::getFaces() const {
    return faces;
}

const MTLLoader& OBJLoader::getMTLLoader() const {
    return mtlLoader;
}

std::vector<std::string> OBJLoader::getMaterialNames() const {
    return mtlLoader.getMaterialNames();
}

const MTLMaterial* OBJLoader::getMaterial(const std::string& name) const {
    return mtlLoader.getMaterial(name);
}

std::map<std::string, std::vector<Face>> OBJLoader::getFacesByMaterial() const {
    std::map<std::string, std::vector<Face>> result;
    
    for (const auto& face : faces) {
        std::string materialName = face.materialName;
        if (materialName.empty()) {
            materialName = "default"; // 沒有材質的面使用默認材質
        }
        result[materialName].push_back(face);
    }
    
    return result;
}

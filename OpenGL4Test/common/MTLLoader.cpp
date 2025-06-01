// MTLLoader.cpp
#include "MTLLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

MTLLoader::MTLLoader() {}

std::string MTLLoader::getDirectoryFromPath(const std::string& filepath) {
    size_t lastSlash = filepath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        return filepath.substr(0, lastSlash + 1);
    }
    return "./";
}

glm::vec3 MTLLoader::parseVec3(const std::string& line) {
    std::istringstream iss(line);
    std::string token;
    iss >> token; // 跳過命令前綴
    
    float x = 0.0f, y = 0.0f, z = 0.0f;
    iss >> x >> y >> z;
    return glm::vec3(x, y, z);
}

float MTLLoader::parseFloat(const std::string& line) {
    std::istringstream iss(line);
    std::string token;
    iss >> token; // 跳過命令前綴
    
    float value = 0.0f;
    iss >> value;
    return value;
}

std::string MTLLoader::parseString(const std::string& line) {
    std::istringstream iss(line);
    std::string token;
    iss >> token; // 跳過命令前綴
    
    std::string value;
    iss >> value;
    return value;
}

bool MTLLoader::loadMTL(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "無法開啟 MTL 檔案: " << filepath << std::endl;
        return false;
    }
    
    // 設定基礎路徑
    basePath = getDirectoryFromPath(filepath);
    
    // 清空之前的資料
    materials.clear();
    
    std::string line;
    MTLMaterial* currentMaterial = nullptr;
    
    std::cout << "開始載入 MTL 檔案: " << filepath << std::endl;
    std::cout << "基礎路徑: " << basePath << std::endl;
    
    while (std::getline(file, line)) {
        // 移除行尾的空白字符
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        if (line.empty() || line[0] == '#') {
            continue; // 跳過空行和註解
        }
        
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        
        if (prefix == "newmtl") {
            // 新材質定義
            std::string materialName;
            iss >> materialName;
            
            materials[materialName] = MTLMaterial();
            materials[materialName].name = materialName;
            currentMaterial = &materials[materialName];
            
            std::cout << "發現材質: " << materialName << std::endl;
            
        } else if (currentMaterial != nullptr) {
            // 材質屬性
            if (prefix == "Ka") {
                currentMaterial->Ka = parseVec3(line);
            } else if (prefix == "Kd") {
                currentMaterial->Kd = parseVec3(line);
            } else if (prefix == "Ks") {
                currentMaterial->Ks = parseVec3(line);
            } else if (prefix == "Ns") {
                currentMaterial->Ns = parseFloat(line);
            } else if (prefix == "d") {
                currentMaterial->d = parseFloat(line);
            } else if (prefix == "map_Ka") {
                currentMaterial->map_Ka = parseString(line);
            } else if (prefix == "map_Kd") {
                currentMaterial->map_Kd = parseString(line);
            } else if (prefix == "map_Ks") {
                currentMaterial->map_Ks = parseString(line);
            } else if (prefix == "map_Ns") {
                currentMaterial->map_Ns = parseString(line);
            } else if (prefix == "map_d") {
                currentMaterial->map_d = parseString(line);
            } else if (prefix == "map_bump" || prefix == "bump") {
                currentMaterial->map_bump = parseString(line);
            }
        }
    }
    
    file.close();
    
    std::cout << "MTL 載入完成，共載入 " << materials.size() << " 個材質" << std::endl;
    
    // 顯示載入的材質資訊
    for (const auto& pair : materials) {
        const MTLMaterial& mat = pair.second;
        std::cout << "材質: " << mat.name << std::endl;
        std::cout << "  Ka: (" << mat.Ka.x << ", " << mat.Ka.y << ", " << mat.Ka.z << ")" << std::endl;
        std::cout << "  Kd: (" << mat.Kd.x << ", " << mat.Kd.y << ", " << mat.Kd.z << ")" << std::endl;
        std::cout << "  Ks: (" << mat.Ks.x << ", " << mat.Ks.y << ", " << mat.Ks.z << ")" << std::endl;
        std::cout << "  Ns: " << mat.Ns << std::endl;
        if (!mat.map_Kd.empty()) {
            std::cout << "  漫反射貼圖: " << mat.map_Kd << std::endl;
        }
        if (!mat.map_bump.empty()) {
            std::cout << "  法線貼圖: " << mat.map_bump << std::endl;
        }
        std::cout << std::endl;
    }
    
    return true;
}

const MTLMaterial* MTLLoader::getMaterial(const std::string& name) const {
    auto it = materials.find(name);
    if (it != materials.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<std::string> MTLLoader::getMaterialNames() const {
    std::vector<std::string> names;
    for (const auto& pair : materials) {
        names.push_back(pair.first);
    }
    return names;
}

size_t MTLLoader::getMaterialCount() const {
    return materials.size();
}

void MTLLoader::clear() {
    materials.clear();
    basePath.clear();
}

const std::string& MTLLoader::getBasePath() const {
    return basePath;
}

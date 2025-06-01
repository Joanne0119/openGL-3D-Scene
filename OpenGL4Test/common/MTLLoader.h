// MTLLoader.h
#pragma once
#include <string>
#include <vector>
#include <map>
#include <glm/glm.hpp>

// MTL 材質結構
struct MTLMaterial {
    std::string name;
    
    // 顏色屬性
    glm::vec3 Ka;  // 環境光顏色 (ambient)
    glm::vec3 Kd;  // 漫反射顏色 (diffuse)
    glm::vec3 Ks;  // 鏡面反射顏色 (specular)
    float Ns;      // 光澤度 (shininess)
    float d;       // 透明度 (dissolve/alpha)
    
    // 紋理貼圖路徑
    std::string map_Ka;    // 環境光貼圖
    std::string map_Kd;    // 漫反射貼圖
    std::string map_Ks;    // 鏡面反射貼圖
    std::string map_Ns;    // 光澤度貼圖
    std::string map_d;     // 透明度貼圖
    std::string map_bump;  // 凹凸貼圖
    std::string bump;      // 法線貼圖
    
    // 預設值初始化
    MTLMaterial() :
        Ka(0.2f, 0.2f, 0.2f),
        Kd(0.8f, 0.8f, 0.8f),
        Ks(1.0f, 1.0f, 1.0f),
        Ns(32.0f),
        d(1.0f) {}
};

class MTLLoader {
private:
    std::map<std::string, MTLMaterial> materials;
    std::string basePath;  // MTL 檔案的基礎路徑，用於解析相對紋理路徑
    
    // 輔助函數
    glm::vec3 parseVec3(const std::string& line);
    float parseFloat(const std::string& line);
    std::string parseString(const std::string& line);
    std::string getDirectoryFromPath(const std::string& filepath);

public:
    MTLLoader();
    
    // 載入 MTL 檔案
    bool loadMTL(const std::string& filepath);
    
    // 取得材質
    const MTLMaterial* getMaterial(const std::string& name) const;
    
    // 取得所有材質名稱
    std::vector<std::string> getMaterialNames() const;
    
    // 取得材質數量
    size_t getMaterialCount() const;
    
    // 清空資料
    void clear();
    
    // 取得基礎路徑（用於紋理路徑解析）
    const std::string& getBasePath() const;
};

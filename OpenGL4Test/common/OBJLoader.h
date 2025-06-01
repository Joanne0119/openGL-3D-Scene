// OBJLoader.h
#pragma once
#include <vector>
#include <string>
#include "MTLLoader.h"

struct Vertex {
    float x, y, z;
    Vertex();
    Vertex(float x, float y, float z);
};

struct Normal {
    float x, y, z;
    Normal();
    Normal(float x, float y, float z);
};

struct TexCoord {
    float u, v;
    TexCoord();
    TexCoord(float u, float v);
};

struct Face {
    int v[3];   // 頂點索引
    int vt[3];  // 紋理座標索引
    int vn[3];  // 法向量索引
    std::string materialName; // 面所使用的材質名稱
    Face();
};

class OBJLoader {
private:
    std::vector<Vertex> vertices;
    std::vector<Normal> normals;
    std::vector<TexCoord> texCoords;
    std::vector<Face> faces;
    
    MTLLoader mtlLoader;
    std::string basePath;
    std::string currentMaterial; // 當前使用的材質
    
    void parseFace(const std::string& faceData, Face& face);
    std::string getDirectoryFromPath(const std::string& filepath);

public:
    bool loadOBJ(const std::string& filename);
    std::vector<float> getVertexData();
    int getVertexCount();
    
    // Getter 方法
    const std::vector<Vertex>& getVertices() const;
    const std::vector<Normal>& getNormals() const;
    const std::vector<TexCoord>& getTexCoords() const;
    const std::vector<Face>& getFaces() const;
    
    // MTL 相關方法
    const MTLLoader& getMTLLoader() const;
    std::vector<std::string> getMaterialNames() const;
    const MTLMaterial* getMaterial(const std::string& name) const;
    
    // 取得按材質分組的面
    std::map<std::string, std::vector<Face>> getFacesByMaterial() const;
};

// CTexturePool.h
#pragma once
#include <string>
#include <unordered_map>
#include <GL/glew.h>

struct TextureData {
    GLuint id;
    int    width;
    int    height;
};

class CTexturePool {
public:
    static CTexturePool& getInstance();

    /// 取得或載入貼圖，並回傳資料（id、寬、高）
    const TextureData& getTexture(const std::string& path, bool bMipMap = false);

    /// 僅查詢已載入的貼圖資料，不存在時 id=0, width=height=0
    TextureData getTextureData(const std::string& path) const;

private:
    CTexturePool();
    ~CTexturePool();
    CTexturePool(const CTexturePool&) = delete;
    CTexturePool& operator=(const CTexturePool&) = delete;

    void cleanup();  // 釋放所有貼圖

    std::unordered_map<std::string, TextureData> m_pool;
};

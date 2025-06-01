#include "CTexturePool.h"
#include "png_loader.h"

CTexturePool& CTexturePool::getInstance() {
    static CTexturePool inst;
    return inst;
}

CTexturePool::CTexturePool() { }
CTexturePool::~CTexturePool() { cleanup(); }

void CTexturePool::cleanup() {
    for (auto& kv : m_pool) {
        glDeleteTextures(1, &kv.second.id);
    }
    m_pool.clear();
}

const TextureData& CTexturePool::getTexture(const std::string& path, bool bmipmap) {
    auto it = m_pool.find(path);
    if (it != m_pool.end()) {
        return it->second;
    }
    // 沒載過就載入
    int w = 0, h = 0;
    GLuint texID = png_load_SOIL(path.c_str(), &w, &h, bmipmap);
    TextureData td{ texID, w, h };
    m_pool.emplace(path, td);
    return m_pool[path];
}

TextureData CTexturePool::getTextureData(const std::string& path) const {
    auto it = m_pool.find(path);
    if (it != m_pool.end()) {
        return it->second;
    }
    return TextureData{ 0,0,0 };
}

// GLuint grassTex = CTexturePool::getInstance().getTexture("textures/grass.png");

// GLuint maybeTex = CTexturePool::getInstance().getTextureID("textures/wood.png");

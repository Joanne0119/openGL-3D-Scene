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

    /// ���o�θ��J�K�ϡA�æ^�Ǹ�ơ]id�B�e�B���^
    const TextureData& getTexture(const std::string& path, bool bMipMap = false);

    /// �Ȭd�ߤw���J���K�ϸ�ơA���s�b�� id=0, width=height=0
    TextureData getTextureData(const std::string& path) const;

private:
    CTexturePool();
    ~CTexturePool();
    CTexturePool(const CTexturePool&) = delete;
    CTexturePool& operator=(const CTexturePool&) = delete;

    void cleanup();  // ����Ҧ��K��

    std::unordered_map<std::string, TextureData> m_pool;
};

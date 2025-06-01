// Transform.h
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Transform {
    glm::vec3 position;     // 位置
    glm::vec3 rotation;     // 旋轉 (歐拉角，以度為單位)
    glm::vec3 scale;        // 縮放
    
    Transform() : position(0.0f), rotation(0.0f), scale(1.0f) {}
    
    // 取得變換矩陣
    glm::mat4 getMatrix() const {
        glm::mat4 model = glm::mat4(1.0f);
        
        // 套用變換：縮放 -> 旋轉 -> 平移
        model = glm::translate(model, position);
        model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, scale);
        
        return model;
    }
    
    // 設定縮放（統一縮放）
    void setScale(float uniformScale) {
        scale = glm::vec3(uniformScale);
    }
    
    // 設定縮放（各軸不同）
    void setScale(float x, float y, float z) {
        scale = glm::vec3(x, y, z);
    }
    
    // 設定位置
    void setPosition(float x, float y, float z) {
        position = glm::vec3(x, y, z);
    }
    
    // 設定旋轉
    void setRotation(float x, float y, float z) {
        rotation = glm::vec3(x, y, z);
    }
};

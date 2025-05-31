// v_NPR.glsl
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aColor;    // 頂點顏色模式用
layout(location=2) in vec3 aNormal;
layout(location=3) in vec2 aTex;    // Texture Coordinates

uniform mat4 mxModel;
uniform mat4 mxView;
uniform mat4 mxProj;

uniform vec3 viewPos;   // 照明計算必須有鏡頭位置 
uniform vec3 lightPos;  // 照明計算必須有光源位置 

out vec3 vNormal;      // 頂點的法向量 (N)
out vec3 vLight;       // 頂點指向光源的 Light(L) 向量
out vec3 vView;        // 頂點指向 view 的 view(V) 向量
out vec3 vColor;       // 頂點顏色
out vec3 v3Pos;        // 頂點的位置 

void main() {
    vec4 worldPos = mxModel * vec4(aPos, 1.0);
    v3Pos   = worldPos.xyz;
    vNormal = normalize((mat3(mxModel) * aNormal));
    vLight  = normalize(lightPos - v3Pos);
    vView   = normalize(viewPos - v3Pos);
    vColor   = aColor;
    gl_Position = mxProj * mxView * worldPos;
}
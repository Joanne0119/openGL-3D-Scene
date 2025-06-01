// v_phong.glsl
#version 330 core
layout(location=0) in vec3 aPos;
//layout(location=1) in vec3 aColor;
layout(location=2) in vec3 aNormal;
layout(location=3) in vec2 aTex;    // Texture Coordinates

uniform mat4 mxModel;
uniform mat4 mxView;
uniform mat4 mxProj;

uniform vec3 viewPos;
uniform vec3 lightPos;

out vec3 vNormal;
out vec3 vLight;
out vec3 vView;
out vec3 vColor;
out vec3 v3Pos;
out vec2 vTexCoord;

void main() {
    vec4 worldPos = mxModel * vec4(aPos, 1.0);
    v3Pos   = worldPos.xyz;
    vNormal = normalize((mat3(mxModel) * aNormal));
    vLight  = normalize(lightPos - v3Pos);
    vView   = normalize(viewPos - v3Pos);
//    vColor   = aColor;
    vColor = vec3(1.0, 1.0, 1.0);
    vTexCoord = aTex;
    gl_Position = mxProj * mxView * worldPos;
}


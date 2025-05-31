#version 330 core
layout (location = 0) in vec3 aPos;    // Pos
layout (location = 1) in vec3 aColor;  // Color
layout (location = 2) in vec3 aNormal; // Normal
layout (location = 3) in vec2 aTex;    // Texture Coordinates
out vec4 vColor;

uniform mat4 mxModel;
uniform mat4 mxView;
uniform mat4 mxProj;
uniform vec4 ui4Color;     // ����Τ@�C��
uniform int  uShadingMode;   // 1 = ���I��, 2 = �����, 3 = Gouraud(���I����)
uniform vec3 viewPos;   // �ө��p�⥲�������Y��m 

struct LightSource {
    vec3 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float constant;
    float linear;
    float quadratic;
    // Spot fields:
    vec3 direction;
    float cutOff;       // cos(inner)
    float outerCutOff;  // cos(outer)
    float exponent;
};
uniform LightSource uLight;

struct Material {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shininess;
};
uniform Material uMaterial;
uniform int lightType; // 0 = Point, 1 = Spot

void main() {
    if( uShadingMode == 1 ) { // �䴩 vertex color �� per vertex lighting�Aobject color �ۤv�W�[
        vColor = vec4(aColor, 1.0);
        gl_Position = mxProj * mxView * mxModel*vec4(aPos, 1.0);
        return;
    }

     // 1. �p��y�лP�k�u
     vec4 worldPos4 = mxModel * vec4(aPos, 1.0);
     vec3 v3Pos  = worldPos4.xyz;
     vec3 N = normalize((mat3(mxModel) * aNormal));
 //  vec3 N = normalize(mat3(transpose(inverse(mxModel))) * aNormal);

     // 2. ���ӦV�q
     vec3 L = normalize(uLight.position - v3Pos);
     vec3 V = normalize(viewPos - v3Pos);
     vec3 R = reflect(-L, N);

     // 3. ��¦ Ambient
     vec4 ambient = uLight.ambient * uMaterial.ambient;

     // 4. ���Ϯg
     float diff = max(dot(N, L), 0.0);
     vec4 diffuse = uLight.diffuse * diff * uMaterial.diffuse;

     // 5. �譱�Ϯg
     float RdotV = dot(R, V);
     float spec = pow(max(RdotV, 0.0), uMaterial.shininess);
     vec4 specular = uLight.specular * spec * uMaterial.specular;

     // 6. �I��
     float distance = length(uLight.position - v3Pos);
     float atten = 1.0 / (
           uLight.constant
         + uLight.linear   * distance
         + uLight.quadratic* distance * distance
     );

     // 7. �p�G�O Spot Light�A�[�W�I������
     if( lightType == 1 ) {
         float theta = dot(L, normalize(-uLight.direction));
         float intensity = clamp(
             (theta - uLight.outerCutOff) /
             (uLight.cutOff - uLight.outerCutOff),
             0.0,
             1.0
         );
         if( uLight.exponent == 1.0f ) atten *= intensity;
         else { atten *= pow(intensity, uLight.exponent); }
     }

     // 8. �X���C��ÿ�X
     vec4 result = (ambient + diffuse + specular) * atten;  
     result.w = 1.0f;  // �ثe���B�z�b�z����
     vColor = result;

     // �̫��v
     gl_Position = mxProj * mxView * worldPos4;
}

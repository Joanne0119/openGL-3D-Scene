#version 330 core

in vec3 vColor;
in vec3 vNormal;      // 頂點的法向量 (N)
in vec3 vLight;       // 頂點指向光源的 Light(L) 向量
in vec3 vView;        // 頂點指向 view 的 view(V) 向量
in vec3 v3Pos;

uniform int  uShadingMode;     // 1=頂點色, 2=物件色, 3=Per-Pixel
uniform vec4 ui4Color;         // 模型顏色

struct LightSource {
    vec3 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float constant;
    float linear;
    float quadratic;
    // 若有 Spot Light，可加以下
    vec3 direction;
    float cutOff;
    float outerCutOff;
    float exponent;
};
uniform LightSource uLight;

struct Material {
    vec4 ambient;   // ka
    vec4 diffuse;   // kd
    vec4 specular;  // ks
    float shininess;
};
uniform Material uMaterial;
out vec4 FragColor;

void main() {
    // 模式切換：1 = 頂點顏色、2 = 模型顏色，3 = Per Pixel Lighting
    if( uShadingMode == 1) { FragColor = vec4(vColor, 1.0);  return; }
    if( uShadingMode == 2 ){ FragColor = ui4Color; return; }

    // 3 = Per-Pixel Phong
    vec3 N = normalize(vNormal);
    vec3 L = normalize(vLight);
    vec3 V = normalize(vView);
    vec3 R = reflect(-L, N);

    // Ambient
    vec4 ambient = uLight.ambient * uMaterial.ambient;

    // Diffuse
    float diff = max(dot(N, L), 0.0);
    vec4 diffuse = uLight.diffuse * diff * uMaterial.diffuse;

    // Specular
    float spec = pow(max(dot(R, V), 0.0), uMaterial.shininess);
    vec4 specular = uLight.specular * spec * uMaterial.specular;

    // Attenuation
    float dist = length(uLight.position - v3Pos);
    float atten = 1.0 / (uLight.constant
                       + uLight.linear   * dist
                       + uLight.quadratic* dist * dist);

    // Spot Light 漸變（如有設定 cutOff > 0）
    if(uLight.cutOff > 0.0) {
        float theta = dot(L, normalize(-uLight.direction));
        float intensity = clamp(
            (theta - uLight.outerCutOff) /
            (uLight.cutOff - uLight.outerCutOff),
            0.0, 1.0
        );
         if( uLight.exponent == 1.0f ) atten *= intensity;
         else { atten *= pow(intensity, uLight.exponent); }
    }

    vec4 color = (ambient + diffuse + specular) * atten;
    color.w = 1.0f;
    FragColor  = color;
}

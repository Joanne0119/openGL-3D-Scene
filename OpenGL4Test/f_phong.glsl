#version 330 core

in vec3 vColor;
in vec3 vNormal;
in vec3 vLight;
in vec3 vView;
in vec3 v3Pos;
in vec2 vTexCoord;

uniform int  uShadingMode;
uniform vec4 ui4Color;

struct LightSource {
    vec3 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float constant;
    float linear;
    float quadratic;
    // Spot Light
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
    
    sampler2D diffuseTexture;
    sampler2D normalTexture;
    sampler2D specularTexture;
    
    bool hasDiffuseTexture;
    bool hasNormalTexture;
    bool hasSpecularTexture;
};
uniform Material uMaterial;
out vec4 FragColor;

void main() {

    if( uShadingMode == 1) { FragColor = vec4(vColor, 1.0);  return; }
    if( uShadingMode == 2 ){ FragColor = ui4Color; return; }

    // 3 = Per-Pixel Phong with Textures
    vec3 N = normalize(vNormal);
    vec3 L = normalize(vLight);
    vec3 V = normalize(vView);
    vec3 R = reflect(-L, N);
    
    vec4 texDiffuse = vec4(1.0);
    vec4 texSpecular = vec4(1.0);
    
    if(uMaterial.hasDiffuseTexture) {
        texDiffuse = texture(uMaterial.diffuseTexture, vTexCoord);
        if(texDiffuse.a < 0.01) texDiffuse = vec4(1.0);
    }
    
    if(uMaterial.hasSpecularTexture) {
        texSpecular = texture(uMaterial.specularTexture, vTexCoord);
    }

    // Ambient
    vec4 ambient = uLight.ambient * uMaterial.ambient * texDiffuse;

    // Diffuse
    float diff = max(dot(N, L), 0.0);
    vec4 diffuse = uLight.diffuse * diff * uMaterial.diffuse * texDiffuse;

    // Specular
    float spec = pow(max(dot(R, V), 0.0), uMaterial.shininess);
    vec4 specular = uLight.specular * spec * uMaterial.specular * texDiffuse;

    // Attenuation
    float dist = length(uLight.position - v3Pos);
    float atten = 1.0 / (uLight.constant
                       + uLight.linear   * dist
                       + uLight.quadratic* dist * dist);

    
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

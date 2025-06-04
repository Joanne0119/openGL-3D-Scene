#version 330 core

in vec3 vColor;
in vec3 vNormal;
in vec3 vLight;
in vec3 vView;
in vec3 v3Pos;
in vec2 vTexCoord;

// Tangent and Bitangent from Vertex Shader
in vec3 vTangent;
in vec3 vBitangent;

uniform int  uShadingMode;
uniform vec4 ui4Color;

#define MAX_LIGHTS 8

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
    int type; // 0 = POINT, 1 = SPOT, 2 = DIRECTIONAL
    bool enabled;
};

uniform LightSource uLights[MAX_LIGHTS];
uniform int uNumLights;

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

    mat3 TBN = mat3(normalize(vTangent), normalize(vBitangent), normalize(vNormal));
    vec3 normalMap = texture(uMaterial.normalTexture, vTexCoord).rgb;
    
//    vec3 N = normalize(TBN * (2.0 * normalMap - 1.0));
    vec3 N;
    if (uMaterial.hasNormalTexture) {
        mat3 TBN = mat3(normalize(vTangent), normalize(vBitangent), normalize(vNormal));
        vec3 normalMap = texture(uMaterial.normalTexture, vTexCoord).rgb;
        N = normalize(TBN * (2.0 * normalMap - 1.0));
    } else {
        N = normalize(vNormal);
    }

//    vec3 L = normalize(vLight);
    vec3 V = normalize(vView);
//    vec3 H = normalize(L + V);  // Halfway vector (Blinn-Phong)
//    vec3 R = reflect(-L, N); // Phong

    vec4 texDiffuse = vec4(1.0);
    vec4 texSpecular = vec4(1.0);

    if(uMaterial.hasDiffuseTexture) {
        texDiffuse = texture(uMaterial.diffuseTexture, vTexCoord);
        if(texDiffuse.a < 0.01) texDiffuse = vec4(1.0);
    }

    if(uMaterial.hasSpecularTexture) {
        texSpecular = texture(uMaterial.specularTexture, vTexCoord);
    }
    
    vec4 finalColor = vec4(0.0);
        
    // add ambient, diffuse, specular
    vec4 totalAmbient = vec4(0.0);
    vec4 totalDiffuse = vec4(0.0);
    vec4 totalSpecular = vec4(0.0);
    
    for (int i = 0; i < min(uNumLights, MAX_LIGHTS); i++) {
        if (!uLights[i].enabled) continue;
        
        vec3 L;
        float attenuation = 1.0;
        
        if (uLights[i].type == 2) { // DIRECTIONAL
            L = normalize(-uLights[i].direction);
        } else { // POINT or SPOT
            L = normalize(uLights[i].position - v3Pos);
            
            float dist = length(uLights[i].position - v3Pos);
            attenuation = 1.0 / (uLights[i].constant + uLights[i].linear * dist + uLights[i].quadratic * dist * dist);
        }
        
        // Spot light
        if (uLights[i].type == 1 && uLights[i].cutOff > 0.0) { // SPOT
            float theta = dot(L, normalize(-uLights[i].direction));
            float intensity = clamp(
                (theta - uLights[i].outerCutOff) / (uLights[i].cutOff - uLights[i].outerCutOff),
                0.0, 1.0
            );
            
            if (uLights[i].exponent == 1.0) {
                attenuation *= intensity;
            } else {
                attenuation *= pow(intensity, uLights[i].exponent);
            }
        }
        
        vec3 H = normalize(L + V);
        
        // first ambient
        if (i == 0) {
            totalAmbient += uLights[i].ambient * uMaterial.ambient * texDiffuse * attenuation;
        }
        
        // Diffuse
        float diff = max(dot(N, L), 0.0);
        totalDiffuse += uLights[i].diffuse * diff * uMaterial.diffuse * texDiffuse * attenuation;
        
        // Specular
        float spec = pow(max(dot(N, H), 0.0), uMaterial.shininess);
        totalSpecular += uLights[i].specular * spec * uMaterial.specular * texSpecular * attenuation;
    }
    
    finalColor = totalAmbient + totalDiffuse + totalSpecular;
    finalColor = clamp(finalColor, 0.0, 1.0);
    finalColor.w = 1.0;
    FragColor = finalColor;
    
}

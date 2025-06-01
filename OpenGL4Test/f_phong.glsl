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

    mat3 TBN = mat3(normalize(vTangent), normalize(vBitangent), normalize(vNormal));
    vec3 normalMap = texture(uMaterial.normalTexture, vTexCoord).rgb;
    vec3 N = normalize(TBN * (2.0 * normalMap - 1.0));

    vec3 L = normalize(vLight);
    vec3 V = normalize(vView);
    vec3 H = normalize(L + V);  // Halfway vector (Blinn-Phong)
    vec3 R = reflect(-L, N); // Phong

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

    // Specular (Blinn-Phong)
    float spec = pow(max(dot(N, H), 0.0), uMaterial.shininess);
    //float spec = smoothstep(0.0, 1.0, pow(max(dot(N, H), 0.0), uMaterial.shininess)); // Smoothstep
    vec4 specular = uLight.specular * spec * uMaterial.specular * texSpecular * texDiffuse; // Specular color modulation

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
//void main() {
//
//    if( uShadingMode == 1) { FragColor = vec4(vColor, 1.0);  return; }  // Assuming vColor is available if needed
//    if( uShadingMode == 2 ){ FragColor = ui4Color; return; }
//
//    // 3 = Per-Pixel Phong with Textures
//    // Normal Mapping
//    mat3 TBN = mat3(normalize(vTangent), normalize(vBitangent), normalize(vNormal));
//    vec3 normalMap = texture(uMaterial.normalTexture, vTexCoord).rgb;
//    vec3 N = normalize(TBN * (2.0 * normalMap - 1.0)); // Tangent to World Space
//
//    vec3 L = normalize(vLight);
//    vec3 V = normalize(vView);
//    vec3 R = reflect(-L, N);
//
//    vec4 texDiffuse = vec4(1.0);
//    vec4 texSpecular = vec4(1.0);
//
//    if(uMaterial.hasDiffuseTexture) {
//        texDiffuse = texture(uMaterial.diffuseTexture, vTexCoord);
//        if(texDiffuse.a < 0.01) texDiffuse = vec4(1.0);
//    }
//
//    if(uMaterial.hasSpecularTexture) {
//        texSpecular = texture(uMaterial.specularTexture, vTexCoord);
//    }
//
//    // Ambient
//    vec4 ambient = uLight.ambient * uMaterial.ambient * texDiffuse;
//
//    // Diffuse
//    float diff = max(dot(N, L), 0.0);
//    vec4 diffuse = uLight.diffuse * diff * uMaterial.diffuse * texDiffuse;
//
//    // Specular
//    float spec = pow(max(dot(R, V), 0.0), uMaterial.shininess);
//    vec4 specular = uLight.specular * spec * uMaterial.specular * texSpecular;  // Use texSpecular here
//
//    // Attenuation
//    float dist = length(uLight.position - v3Pos); // Assuming v3Pos is available
//    float atten = 1.0 / (uLight.constant
//                        + uLight.linear   * dist
//                        + uLight.quadratic* dist * dist);
//
//
//    if(uLight.cutOff > 0.0) {
//        float theta = dot(L, normalize(-uLight.direction));
//        float intensity = clamp(
//            (theta - uLight.outerCutOff) /
//            (uLight.cutOff - uLight.outerCutOff),
//            0.0, 1.0
//        );
//         if( uLight.exponent == 1.0f ) atten *= intensity;
//         else { atten *= pow(intensity, uLight.exponent); }
//    }
//
//    vec4 color = (ambient + diffuse + specular) * atten;
//    color.w = 1.0f;
//    FragColor  = color;
//}

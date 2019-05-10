#version 330 core
struct EngineMaterial 
{
	sampler2D texture_diffuse1;
	sampler2D texture_normal;
	sampler2D texture_specular1;
	float shininess;
};

struct VS_OUT 
{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 ViewPos;
	mat3 invTBN;
	vec4 FragPosLightSpace;
};

struct EnginePointLight
{
	vec3 position;
	vec3 color;
	float intensity;
	float distance;
};
const float pointConstant = 1.0f;
const float pointLinear = 0.09f;
const float pointQuadratic = 0.032f;

vec3 calculate_point_light(EnginePointLight light, VS_OUT fs_in, EngineMaterial material, vec3 normal)
{

	float distance    = length(light.position - fs_in.FragPos);
	float attenuation = min(light.distance / (pointConstant + pointLinear * distance +
		pointQuadratic * (distance * distance)), 1.0);
	if(attenuation < 0.01)
		attenuation = 0.0;
	if (light.distance < distance)
		return vec3(0,0,0);
	vec3 lightDir = normalize(light.position - fs_in.FragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = 0.5 * diff * vec3(texture(material.texture_diffuse1, fs_in.TexCoords))* light.color;

	vec3 viewDir = normalize(fs_in.ViewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);  
	vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    vec3 specular = spec * vec3(texture(material.texture_specular1, fs_in.TexCoords))* light.color;

	return light.intensity * attenuation * (diffuse + specular);
}

struct EngineDirectionLight 
{
    vec3 direction;
	float intensity;
	vec3 color;
};

vec3 calculate_directional_light(EngineDirectionLight light, VS_OUT fs_in, EngineMaterial material, vec3 normal)
{
 // diffuse 
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = 0.5 * diff * vec3(texture(material.texture_diffuse1, fs_in.TexCoords)) * light.color;
    
    // specular
    vec3 viewDir = normalize(fs_in.ViewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
	vec3 specular = spec * vec3(texture(material.texture_specular1, fs_in.TexCoords))* light.color;
        
    return light.intensity * (diffuse + specular);
}

struct EngineSpotLight {
	vec3 position;
	vec3  direction;
	vec3 color;
	float cutOff;
	float outerCutOff;
	float intensity;
};

vec3 calculate_spot_light(EngineSpotLight light, VS_OUT fs_in, EngineMaterial material, vec3 normal)
{
	vec3 lightDir = normalize(light.position - fs_in.FragPos);
  	float theta = dot(lightDir, normalize(-light.direction));
    float epsilon   = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0); 
	     
	// diffuse 
	vec3 norm = normalize(normal);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = 0.5 * diff * vec3(texture(material.texture_diffuse1, fs_in.TexCoords))* light.color;
    
	// specular
	vec3 viewDir = normalize(fs_in.ViewPos - fs_in.FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);  
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
	vec3 specular =  spec * vec3(texture(material.texture_specular1, fs_in.TexCoords))* light.color;
    return light.intensity * intensity * (diffuse + specular);
}

float linearize(float depth, float zNear, float zFar) {

	return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

const int MAX_POINT_LIGHT = 128;
const int MAX_SPOT_LIGHT = 5;

uniform EnginePointLight pointLights[MAX_POINT_LIGHT];
uniform EngineSpotLight spotLights[MAX_SPOT_LIGHT];
uniform EngineDirectionLight directionLight;
uniform int pointLightsNmb = 0;
uniform int spotLightsNmb = 0;
uniform bool directionalLightEnable = false;
uniform float ambientIntensity = 0.2;







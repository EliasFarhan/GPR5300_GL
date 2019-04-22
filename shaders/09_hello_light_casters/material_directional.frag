
layout(location = 0) out vec4 FragColor;
struct TextureMaterial {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};
struct DirectionLight {
//vec3 position; //no longer needed for directional lights
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
    uniform DirectionLight light;

    uniform vec3 objectColor;
    uniform vec3 lightColor;
    uniform vec3 viewPos;

 in vec3 FragPos;
 in vec3 Normal;
 in vec2 TexCoords;
uniform TextureMaterial material;


void main()
{    
    // ambient
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
        
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
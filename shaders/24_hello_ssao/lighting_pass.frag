layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D gSsaoAlbedo;

uniform vec3 viewPos;

void main()
{             
    // retrieve data from G-buffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;
    float AmbientOcclusion = texture(gSsaoAlbedo, TexCoords).r;
    // then calculate lighting as usual
    vec3 lighting = AmbientOcclusion * Albedo * ambientIntensity; // hard-coded ambient component
    vec3 viewDir = normalize(viewPos - FragPos);
    for(int i = 0; i < pointLightsNmb; ++i)
    {
		EnginePointLight light = pointLights[i];
		float distance    = length(light.position - FragPos);
		float attenuation = min(light.distance / (pointConstant + pointLinear * distance + 
    				pointQuadratic * (distance * distance)), 1.0); 
		if(attenuation < 0.01)
			attenuation = 0.0;

		vec3 lightDir = normalize(light.position - FragPos);
		float diff = max(dot(Normal, lightDir), 0.0);
		vec3 diffuse = 0.5 * diff * Albedo * light.color;

		vec3 viewDir = normalize(viewPos - FragPos);
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
        vec3 specular = light.color * spec * Specular;
		lighting += light.intensity * attenuation * (diffuse + specular);
    }
    FragColor = vec4(lighting, 1.0);
	float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}  
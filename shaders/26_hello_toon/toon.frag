out vec4 FragColor;

in VS_OUT vs_out;
in vec3 Normal;

uniform EngineMaterial material;
uniform vec4 unlitColor = vec4(0.5,0.5,0.5,1);
uniform vec4 litColor = vec4(0.75,0.75,0.75,1);
uniform float diffuseThreshold = 0.5;
uniform vec4 specColor = vec4(1,1,1,1); 
uniform float shininess = 32.0;


void main()
{
    vec3 normal = Normal;
   
    // get diffuse color
    vec3 color = texture(material.texture_diffuse1, vs_out.TexCoords).rgb;
    // ambient
    vec3 ambient = ambientIntensity * color;
	vec3 lightColor = vec3(0.0,0.0,0.0);
	
	for(int i = 0; i < pointLightsNmb;i++)
	{
		vec3 currentLightColor = calculate_point_light(
			pointLights[i], 
			vs_out, 
			material, 
			normal);
		vec3 viewDir = normalize(vs_out.ViewPos - vs_out.FragPos);
        vec3 lightDir = pointLights[i].position - vs_out.FragPos;
        float attenuation = pointLights[i].distance / length(lightDir);
		lightDir = normalize(lightDir);

		vec3 fragmentColor = vec3(unlitColor); 
            
        // low priority: diffuse illumination
        if (attenuation * max(0.0, dot(normal, lightDir)) >= diffuseThreshold)
        {
            fragmentColor = vec3(litColor); 
        }
        
       
        vec3 reflectDir = reflect(-lightDir, normal);  
		vec3 halfwayDir = normalize(lightDir + viewDir);
		float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
        // highest priority: highlights
        if (dot(normal, lightDir) > 0.0 
            // light source on the right side?
            && attenuation *  spec > 0.5) 
               // more than half highlight intensity? 
		{
			fragmentColor = vec3(specColor);
		}
		currentLightColor = fragmentColor;
          

		lightColor += currentLightColor;
	}
	
    FragColor = vec4(ambient + lightColor * color, 1.0);
}
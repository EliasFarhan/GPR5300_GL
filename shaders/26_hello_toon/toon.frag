out vec4 FragColor;

in VS_OUT vs_out;
in vec3 Normal;

uniform EngineMaterial material;
uniform vec4 unlitColor = vec4(0.25,0.25,0.25,1);
uniform vec4 litColor = vec4(1,1,1,1);
uniform float shininess = 32.0;
uniform int toonLayers = 4;



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
		vec3 currentLightColor;
		vec3 viewDir = normalize(vs_out.ViewPos - vs_out.FragPos);
        vec3 lightDir = pointLights[i].position - vs_out.FragPos;
        float attenuation = pointLights[i].distance / length(lightDir);
		lightDir = normalize(lightDir);

		vec3 fragmentColor = vec3(unlitColor); 
            
        // low priority: diffuse illumination
		for(int i = 0; i < toonLayers; i++)
		{
			if (attenuation * max(0.0, dot(normal, lightDir)) >= 0.5+1.0*i/toonLayers/2.0)
			{
				fragmentColor = vec3(litColor)*(unlitColor.r+(1.0-unlitColor.r)*i/toonLayers); 
			}
		}
		currentLightColor = fragmentColor;
          

		lightColor += currentLightColor;
	}
	
    FragColor = vec4(ambient + lightColor * color, 1.0);
}
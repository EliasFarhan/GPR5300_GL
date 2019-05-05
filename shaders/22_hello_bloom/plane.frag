layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in VS_OUT vs_out;

uniform EngineMaterial material;

void main()
{
	// obtain normal from normal map in range [0,1]
    vec3 normal = texture(material.texture_normal, vs_out.TexCoords).rgb;
	// transform normal vector to range [-1,1]
	normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space
	//set normal to world space
	normal = vs_out.invTBN * normal;
	normal = normalize(normal);
   
    // get diffuse color
    vec3 color = texture(material.texture_diffuse1, vs_out.TexCoords).rgb;
    // ambient
    vec3 ambient = ambientIntensity * color;
	vec3 lightColor = vec3(0.0,0.0,0.0);

	for(int i = 0; i < pointLightsNmb;i++)
	{
		lightColor += calculate_point_light(
			pointLights[i], 
			vs_out, 
			material, 
			normal);
	}

	FragColor = vec4(ambient + lightColor, 1.0);
	float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
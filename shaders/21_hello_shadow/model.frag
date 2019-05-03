out vec4 FragColor;

in VS_OUT vs_out;
uniform sampler2D shadowMap;

uniform EngineMaterial material;
uniform vec3 viewPos;
uniform bool shadowBiasEnable = false;
uniform bool pcf = false;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal)
{
	// perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    if(shadowBiasEnable)
	{
		float bias = 0.005;  
		if(pcf)
		{ 
			float shadow = 0.0;
			vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
			for(int x = -1; x <= 1; ++x)
			{
				for(int y = -1; y <= 1; ++y)
				{
					float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
					shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
				}    
			}
			shadow /= 9.0;
			return shadow;
		}
		else
		{
			float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;  
			if(projCoords.z > 1.0)
				shadow = 0.0;
			return shadow;
		}
	}
	else
	{
		if(pcf)
		{
			float shadow = 0.0;
			vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
			for(int x = -1; x <= 1; ++x)
			{
				for(int y = -1; y <= 1; ++y)
				{
					float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
					shadow += currentDepth > pcfDepth ? 1.0 : 0.0;        
				}    
			}
			shadow /= 9.0;
			return shadow;
		}
		else
		{
			// check whether current frag pos is in shadow
			float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;
			if(projCoords.z > 1.0)
				shadow = 0.0;
			return shadow;
		}
	}
}

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
	if(directionalLightEnable)
	{
		lightColor += calculate_directional_light(
		directionLight, 
		vs_out, 
		material, 
		normal);
	}
	for(int i = 0; i < pointLightsNmb;i++)
	{
		lightColor += calculate_point_light(
			pointLights[i], 
			vs_out, 
			material, 
			normal);
	}
	for(int i = 0; i < spotLightsNmb;i++)
	{
		lightColor += calculate_spot_light(
			spotLights[i], 
			vs_out, 
			material, 
			normal);
	}
	
    float shadow = ShadowCalculation(vs_out.FragPosLightSpace, normal); 
    FragColor = vec4(ambient + (1.0 - shadow) * lightColor, 1.0);
}
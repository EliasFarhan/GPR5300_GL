out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D texNoise;

uniform vec3 samples[64];
uniform mat4 projection;
uniform bool enableRangeCheck = false;

uniform int kernelSize = 64;
uniform float radius = 0.5;
uniform float bias = 0.025;

// tile noise texture over screen based on screen dimensions divided by noise size
const vec2 noiseScale = vec2(1280.0/4.0, 720.0/4.0); // screen = 1280x720

void main()
{
	vec3 fragPos   = texture(gPosition, TexCoords).xyz;
	vec3 normal    = texture(gNormal, TexCoords).rgb;
	vec3 randomVec = texture(texNoise, TexCoords * noiseScale).xyz;
	vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN       = mat3(tangent, bitangent, normal); 
	vec3 color = texture(gAlbedoSpec, TexCoords).rgb;
	float occlusion = 0.0;
	for(int i = 0; i < kernelSize; ++i)
	{
		// get sample position
		vec3 sample = TBN * samples[i]; // From tangent to view-space
		sample = fragPos + sample * radius; 
		vec4 offset = vec4(sample, 1.0);
		offset      = projection * offset;    // from view to clip-space
		offset.xyz /= offset.w;               // perspective divide
		offset.xyz  = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0  

		float sampleDepth = texture(gPosition, offset.xy).z; 
		occlusion += (sampleDepth >= sample.z + bias ? 1.0 : 0.0);  
		if(enableRangeCheck)
		{
			float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
			occlusion += (sampleDepth >= sample.z + bias ? 1.0 : 0.0) * rangeCheck;
		}
	}
	occlusion = 1.0 - (occlusion / kernelSize);
	FragColor = vec4(occlusion * color,1.0);
}
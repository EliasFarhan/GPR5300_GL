layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec3 gSsaoAlbedo;

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

    // store the fragment position vector in the first gbuffer texture
    gPosition = vs_out.FragPos;
    // also store the per-fragment normals into the gbuffer
    gNormal = normalize(normal);
    // and the diffuse per-fragment color
    gAlbedoSpec.rgb = texture(material.texture_diffuse1, vs_out.TexCoords).rgb;
    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = texture(material.texture_specular1, vs_out.TexCoords).r;
	gSsaoAlbedo = vec3(0.95);
}  
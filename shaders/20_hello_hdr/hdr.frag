out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D hdrBuffer;
uniform bool toneMapping = true;
uniform float exposure = 1.0;
void main()
{             
	if(toneMapping)
	{
		const float gamma = 2.2;
		vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
  
		// Exposure tone mapping
		vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
		// Gamma correction 
		mapped = pow(mapped, vec3(1.0 / gamma));
  
		FragColor = vec4(mapped, 1.0);
	}
	else
	{
		vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
		FragColor = vec4(hdrColor, 1.0);
	}  
}
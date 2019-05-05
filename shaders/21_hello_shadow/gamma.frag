
layout(location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform bool gammaCorrection = false;

void main()
{ 
    vec3 fragColor = texture(screenTexture, TexCoords).rgb;
	if(gammaCorrection)
	{
		float gamma = 2.2;
		fragColor = pow(fragColor, vec3(1.0/gamma));
	}
	FragColor = vec4(fragColor,1.0);
}
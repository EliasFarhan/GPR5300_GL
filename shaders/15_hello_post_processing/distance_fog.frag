layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D depthTexture;
uniform vec3 fogColor;
uniform float zFar;// = 100.0;
uniform float zNear;// = 0.1;
uniform float fogBegin;// = 20.0;
uniform float fogComplete;// = 30.0;


void main()
{ 
	vec4 screenColor = texture(screenTexture, TexCoords);
	vec4 depthColor = texture(depthTexture, TexCoords);
	float depthValue = linearize(depthColor.x, 0.1, 100.0);
	
	vec4 color = mix(screenColor, vec4(fogColor,1.0), depthValue);
	
	//float depthValue = 1.0-linearize(depthColor.r);
	//vec4 color = mix(screenColor, vec4(fogColor,1.0), depthValue);
    FragColor = color;
}
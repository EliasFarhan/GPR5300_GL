
out vec4 FragColor;
in vec2 TexCoords;
in vec3 Normals;
in vec4 Position;

uniform float waterHeight;
uniform sampler2D texture_diffuse1;

void main()
{    
	if(Position.y < waterHeight)
		discard;
    FragColor = texture(texture_diffuse1, TexCoords);
}
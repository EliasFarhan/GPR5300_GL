layout(location = 0) out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
	vec4 p1 = texture(texture1, TexCoord);
	vec4 p2 = texture(texture2, TexCoord);
	FragColor = mix(p1.rgba*p1.a, p2.rgba*p2.a, 0.2);
}
out vec4 FragColor;

in VS_OUT vs_out;

uniform EngineMaterial material;

void main()
{
    // get diffuse color
    vec3 color = texture(material.texture_diffuse1, vs_out.TexCoords).rgb;

    FragColor = vec4(color, 1.0);
}
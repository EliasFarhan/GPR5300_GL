layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 viewPos;
uniform vec2 texTiling = vec2(1,1);

void main()
{
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));   
    vs_out.TexCoords = vec2(aTexCoords.x * texTiling.x, aTexCoords.y * texTiling.y);
    vs_out.ViewPos  =  viewPos;
    
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    vec3 B = normalize(normalMatrix * aBitangent);
    
	vs_out.invTBN = mat3(T, B, N);
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}  
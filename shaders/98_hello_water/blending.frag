
layout(location = 0) out vec4 FragColor;

in vec2 TexCoords;
in vec4 FragPos;
in vec4 FragWorldPos;

uniform sampler2D reflectionMap;//clipspace
uniform sampler2D refractionMap;//clipspace
uniform sampler2D dudvMap;//local space
uniform sampler2D depthMap;//clipspace
uniform sampler2D normalMap;//local space

uniform float waveStrength;// = 0.02;
uniform vec3 viewPos;
uniform float timeSinceStart;
uniform float waveSpeed;// = 0.01;

void main()
{ 
	
	vec2 dudv = texture(dudvMap, TexCoords+(timeSinceStart*waveSpeed)).rg * 2.0 - 1.0;
	vec3 normal = texture(normalMap, TexCoords+(timeSinceStart*waveSpeed)).rgb;
	normal = normalize(vec3(normal.r, normal.b, normal.g))*2.0-1.0;
	float refractiveFactor = abs(dot(normalize(viewPos-FragWorldPos.xyz), normal));
	vec2 ndc = (FragPos.xy/FragPos.w)/2.0 + 0.5;//clipspace
	float depth = linearize(texture(depthMap, ndc).r, 0.1, 100.0);
	float depthAttenuation = clamp(abs(depth-FragWorldPos.z), 0.0,1.0);
	ndc += depthAttenuation * dudv * waveStrength;
	ndc = clamp(ndc, 0.001, 0.999);
	vec4 reflectionColor = texture(reflectionMap, vec2(1.0-ndc.x, ndc.y));
	vec4 refractionColor = texture(refractionMap, ndc);
    FragColor = mix(reflectionColor, refractionColor, refractiveFactor);
}
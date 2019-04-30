#version 330 core

struct VS_OUT
{
	vec3 FragPos;
	vec2 TexCoords;
	vec3 ViewPos;
	mat3 invTBN;
	vec4 FragPosLightSpace;
};

mat4 scale(float x, float y, float z) {
	return mat4(
		vec4(x, 0.0, 0.0, 0.0),
		vec4(0.0, y, 0.0, 0.0),
		vec4(0.0, 0.0, z, 0.0),
		vec4(0.0, 0.0, 0.0, 1.0)
	);
}

mat4 translate(float x, float y, float z) {
	return mat4(
		vec4(1.0, 0.0, 0.0, 0.0),
		vec4(0.0, 1.0, 0.0, 0.0),
		vec4(0.0, 0.0, 1.0, 0.0),
		vec4(x, y, z, 1.0)
	);
}

mat4 RotateX(float phi) {
	return mat4(
		vec4(1., 0., 0., 0),
		vec4(0., cos(phi), -sin(phi), 0.),
		vec4(0., sin(phi), cos(phi), 0.),
		vec4(0., 0., 0., 1.));
}

mat4 RotateY(float theta) {
	return mat4(
		vec4(cos(theta), 0., -sin(theta), 0),
		vec4(0., 1., 0., 0.),
		vec4(sin(theta), 0., cos(theta), 0.),
		vec4(0., 0., 0., 1.));
}

mat4 RotateZ(float psi) {
	return mat4(
		vec4(cos(psi), -sin(psi), 0., 0),
		vec4(sin(psi), cos(psi), 0., 0.),
		vec4(0., 0., 1., 0.),
		vec4(0., 0., 0., 1.));
}

mat4 identity() {
	return mat4(
		vec4(1.0, 0.0, 0.0, 0.0),
		vec4(0.0, 1.0, 0.0, 0.0),
		vec4(0.0, 0.0, 1.0, 0.0),
		vec4(0.0, 0.0, 0.0, 1.0)
	);
}
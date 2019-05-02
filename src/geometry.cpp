#include <engine.h>
#include <graphics.h>
#include <geometry.h>

void Plane::Init()
{
	// positions
	const glm::vec3 pos1(-1.0f, 1.0f, 0.0f);
	const glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
	const glm::vec3 pos3(1.0f, -1.0f, 0.0f);
	const glm::vec3 pos4(1.0f, 1.0f, 0.0f);
	// texture coordinates
	const glm::vec2 uv1(0.0f, 1.0f);
	const glm::vec2 uv2(0.0f, 0.0f);
	const glm::vec2 uv3(1.0f, 0.0f);
	const glm::vec2 uv4(1.0f, 1.0f);
	// normal vector
	glm::vec3 nm(0.0f, 0.0f, 1.0f);


	// triangle 1
	// ----------
	glm::vec3 edge1 = pos2 - pos1;
	glm::vec3 edge2 = pos3 - pos1;
	glm::vec2 deltaUV1 = uv2 - uv1;
	glm::vec2 deltaUV2 = uv3 - uv1;

	GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	glm::vec3 tangent1 = glm::normalize((edge1 * deltaUV2.y - edge2 * deltaUV1.y) * f);
	glm::vec3 bitangent1 = glm::normalize((-deltaUV2.x * edge1 + deltaUV1.x * edge2) * f);

	// triangle 2
	// ----------
	edge1 = pos3 - pos1;
	edge2 = pos4 - pos1;
	deltaUV1 = uv3 - uv1;
	deltaUV2 = uv4 - uv1;

	f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	glm::vec3 tangent2 = glm::normalize((edge1 * deltaUV2.y - edge2 * deltaUV1.y) * f);
	glm::vec3 bitangent2 = glm::normalize((-deltaUV2.x * edge1 + deltaUV1.x * edge2) * f);


	vertices = {
		// positions            // normal         // texcoords  // tangent                          // bitangent
		pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
		pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
		pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

		pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
		pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
		pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
	};
	// configure plane VAO
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);

	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
	glBindVertexArray(0);
}


void Plane::Draw() const
{
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void Cube::Init()
{
	const int verticesNmb = 36;
	const int faceNmb = 6;
	
	float cubeVertices[5*36] = 
	{
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	};
	
	vertices.reserve(14 * verticesNmb);
	auto it = vertices.begin();
	for(int i = 0 ; i < faceNmb*2;i++)
	{

		const int verticeData = 5;
		const int faceVerticesNmb = 3*verticeData;
		glm::vec3 pos1 = glm::vec3(cubeVertices[i * faceVerticesNmb], 
			cubeVertices[i * faceVerticesNmb + 1],
			cubeVertices[i * faceVerticesNmb + 2]);
		glm::vec3 pos2 = glm::vec3(cubeVertices[i * faceVerticesNmb + verticeData],
			cubeVertices[i * faceVerticesNmb + 1 + verticeData],
			cubeVertices[i * faceVerticesNmb + 2 + verticeData]);
		glm::vec3 pos3 = glm::vec3(cubeVertices[i * faceVerticesNmb + verticeData*2],
			cubeVertices[i * faceVerticesNmb + 1+verticeData*2],
			cubeVertices[i * faceVerticesNmb + 2+verticeData*2]);

		glm::vec3 normal = glm::normalize(glm::cross(pos2-pos1, pos3-pos1));

		glm::vec2 uv1 = glm::vec2(cubeVertices[i * faceVerticesNmb + 3],
			cubeVertices[i * faceVerticesNmb + 1 + 4]);
		glm::vec2 uv2 = glm::vec2(cubeVertices[i * faceVerticesNmb + 3],
			cubeVertices[i * faceVerticesNmb + 1 + 4]);
		glm::vec2 uv3 = glm::vec2(cubeVertices[i * faceVerticesNmb + 3],
			cubeVertices[i * faceVerticesNmb + 1 + 4]);

		// triangle 1
		// ----------
		glm::vec3 edge1 = pos2 - pos1;
		glm::vec3 edge2 = pos3 - pos1;
		glm::vec2 deltaUV1 = uv2 - uv1;
		glm::vec2 deltaUV2 = uv3 - uv1;

		GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		glm::vec3 tangent1 = glm::normalize((edge1 * deltaUV2.y - edge2 * deltaUV1.y) * f);
		glm::vec3 bitangent1 = glm::normalize((-deltaUV2.x * edge1 + deltaUV1.x * edge2) * f);


		for (int k = 0; k < 3; k++)
		{
			const int index = i * faceVerticesNmb + verticeData * k;
			vertices.insert(it, &cubeVertices[index], &cubeVertices[index + 3]);//pos
			vertices.insert(it + 3, (float*)&normal, ((float*)&normal) + 3);//normal
			vertices.insert(it + 6, &cubeVertices[index + 3], &cubeVertices[index + 5]);//uv
			vertices.insert(it + 8, (float*)&tangent1, ((float*)&tangent1)+3);//tangent
			vertices.insert(it + 11, (float*)&bitangent1, ((float*)&bitangent1)+3);//bitangent

			it += 14;
		}

	}

	// configure cube VAO
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);

	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), &vertices[0], GL_STATIC_DRAW);
	//position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
	//normal
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
	//texcoords uv
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
	//tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
	//bitangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
	glBindVertexArray(0);

}

void Cube::Draw()
{
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

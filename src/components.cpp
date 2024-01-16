#include "components.hpp"
#include "render_system.hpp" // for gl_has_errors

#define STB_IMAGE_IMPLEMENTATION
#include "../ext/stb_image/stb_image.h"

// stlib
#include <iostream>
#include <sstream>

Debug debugging;

// Very, VERY simple OBJ loader from https://github.com/opengl-tutorials/ogl tutorial 7
// Works with TexturedVertex, input file is expected to contain uv maps for all vertices
// Colors and normals are omitted since we will not be using those info
bool Mesh::loadFromOBJFile(std::string obj_path, std::vector<TexturedVertex>& out_texture_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size,
	std::vector<ColoredVertex>& out_color_vertices, bool containColorVertices)
{
	// disable warnings about fscanf and fopen on Windows
#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

	printf("Loading OBJ file %s...\n", obj_path.c_str());
	std::vector<glm::vec2> out_uvs;

	FILE* file = fopen(obj_path.c_str(), "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}

	
	while (1) {
		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		if (strcmp(lineHeader, "v") == 0) {
			TexturedVertex vertex_textured;
			if (containColorVertices) {
				ColoredVertex vertex_colored;
				int matches = fscanf(file, "%f %f %f %f %f %f\n", &vertex_colored.position.x, &vertex_colored.position.y, &vertex_colored.position.z,
					&vertex_colored.color.x, &vertex_colored.color.y, &vertex_colored.color.z);
				if (matches == 3)
					vertex_colored.color = { 1,1,1 };
				out_color_vertices.push_back(vertex_colored);
				vertex_textured.position = vertex_colored.position;
				vertex_textured.texcoord = {0,0};
			}
			else {
				fscanf(file, "%f %f %f\n", &vertex_textured.position.x, &vertex_textured.position.y, &vertex_textured.position.z);
			}
			out_texture_vertices.push_back(vertex_textured);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			//uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			out_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], normalIndex[3], uvIndex[3];

			int matches = fscanf(file, "%d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);
			
			if (matches == 1) // try again
			{
				// Note first vertex index is already consumed by the first fscanf call (match ==1) since it aborts on the first error
				matches = fscanf(file, "//%d %d//%d %d//%d\n", &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);
				if (matches != 5) // try again
				{
					matches = fscanf(file, "%d/%d %d/%d/%d %d/%d/%d\n", &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
					if (matches != 8)
					{
						printf("File can't be read by our simple parser :-( Try exporting with other options\n");
						fclose(file);
						return false;
					}
				}
			}
;
			// -1 since .obj starts counting at 1 and OpenGL starts at 0
			out_vertex_indices.push_back((uint16_t)vertexIndex[0] - 1);
			out_vertex_indices.push_back((uint16_t)vertexIndex[1] - 1);
			out_vertex_indices.push_back((uint16_t)vertexIndex[2] - 1);
			
			
			// Find uv coordinate of each vertex by mapping the uv indices to vertex indices
			if (matches == 8 && !containColorVertices) {
				for (int i = 0; i < 3; i++) {
					out_texture_vertices[vertexIndex[i] - 1].texcoord = out_uvs[uvIndex[i] - 1];
				}
			} else if(matches != 8 && !containColorVertices){
				printf("WARNING: Cannot find uv info from input obj file");
			}
		}
		else {
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}
	}
	fclose(file);

	// Compute bounds of the mesh
	vec3 max_position = { -99999,-99999,-99999 };
	vec3 min_position = { 99999,99999,99999 };

	for (TexturedVertex& pos : out_texture_vertices)
	{
		max_position = glm::max(max_position, pos.position);
		min_position = glm::min(min_position, pos.position);
	}
		
	if(abs(max_position.z - min_position.z)<0.001)
	max_position.z = min_position.z+1; // don't scale z direction when everythin is on one plane

	vec3 size3d = max_position - min_position;
	out_size = size3d;

	// Normalize mesh to range -0.5 ... 0.5
	for (TexturedVertex& pos : out_texture_vertices)
				pos.position = ((pos.position - min_position) / size3d) - vec3(0.5f, 0.5f, 0.5f);

	if (containColorVertices) {
		for (ColoredVertex& pos : out_color_vertices)
			pos.position = ((pos.position - min_position) / size3d) - vec3(0.5f, 0.5f, 0.5f);
	}
	
	return true;
}

#include <IO/ObjLoader.h>
#include <Utils/FileUtils.h>
#include <Utils/Errors.h>
#include <Utils/Tokenizer.h>
#include <Math/Vec3.h>
#include <Math/Vec2.h>

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include <IO/TinyLoader.h>

#include <stdio.h>

namespace Enjon {

	Mesh LoadMeshFromFile(const std::string& FilePath)
	{
		Mesh Mesh;

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		std::string err;
		bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, FilePath.c_str());

		if (!err.empty()) { // `err` may contain warning message.
		  std::cerr << err << std::endl;
		}

		if (!ret) {
		  exit(1);
		}

		// Loop over shapes
		for (size_t s = 0; s < shapes.size(); s++) {
		  // Loop over faces(polygon)
		  size_t index_offset = 0;
		  for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
		    int fv = shapes[s].mesh.num_face_vertices[f];

		    // Loop over vertices in the face.
		    for (size_t v = 0; v < fv; v++) {
		      // access to vertex
		      tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
		      float vx = attrib.vertices[3*idx.vertex_index+0];
		      float vy = attrib.vertices[3*idx.vertex_index+1];
		      float vz = attrib.vertices[3*idx.vertex_index+2];
		      float nx = attrib.normals[3*idx.normal_index+0];
		      float ny = attrib.normals[3*idx.normal_index+1];
		      float nz = attrib.normals[3*idx.normal_index+2];
		      float tx = attrib.texcoords[2*idx.texcoord_index+0];
		      float ty = attrib.texcoords[2*idx.texcoord_index+1];

		      Vert Vertex = {};
		      Vertex.Position[0] = vx;
		      Vertex.Position[1] = vy;
		      Vertex.Position[2] = vz;
		      Vertex.Normals[0] = nx;
		      Vertex.Normals[1] = ny;
		      Vertex.Normals[2] = nz;
		      Vertex.UV[0] = tx;
		      Vertex.UV[1] = ty;
		      Vertex.Tangent[0] = 1.0f;
		      Vertex.Tangent[1] = 0.0f;
		      Vertex.Tangent[2] = 0.0f;

		      Mesh.Verticies.push_back(Vertex);
		    }

		    index_offset += fv;

		    // per-face material
		    shapes[s].mesh.material_ids[f];
		  }
		}

		// Now calculate tangents for each vert in mesh
		for (int i = 0; i < Mesh.Verticies.size(); i += 3)
		{
			auto& Vert1 = Mesh.Verticies.at(i);
			auto& Vert2 = Mesh.Verticies.at(i + 1);
			auto& Vert3 = Mesh.Verticies.at(i + 2);

			EM::Vec3 pos1 = EM::Vec3(Vert1.Position[0], Vert1.Position[1], Vert1.Position[2]);
			EM::Vec3 pos2 = EM::Vec3(Vert2.Position[0], Vert2.Position[1], Vert2.Position[2]);
			EM::Vec3 pos3 = EM::Vec3(Vert3.Position[0], Vert3.Position[1], Vert3.Position[2]);

			EM::Vec2 uv1 = EM::Vec2(Vert1.UV[0], Vert1.UV[1]);
			EM::Vec2 uv2 = EM::Vec2(Vert2.UV[0], Vert2.UV[1]);
			EM::Vec2 uv3 = EM::Vec2(Vert3.UV[0], Vert3.UV[1]);

			// calculate tangent vectors of both triangles
		    EM::Vec3 tangent;

		    // - triangle 1
		    EM::Vec3 edge1 = pos2 - pos1;
		    EM::Vec3 edge2 = pos3 - pos1;
		    EM::Vec2 deltaUV1 = uv2 - uv1;
		    EM::Vec2 deltaUV2 = uv3 - uv1;

		    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		    tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		    tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		    tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		    tangent = EM::Vec3::Normalize(tangent);

		    // Set tangents for the verticies
		    Vert1.Tangent[0] = tangent.x;
		    Vert1.Tangent[1] = tangent.y;
		    Vert1.Tangent[2] = tangent.z;

		    Vert2.Tangent[0] = tangent.x;
		    Vert2.Tangent[1] = tangent.y;
		    Vert2.Tangent[2] = tangent.z;

		    Vert3.Tangent[0] = tangent.x;
		    Vert3.Tangent[1] = tangent.y;
		    Vert3.Tangent[2] = tangent.z;
		}

		// Create and upload mesh data
	    glGenBuffers(1, &Mesh.VBO);
	    glBindBuffer(GL_ARRAY_BUFFER, Mesh.VBO);
	    glBufferData(GL_ARRAY_BUFFER, sizeof(Vert) * Mesh.Verticies.size(), &Mesh.Verticies[0], GL_STATIC_DRAW);

	    glGenVertexArrays(1, &Mesh.VAO);
	    glBindVertexArray(Mesh.VAO);

	    // Position
	    glEnableVertexAttribArray(0);
	    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*)offsetof(Vert, Position));
	    // Normal
	    glEnableVertexAttribArray(1);
	    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*)offsetof(Vert, Normals));
	    // Tangent
	    glEnableVertexAttribArray(2);
	    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*)offsetof(Vert, Tangent));
	    // UV
	    glEnableVertexAttribArray(3);
	    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*)offsetof(Vert, UV));

	    // Unbind VAO
	    glBindVertexArray(0);

	    // Set draw type
	    Mesh.DrawType = GL_TRIANGLES;
	    // Set draw count
	    Mesh.DrawCount = Mesh.Verticies.size();

		return Mesh;
	}

	/*
	Mesh LoadMeshFromFile(const char* FilePath)
	{
		Mesh Mesh;

		std::vector<EM::Vec3> TempVertexPositions;
		std::vector<EM::Vec3> TempVertexNormals;
		std::vector<EM::Vec2> TempVertexUVs;

		char* FileContents = EU::ReadFileContentsIntoString(FilePath);

		// Now need to parse contents and load mesh

		EU::tokenizer Tokenizer = {};
		Tokenizer.At = FileContents;

		bool Parsing = true;
		while(Parsing)
		{
			EU::token Token = EU::GetToken(&Tokenizer);

			switch(Token.Type)
			{
				default: 
				{
				} break;

				case EU::token_type::Token_Hash:
				{
					// Consume the entire line until we hit the new line
					// while(Token.Type != EU::token_type::Token_NewLine)
					// {
					// 	Token = EU::GetToken(&Tokenizer);
					// 	// printf("%s\n", NullTerminatedStringFromToken(&Token));
					// }

				} break;

				case EU::token_type::Token_Unknown: 
				{
				} break;

				case EU::token_type::Token_Identifier:
				{
					// Texture coordinate
					if (EU::TokenEquals(Token, "vt"))
					{
						EU::token XToken 	= EU::GetToken(&Tokenizer);
						EU::token YToken 	= EU::GetToken(&Tokenizer);

						// Need to make new char* that are null terminated
						char* XString = NullTerminatedStringFromToken(&XToken);
						char* YString = NullTerminatedStringFromToken(&YToken);

						// Parse all vert positions as floats
						float X = std::atof(XString); 
						float Y = std::atof(YString); 

						TempVertexUVs.push_back(EM::Vec2(X, Y));

						// Delete strings to free memory
						free(XString);
						free(YString);
					}

					// Vertex Normal
					else if (EU::TokenEquals(Token, "vn"))
 					{
 						// Parse a normal
						EU::token XToken 	= EU::GetToken(&Tokenizer);
						EU::token YToken 	= EU::GetToken(&Tokenizer);
						EU::token ZToken 	= EU::GetToken(&Tokenizer);

						// Need to make new char* that are null terminated
						char* XString = NullTerminatedStringFromToken(&XToken);
						char* YString = NullTerminatedStringFromToken(&YToken);
						char* ZString = NullTerminatedStringFromToken(&ZToken);

						// Parse all vert positions as floats
						float X = std::atof(XString); 
						float Y = std::atof(YString); 
						float Z = std::atof(ZString); 

						TempVertexNormals.push_back(EM::Vec3(X, Y, Z));

						// Delete strings to free memory
						free(XString);
						free(YString);
						free(ZString);
					}

					// Vertex Position
					else if (EU::TokenEquals(Token, "v"))
					{
						if (Tokenizer.At[0] != ' ') continue;

						else
						{
							// Parse a Position
							EU::token XToken 	= EU::GetToken(&Tokenizer);
							EU::token YToken 	= EU::GetToken(&Tokenizer);
							EU::token ZToken 	= EU::GetToken(&Tokenizer);

							// Need to make new char* that are null terminated
							char* XString = NullTerminatedStringFromToken(&XToken);
							char* YString = NullTerminatedStringFromToken(&YToken);
							char* ZString = NullTerminatedStringFromToken(&ZToken);

							// printf("XString: %s\n", XString);
							// printf("YString: %s\n", YString);
							// printf("ZString: %s\n", ZString);

							// Parse all vert positions as floats
							float X = std::atof(XString); 
							float Y = std::atof(YString); 
							float Z = std::atof(ZString); 

							TempVertexPositions.push_back(EM::Vec3(X, Y, Z));

							// Delete strings to free memory
							free(XString);
							free(YString);
							free(ZString);
						}
					}

					else if (EU::TokenEquals(Token, "s"))
					{
					}

					else if (EU::TokenEquals(Token, "usemtl"))
					{

					}	

					else if (EU::TokenEquals(Token, "f"))
					{
						static int count = 0;
						count++;

						// Parse a face	
						EU::token PositionToken, NormalToken, UVToken;

						// for (auto& P : TempVertexPositions)
						// {
						// 	printf("v: %.2f %.2f %.2f\n", P.x, P.y, P.z);
						// }

						// for (auto& T : TempVertexUVs)
						// {
						// 	printf("vt: %.2f %.2f\n", T.x, T.y);
						// }

						// for (auto& N : TempVertexNormals)
						// {
						// 	printf("vn: %.2f %.2f %.2f\n", N.x, N.y, N.z);
						// }

						// Need to search ahead to find out whether or not this line contains a quad
						// or a triangle

						// Loop through the triangles
						for (int i = 0; i < 3; i++)
						{
							// First triangle
							PositionToken = EU::GetToken(&Tokenizer);

							// Check for first '/''
							if (Tokenizer.At[0] == '/')
							{
								// Check for another '/'. If exists, then there is normal but no texture
								if (Tokenizer.At[1] && Tokenizer.At[1] == '/')
								{
									// Consume slahes
									EU::token Slash = EU::GetToken(&Tokenizer);
									Slash = EU::GetToken(&Tokenizer);

									// Get normal
									NormalToken = EU::GetToken(&Tokenizer);
								}
								else	
								// Otherwise there is a UV coord
								{
									// Consume slash
									EU::token Slash = EU::GetToken(&Tokenizer);

									// Get uv
									UVToken = EU::GetToken(&Tokenizer);

									// Check for another slash for normal
									if (Tokenizer.At[0] && Tokenizer.At[0] == '/')
									{
										// Consume slash
										Slash = EU::GetToken(&Tokenizer);

										// Get normal
										NormalToken = EU::GetToken(&Tokenizer);
									}
								}
							}

							// Parse numbers
							char* PositionString 	= NullTerminatedStringFromToken(&PositionToken);
							char* UVString 			= NullTerminatedStringFromToken(&UVToken);
							char* NormalString 		= NullTerminatedStringFromToken(&NormalToken);

							// .obj files list indicies from 1 - n, so need to subtract 1
							int PositionIndex 	= std::atoi(PositionString) - 1;
							int UVIndex 		= std::atoi(UVString) - 1;
							int NormalIndex 	= std::atoi(NormalString) - 1;

							if(PositionIndex >= TempVertexPositions.size())
							{
								EU::FatalError("Failed to load mesh: Position Index: " + std::to_string(PositionIndex) + "Positions: " + std::to_string(TempVertexPositions.size()));
							}

							if(UVIndex >= TempVertexUVs.size())
							{
								EU::FatalError("Failed to load mesh: UV Index: " + std::to_string(UVIndex) + "UVs: " + std::to_string(TempVertexUVs.size()));
							}

							if(NormalIndex >= TempVertexNormals.size())
							{
								EU::FatalError("Failed to load mesh: Normal Index: " + std::to_string(NormalIndex) + "Normals: " + std::to_string(TempVertexNormals.size()));
							}


							// Now that we have the indicies, need to grab them from our previously stored temp 
							// positions, normals, and uvs and store them in the mesh
							Vert Vertex = {};

							EM::Vec3* Position 	= &TempVertexPositions.at(PositionIndex);
							EM::Vec2* UV 		= &TempVertexUVs.at(UVIndex);
							EM::Vec3* Normal 	= &TempVertexNormals.at(NormalIndex);

							// printf("%d: %s %s %s\n", count, PositionString, UVString, NormalString);
							// printf("%.2f %.2f %.2f\n", Position->x, Position->y, Position->z);
							// printf("%.2f %.2f \n", UV->x, UV->y);
							// printf("%.2f %.2f %.2f\n", Normal->x, Normal->y, Normal->z);
							// printf("\n\n");

							Vertex.Position[0] = Position->x;
							Vertex.Position[1] = Position->y;
							Vertex.Position[2] = Position->z;

							Vertex.Normals[0] = Normal->x;
							Vertex.Normals[1] = Normal->y;
							Vertex.Normals[2] = Normal->z;

							Vertex.UV[0] = UV->x;
							Vertex.UV[1] = UV->y;

							Vertex.Tangent[0] = 1.0f;
							Vertex.Tangent[1] = 0.0f;
							Vertex.Tangent[2] = 0.0f;

							Vertex.Bitangent[0] = 0.0f;
							Vertex.Bitangent[1] = 1.0f;
							Vertex.Bitangent[2] = 0.0f;

							Mesh.Verticies.push_back(Vertex);

							// Free char*s
							free(PositionString);
							free(UVString);
							free(NormalString);
						}

					}
					else
					{

					}

				} break;

				case EU::token_type::Token_EndOfStream: 
				{
					Parsing = false;
				}break;
			}
		}

		free(FileContents);

		// Debug print
		// printf("Positions: %d\n", TempVertexPositions.size());
		// printf("Normals: %d\n", TempVertexNormals.size());
		// printf("UVs: %d\n", TempVertexUVs.size());

		// Now calculate tangents for each vert in mesh
		for (int i = 0; i < Mesh.Verticies.size(); i += 3)
		{
			auto& Vert1 = Mesh.Verticies.at(i);
			auto& Vert2 = Mesh.Verticies.at(i + 1);
			auto& Vert3 = Mesh.Verticies.at(i + 2);

			EM::Vec3 pos1 = EM::Vec3(Vert1.Position[0], Vert1.Position[1], Vert1.Position[2]);
			EM::Vec3 pos2 = EM::Vec3(Vert2.Position[0], Vert2.Position[1], Vert2.Position[2]);
			EM::Vec3 pos3 = EM::Vec3(Vert3.Position[0], Vert3.Position[1], Vert3.Position[2]);

			EM::Vec2 uv1 = EM::Vec2(Vert1.UV[0], Vert1.UV[1]);
			EM::Vec2 uv2 = EM::Vec2(Vert2.UV[0], Vert2.UV[1]);
			EM::Vec2 uv3 = EM::Vec2(Vert3.UV[0], Vert3.UV[1]);

			// calculate tangent vectors of both triangles
		    EM::Vec3 tangent;

		    // - triangle 1
		    EM::Vec3 edge1 = pos2 - pos1;
		    EM::Vec3 edge2 = pos3 - pos1;
		    EM::Vec2 deltaUV1 = uv2 - uv1;
		    EM::Vec2 deltaUV2 = uv3 - uv1;

		    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		    tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		    tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		    tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		    tangent = EM::Vec3::Normalize(tangent);

		    // Set tangents for the verticies
		    Vert1.Tangent[0] = tangent.x;
		    Vert1.Tangent[1] = tangent.y;
		    Vert1.Tangent[2] = tangent.z;

		    Vert2.Tangent[0] = tangent.x;
		    Vert2.Tangent[1] = tangent.y;
		    Vert2.Tangent[2] = tangent.z;

		    Vert3.Tangent[0] = tangent.x;
		    Vert3.Tangent[1] = tangent.y;
		    Vert3.Tangent[2] = tangent.z;
		}


		return Mesh;
	}
	*/
}
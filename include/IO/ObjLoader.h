#ifndef ENJON_OBJ_LOADER_H
#define ENJON_OBJ_LOADER_H

#include <Graphics/Mesh.h>

namespace Enjon { 
	
	/*
		Parse the file
		It outputs a mesh?
		.obj files for now
		Meshes have verts, of course
		So for now, just use vectors. Eventually I'll switch to my own implementation of these
		as well as many other things
	
		.objs have the following format:

		# is a comment
		usemtl and mtllib describe look of model
		v is a vertex
		vt is texture coord
		vn is normal
		f is face

		faces are described by the notation v1/t1/n1 v2/t2/n2 v3/t3/n3 (e.g. 8/11/7 7/12/7 6/10/7)
		where 1 is first vertex, 2 is 2nd, etc.

	*/

	Mesh LoadMeshFromFile(const std::string& FilePath);
} 



#endif


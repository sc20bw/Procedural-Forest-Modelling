#include "simple_mesh.hpp"
#include <stdio.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

SimpleMeshData concatenate( SimpleMeshData aM, SimpleMeshData const& aN )
{
	aM.positions.insert( aM.positions.end(), aN.positions.begin(), aN.positions.end() );
	aM.colors.insert( aM.colors.end(), aN.colors.begin(), aN.colors.end() );
	aM.normals.insert( aM.normals.end(), aN.normals.begin(), aN.normals.end() );
	aM.textcoords.insert( aM.textcoords.end(), aN.textcoords.begin(), aN.textcoords.end() );
	return aM;
}


GLuint create_vao(SimpleMeshData const& aMeshData)
{
	//TODO: implement me
	//create VBOs and VAO
	GLuint posVBO = 0;
	glGenBuffers(1, &posVBO);

	//Now we need the actual vertex data, which is coming from aMeshData.pos
	//bind the position buffer, and fill it:
	glBindBuffer(GL_ARRAY_BUFFER, posVBO);
	//NB use of sizeof will work on arrays, but not pointers or classes such as vectors!
	glBufferData(GL_ARRAY_BUFFER, aMeshData.positions.size() * sizeof(Vec3f),
		aMeshData.positions.data(), GL_STATIC_DRAW);

	GLuint colourVBO = 0;
	glGenBuffers(1, &colourVBO);
	//Colour information for the three points, as RGB, using 0 - 1 scale. In cube.hpp

	//Now we bind the position buffer, and fill it:
	glBindBuffer(GL_ARRAY_BUFFER, colourVBO);
	glBufferData(GL_ARRAY_BUFFER, aMeshData.colors.size() * sizeof(Vec3f),
		aMeshData.colors.data(), GL_STATIC_DRAW);
	
	GLuint normVBO = 0;
	glGenBuffers(1, &normVBO);
	//Colour information for the three points, as RGB, using 0 - 1 scale. In cube.hpp

	//Now we bind the position buffer, and fill it:
	glBindBuffer(GL_ARRAY_BUFFER, normVBO);
	glBufferData(GL_ARRAY_BUFFER, aMeshData.normals.size() * sizeof(Vec3f),
		aMeshData.normals.data(), GL_STATIC_DRAW);

	GLuint textureVBO = 0;
	glGenBuffers(1, &textureVBO);
	//Colour information for the three points, as RGB, using 0 - 1 scale. In cube.hpp

	//Now we bind the position buffer, and fill it:
	glBindBuffer(GL_ARRAY_BUFFER, textureVBO);
	glBufferData(GL_ARRAY_BUFFER, aMeshData.textcoords.size() * sizeof(Vec2f),
		aMeshData.textcoords.data(), GL_STATIC_DRAW);

	//Now we generate a vertex array object, which tells openGL how to interpret the buffer data
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	//We now give it the actual info, and tell it what it means
	glBindBuffer(GL_ARRAY_BUFFER, posVBO);
	//Need to tell it which attribute, how many parts (eg 2 for x,y), data type, 
	//whether normalised, the stride (i.e any padding between elements),and offset from start
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	//...and now the colour data
	glBindBuffer(GL_ARRAY_BUFFER, colourVBO);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	
	//...and now the normal data
	glBindBuffer(GL_ARRAY_BUFFER, normVBO);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, textureVBO);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);

	//Reset state
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// Clean up buffers
	// Note: these are not deleted fully, as the VAO holds a reference to them.
	glDeleteBuffers(1, &colourVBO);
	glDeleteBuffers(1, &posVBO);
	glDeleteBuffers(1, &normVBO);
	//glDeleteBuffers(1, &textureVBO);

	return vao;
}

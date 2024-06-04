#version 430

// Input data
layout( location = 0 ) in vec3 iPosition;
layout( location = 1 ) in vec3 iColor;
layout( location = 2 ) in vec3 iNormal;
layout( location = 3 ) in vec2 textcoords;

// Uniforms
layout( location = 0 ) uniform mat4 uModelViewProjection;

out vec3 v2fColor;
out vec3 normal;
out vec3 fragPos;
out vec2 texCoords;

void main()
{
	fragPos = iPosition;
	v2fColor = iColor;
	normal = iNormal;
	texCoords = textcoords;

	gl_Position = uModelViewProjection * vec4( iPosition, 1.0 );
}


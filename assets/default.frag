#version 430

in vec3 v2fColor;
in vec3 normal;
in vec3 fragPos;
in vec2 texCoords;

layout( location = 1 ) uniform vec3 lightPos;
layout( location = 2 ) uniform vec3 lightColour;
layout( location = 3 ) uniform int textured;

layout( binding = 0 ) uniform sampler2D uTexture;

layout( location = 0 ) out vec4 oColor;

void main()
{
    float ambientStrength = 0.5;
	vec3 ambient = ambientStrength * lightColour;

	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(lightPos - fragPos);

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColour;
	vec3 result = (ambient + diffuse) * v2fColor;

	if(textured == 0){
		oColor = vec4(result, 1.0);
	}
	else if(textured == 1){
		ambientStrength = 0.3;
		ambient = ambientStrength * lightColour;
		result = (ambient + diffuse) * v2fColor;
		oColor = texture(uTexture, texCoords) * vec4(result, 1.0);
	}
	else if(textured == 2){
		ambientStrength = 0.2;
		ambient = ambientStrength * lightColour;
		result = (ambient + diffuse) * v2fColor;
		vec4 texColor = texture(uTexture, texCoords);
		if(texColor.a < 0.1){
			discard;
		}
		oColor = texColor * vec4(result, 1.0);
	}
}
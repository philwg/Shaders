#version 450

uniform mat4 MVP;

layout(location = 0) in vec3 position;
layout(location = 2) in vec2 UVpos;
layout(location = 3) in vec3 normale;

out vec4 fragColor;
out vec4 fragPosition;
out vec3 fragNormale;

out vec2 fragUV;

void main(){
		
	fragColor = vec4(1.0, 0.0, 1.0, 1.0);
	fragNormale = normale;
	fragPosition = vec4(position, 1.0);
	fragUV = UVpos;
	
	gl_Position = MVP*vec4(position, 1.0);
	
}

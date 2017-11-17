#version 450

uniform mat4 mvp;
layout(location = 0) in vec3 position;
out vec4 vColor;


void main() 
{

  vColor = vec4(1.0,0.0,0.0,1.0);	
  gl_Position = mvp * vec4(position,1.0) ;

}
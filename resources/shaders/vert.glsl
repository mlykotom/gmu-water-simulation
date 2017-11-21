#version 450

struct ObjectData
{
  vec2 position;
  vec2 size    ;
  vec4 color   ;
};

layout(binding=0)buffer Objects{ObjectData objects[];};

layout(location=0)in vec2 position;
layout(location=1)in int drawId;

out vec4 vColor;

void main() 
{
  ObjectData objectData = objects[drawId];
  vColor = objectData.color;
  gl_Position = vec4(position*objectData.size + objectData.position,1,1);
}
#version 330 core

layout( triangles ) in;
layout( line_strip, max_vertices = 6 ) out;

void main()
{
    gl_Position = gl_in[0].gl_Position; EmitVertex();
    gl_Position = gl_in[1].gl_Position; EmitVertex();
    EndPrimitive();

    gl_Position = gl_in[1].gl_Position; EmitVertex();
    gl_Position = gl_in[2].gl_Position; EmitVertex();
    EndPrimitive();

    gl_Position = gl_in[2].gl_Position; EmitVertex();
    gl_Position = gl_in[0].gl_Position; EmitVertex();
    EndPrimitive();

}

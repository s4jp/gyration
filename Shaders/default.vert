#version 460 core
layout (location = 0) in vec3 pos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform bool gravity;

float gravitySize = 0.05f;

void main()
{
   gl_Position = model * vec4(pos, 1.0);

   if (gl_VertexID == 9 || gl_VertexID == 12)
   {
     gl_Position.y -= gravitySize * 5;
   }
   if (gl_VertexID == 10 || gl_VertexID == 11)
   {
     gl_Position.y -= gravitySize * 4;

     if (gl_VertexID == 10)
     {
       gl_Position.x -= gravitySize;
     }
     else
     {
       gl_Position.x += gravitySize;
     }
   }

   gl_Position = proj * view * gl_Position;
}
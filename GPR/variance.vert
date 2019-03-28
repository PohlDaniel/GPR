#version 430 core

layout(location = 0) in vec3 position;
layout(location = 2) in float variance;


uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;
uniform mat4 u_modelView;


out float v_variance;

void main(void){

   gl_Position =   u_projection * u_view * u_model * vec4(position, 1.0);
   v_variance = variance;
}
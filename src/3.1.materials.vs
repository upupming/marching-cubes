#version 330 core
attribute vec3 a_position;
attribute vec3 a_normal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(a_position, 1.0));
    Normal = mat3(transpose(inverse(model))) * a_normal;

    gl_Position = projection * view * vec4(FragPos, 1.0);
}

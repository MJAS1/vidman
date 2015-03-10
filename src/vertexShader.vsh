#version 130

uniform mat4 mvpMatrix;

in vec2 vertex;
in vec2 textureCoordinate;

out vec2 varyingTextureCoordinate;

void main(void)
{
    varyingTextureCoordinate = textureCoordinate;
    gl_Position = vec4(vertex, 0, 1);
}

attribute vec3 vertex;
attribute vec2 texture;

uniform mat4 modelViewProjectionMatrix;
varying vec2 outUV;

void main()
{
    gl_Position = modelViewProjectionMatrix * vec4(vertex,1.0);
    outUV=texture;
}

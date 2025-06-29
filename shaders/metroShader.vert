attribute vec3 vertex;
attribute vec3 color;

uniform mat4 modelViewProjectionMatrix;

varying vec3 outColor;

void main()
{
    gl_Position=modelViewProjectionMatrix*vec4(vertex,1.0);
    outColor=color;
}

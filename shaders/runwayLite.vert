attribute vec3 vertex;
attribute vec4 color;

uniform mat4 modelViewProjectionMatrix;
varying vec4 outColor;

void main()
{
    gl_Position = modelViewProjectionMatrix * vec4(vertex,1.0);
    outColor=color;
}

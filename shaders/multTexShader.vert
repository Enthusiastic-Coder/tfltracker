attribute vec3 vertex;
attribute vec2 texture;
attribute vec2 texture2;

uniform mat4 modelViewProjectionMatrix;
varying vec2 outUV;
varying vec2 outUV2;

void main()
{
    gl_Position = modelViewProjectionMatrix * vec4(vertex,1.0);
    outUV=texture;
    outUV2 = texture2;
}

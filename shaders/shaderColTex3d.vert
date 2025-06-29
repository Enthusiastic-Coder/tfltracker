attribute vec3 vertex;
attribute vec2 texture;
attribute vec3 normal;
attribute vec3 color;

varying vec3 outNormal;
varying vec3 outColor;
varying vec2 outUV;

uniform mat4 modelViewProjectionMatrix;
uniform mat3 normalMatrix;

void main()
{
    gl_Position=modelViewProjectionMatrix*vec4(vertex,1.0);
    outNormal=normalMatrix*normal;
    outColor=color;
    outUV=texture;
}

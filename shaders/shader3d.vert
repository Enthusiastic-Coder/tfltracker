attribute vec3 vertex;
attribute vec3 normal;
attribute vec3 color;

varying vec3 outNormal;
varying vec3 outColor;

uniform mat4 modelViewProjectionMatrix;
uniform mat3 normalMatrix;

void main()
{
    gl_Position=modelViewProjectionMatrix*vec4(vertex,1.0);
    outNormal=normalMatrix*normal;
    outColor=color;
}

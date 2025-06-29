attribute vec2 vertex;
uniform mat4 modelViewProjectionMatrix;

void main()
{
    gl_Position = modelViewProjectionMatrix * vec4(vertex, 0.0 ,1.0);
}

uniform sampler2D texture0;
varying lowp vec2 outUV;

uniform lowp vec4 ground;
uniform lowp vec4 sea;

void main()
{
    lowp vec4 texColor = texture2D(texture0,outUV);

    if( texColor.g < 0.1)
        gl_FragColor = sea;
    else
        gl_FragColor = ground;
}

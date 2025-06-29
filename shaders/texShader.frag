uniform sampler2D texture0;
uniform lowp vec4 Color;

varying mediump vec2 outUV;

void main()
{
    lowp vec4 texColor = Color*texture2D(texture0,outUV);
    gl_FragColor = texColor;
}

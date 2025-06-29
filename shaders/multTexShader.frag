uniform sampler2D texture0;
uniform sampler2D texture2;
uniform lowp vec4 Color;

varying mediump vec2 outUV;
varying mediump vec2 outUV2;


void main()
{
    lowp vec4 tileColor = texture2D(texture2,outUV2);

    if( tileColor.r == 0.0)
        discard;

    gl_FragColor = Color*texture2D(texture0,outUV);
}

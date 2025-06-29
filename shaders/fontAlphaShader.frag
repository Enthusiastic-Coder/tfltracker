uniform sampler2D texID;
uniform lowp vec4 textColor;
varying mediump vec2 outUV;

void main()
{
    lowp vec4 texColor = texture2D(texID,outUV);
    if( texColor.a < 0.1)
        discard;

    gl_FragColor = textColor * texColor;
}

uniform sampler2D texID;
uniform lowp vec4 textColor;
varying mediump vec2 outUV;

void main()
{
    gl_FragColor = textColor * texture2D(texID, outUV);
}

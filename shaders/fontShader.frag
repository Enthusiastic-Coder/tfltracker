uniform sampler2D texID;
uniform lowp vec4 textColor;
uniform lowp vec4 backColor;
varying mediump vec2 outUV;

void main()
{
    lowp vec4 lookupColor = texture2D(texID, outUV);

    gl_FragColor = mix(backColor, textColor, lookupColor.a);
}

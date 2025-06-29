varying mediump vec3 outColor;
uniform lowp float factor;

void main()
{
    gl_FragColor=vec4(outColor*factor,1.0);
}

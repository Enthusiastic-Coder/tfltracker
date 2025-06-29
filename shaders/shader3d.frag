varying mediump vec3 outNormal;
varying mediump vec3 outColor;

uniform mediump vec3 lightDir;
uniform lowp vec3 ambientColor;
uniform lowp vec3 diffuseColor;

void main()
{
    mediump vec3 normal=normalize(outNormal);
    mediump vec3 surf2light=normalize(lightDir);
    mediump float diffuseContribution=max(0.0,dot(normal,surf2light));

    gl_FragColor=vec4(ambientColor+diffuseContribution *diffuseColor*outColor,1.0);
}

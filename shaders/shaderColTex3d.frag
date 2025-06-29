varying mediump vec3 outNormal;
varying mediump vec3 outColor;
varying mediump vec2 outUV;

uniform mediump vec3 lightDir;
uniform sampler2D texture0;
uniform lowp vec3 ambientColor;
uniform lowp vec3 diffuseColor;
uniform lowp float alpha;

void main()
{
    mediump vec3 diffuseTexture = texture2D(texture0,outUV).rgb;

    mediump vec3 normal=normalize(outNormal);
    mediump vec3 surf2light=normalize(lightDir);
    mediump float diffuseContribution=max(0.0,dot(normal,surf2light));

    gl_FragColor=vec4(ambientColor * diffuseTexture + diffuseColor * diffuseContribution * outColor *diffuseTexture, alpha);
}

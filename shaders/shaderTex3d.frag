varying mediump vec3 outNormal;
varying mediump vec2 outUV;

uniform sampler2D texture0;
uniform mediump vec3 lightDir;
uniform lowp vec3 ambientColor;
uniform lowp vec3 diffuseColor;
uniform int useLightDir;

void main()
{
    mediump vec3 diffuseTexture = texture2D(texture0,outUV).rgb;

    if( useLightDir == 1)
    {
        mediump vec3 normal=normalize(outNormal);
        mediump vec3 surf2light=normalize(lightDir);
        mediump float diffuseContribution=max(0.4,dot(normal,surf2light));

        gl_FragColor=vec4(ambientColor * diffuseTexture + diffuseContribution * diffuseTexture, 1.0);
    }
    else
        gl_FragColor=vec4(diffuseTexture*diffuseColor,1.0);
}

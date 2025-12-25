#version 120
#extension GL_EXT_texture_array : enable
uniform float alphaTest;
uniform float gamma;

varying vec3 fTex;
varying vec3 fLightmapTex0;
varying vec3 fLightmapTex1;
varying vec3 fLightmapTex2;
varying vec3 fLightmapTex3;
varying vec4 fColor;

uniform sampler2DArray sTex;
uniform sampler2D sLightmapTex0;
uniform sampler2D sLightmapTex1;
uniform sampler2D sLightmapTex2;
uniform sampler2D sLightmapTex3;

void main()
{
	vec4 texel = texture2DArray(sTex, fTex);
	if (alphaTest != 0.0) {
		if (texel.a == 0.0) {
			discard;
		}
	}
	else {
		texel.a = 1.0;
	}
	if (fColor.a == 0.0)
		discard;

	vec3 lightmap = texture2D(sLightmapTex0, fLightmapTex0.xy).rgb * fLightmapTex0.z;
	lightmap += texture2D(sLightmapTex1, fLightmapTex1.xy).rgb * fLightmapTex1.z;
	lightmap += texture2D(sLightmapTex2, fLightmapTex2.xy).rgb * fLightmapTex2.z;
	lightmap += texture2D(sLightmapTex3, fLightmapTex3.xy).rgb * fLightmapTex3.z;
	vec3 color = texel.rgb * lightmap * fColor.rgb;

	gl_FragColor = vec4(pow(color, vec3(1.0/gamma)), fColor.a*texel.a);
}
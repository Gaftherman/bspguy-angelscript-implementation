#include "shaders.h"

// IMPORTANT: 
// if you have an Nvidia GPU, compile shader code in GPU ShaderAnalyzer to be sure it works for AMD too.
// AMD has a stricter GLSL compiler than Nvidia does.

const char* g_shader_vec3_vertex =
"#version 120\n"
// object variables
"uniform mat4 modelViewProjection;\n"
"uniform vec4 color;\n"

// vertex variables
"attribute vec3 vPosition;\n"

// fragment variables
"varying vec4 fColor;\n"

"void main()\n"
"{\n"
"	gl_Position = modelViewProjection * vec4(vPosition, 1);\n"
"	fColor = color;\n"
"}\n";

const char* g_shader_vec3_fragment =
"#version 120\n"
"varying vec4 fColor;\n"

"void main()\n"
"{\n"
"	gl_FragColor = fColor;\n"
"	gl_FragDepth = gl_FragCoord.z;\n"
"}\n";

const char* g_shader_vec3depth_fragment =
"#version 120\n"
"varying vec4 fColor;\n"

"void main()\n"
"{\n"
"	gl_FragColor = fColor;\n"
"	gl_FragDepth = gl_FragCoord.z - 0.00001f;\n" // smol hack to fix z fighting with polys and outlines (sprites)
"}\n";

const char* g_shader_cVert_vertex =
"#version 120\n"
// object variables
"uniform mat4 modelViewProjection;\n"
"uniform vec4 colorMult;\n"

// vertex variables
"attribute vec3 vPosition;\n"
"attribute vec4 vColor;\n"

// fragment variables
"varying vec4 fColor;\n"

"void main()\n"
"{\n"
"	gl_Position = modelViewProjection * vec4(vPosition, 1);\n"
"	fColor = vColor * colorMult;\n"
"}\n";


const char* g_shader_cVert_fragment =
"#version 120\n"
"varying vec4 fColor;\n"

"void main()\n"
"{\n"
"	float gamma = 1.5;\n"
"	gl_FragColor = vec4(pow(fColor.rgb, vec3(1.0/gamma)), fColor.a);\n"
"}\n";


const char* g_shader_tVert_vertex =
"#version 120\n"
// object variables
"uniform mat4 modelViewProjection;\n"

// vertex variables
"attribute vec3 vPosition;\n"
"attribute vec2 vTex;\n"

// fragment variables
"varying vec2 fTex;\n"

"void main()\n"
"{\n"
"	gl_Position = modelViewProjection * vec4(vPosition, 1);\n"
"	fTex = vTex;\n"
"}\n";


const char* g_shader_tVert_fragment =
"#version 120\n"
"varying vec2 fTex;\n"

"uniform sampler2D sTex;\n"

"void main()\n"
"{\n"
"	gl_FragColor = texture2D(sTex, fTex);\n"
"}\n";


const char* g_shader_spr_vertex =
"#version 120\n"
// object variables
"uniform mat4 modelViewProjection;\n"
"uniform vec4 color;\n"

// vertex variables
"attribute vec3 vPosition;\n"
"attribute vec2 vTex;\n"

// fragment variables
"varying vec2 fTex;\n"
"varying vec4 fColor;\n"

"void main()\n"
"{\n"
"	gl_Position = modelViewProjection * vec4(vPosition, 1);\n"
"	fTex = vTex;\n"
"	fColor = color;\n"
"}\n";


const char* g_shader_spr_fragment =
"#version 120\n"
"varying vec2 fTex;\n"
"varying vec4 fColor;\n"

"uniform sampler2D sTex;\n"

"void main()\n"
"{\n"
"	gl_FragColor = texture2D(sTex, fTex) * fColor;\n"
"}\n";



const char* g_shader_multitexture_vertex =
"#version 120\n"
// object variables
"uniform mat4 modelViewProjection;\n"
"uniform vec4 colorMult;\n"

// vertex variables
"attribute vec3 vPosition;\n"
"attribute vec3 vTex;\n"
"attribute vec3 vLightmapTex0;\n"
"attribute vec3 vLightmapTex1;\n"
"attribute vec3 vLightmapTex2;\n"
"attribute vec3 vLightmapTex3;\n"
"attribute vec4 vColor;\n"

// fragment variables
"varying vec3 fTex;\n"
"varying vec3 fLightmapTex0;\n"
"varying vec3 fLightmapTex1;\n"
"varying vec3 fLightmapTex2;\n"
"varying vec3 fLightmapTex3;\n"
"varying vec4 fColor;\n"

"void main()\n"
"{\n"
"	gl_Position = modelViewProjection * vec4(vPosition, 1);\n"
"	fTex = vTex;\n"
"	fLightmapTex0 = vLightmapTex0;\n"
"	fLightmapTex1 = vLightmapTex1;\n"
"	fLightmapTex2 = vLightmapTex2;\n"
"	fLightmapTex3 = vLightmapTex3;\n"
"	fColor = vColor * colorMult;\n"
"}\n";

const char* g_shader_multitexture_fragment =
"#version 120\n"
"uniform float alphaTest;\n"
"uniform float gamma;\n"

"varying vec3 fTex;\n"
"varying vec3 fLightmapTex0;\n"
"varying vec3 fLightmapTex1;\n"
"varying vec3 fLightmapTex2;\n"
"varying vec3 fLightmapTex3;\n"
"varying vec4 fColor;\n"

"uniform sampler2D sTex;\n"
"uniform sampler2D sLightmapTex0;\n"
"uniform sampler2D sLightmapTex1;\n"
"uniform sampler2D sLightmapTex2;\n"
"uniform sampler2D sLightmapTex3;\n"

"void main()\n"
"{\n"
"	vec4 texel = texture2D(sTex, fTex.xy);\n"
"	if (alphaTest != 0.0) {\n"
"		if (texel.a == 0.0) {\n"
"			discard;\n"
"		}\n"
"	}\n"
"	else {\n"
"		texel.a = 1.0;\n"
"	}\n"

"	vec3 lightmap = texture2D(sLightmapTex0, fLightmapTex0.xy).rgb * fLightmapTex0.z;\n"
"	lightmap += texture2D(sLightmapTex1, fLightmapTex1.xy).rgb * fLightmapTex1.z;\n"
"	lightmap += texture2D(sLightmapTex2, fLightmapTex2.xy).rgb * fLightmapTex2.z;\n"
"	lightmap += texture2D(sLightmapTex3, fLightmapTex3.xy).rgb * fLightmapTex3.z;\n"
"	vec3 color = texel.rgb * lightmap * fColor.rgb;\n"

"	gl_FragColor = vec4(pow(color, vec3(1.0/gamma)), fColor.a*texel.a);\n"
"}\n";

const char* g_shader_multitexture_array_fragment =
"#version 120\n"
"#extension GL_EXT_texture_array : enable\n"
"uniform float alphaTest;\n"
"uniform float gamma;\n"

"varying vec3 fTex;\n"
"varying vec3 fLightmapTex0;\n"
"varying vec3 fLightmapTex1;\n"
"varying vec3 fLightmapTex2;\n"
"varying vec3 fLightmapTex3;\n"
"varying vec4 fColor;\n"

"uniform sampler2DArray sTex;\n"
"uniform sampler2D sLightmapTex0;\n"
"uniform sampler2D sLightmapTex1;\n"
"uniform sampler2D sLightmapTex2;\n"
"uniform sampler2D sLightmapTex3;\n"

"void main()\n"
"{\n"
"	vec4 texel = texture2DArray(sTex, fTex);\n"
"	if (alphaTest != 0.0) {\n"
"		if (texel.a == 0.0) {\n"
"			discard;\n"
"		}\n"
"	}\n"
"	else {\n"
"		texel.a = 1.0;\n"
"	}\n"

"	vec3 lightmap = texture2D(sLightmapTex0, fLightmapTex0.xy).rgb * fLightmapTex0.z;\n"
"	lightmap += texture2D(sLightmapTex1, fLightmapTex1.xy).rgb * fLightmapTex1.z;\n"
"	lightmap += texture2D(sLightmapTex2, fLightmapTex2.xy).rgb * fLightmapTex2.z;\n"
"	lightmap += texture2D(sLightmapTex3, fLightmapTex3.xy).rgb * fLightmapTex3.z;\n"
"	vec3 color = texel.rgb * lightmap * fColor.rgb;\n"

"	gl_FragColor = vec4(pow(color, vec3(1.0/gamma)), fColor.a*texel.a);\n"
"}\n";

const char* g_shader_multitexture_3d_fragment =
"#version 120\n"
"#extension GL_EXT_texture3D : enable\n"
"uniform float alphaTest;\n"
"uniform float gamma;\n"

"varying vec3 fTex;\n"
"varying vec3 fLightmapTex0;\n"
"varying vec3 fLightmapTex1;\n"
"varying vec3 fLightmapTex2;\n"
"varying vec3 fLightmapTex3;\n"
"varying vec4 fColor;\n"

"uniform sampler3D sTex;\n"
"uniform sampler2D sLightmapTex0;\n"
"uniform sampler2D sLightmapTex1;\n"
"uniform sampler2D sLightmapTex2;\n"
"uniform sampler2D sLightmapTex3;\n"

"void main()\n"
"{\n"
"	vec4 texel = texture3D(sTex, fTex);\n"
"	if (alphaTest != 0.0) {\n"
"		if (texel.a == 0.0) {\n"
"			discard;\n"
"		}\n"
"	}\n"
"	else {\n"
"		texel.a = 1.0;\n"
"	}\n"

"	vec3 lightmap = texture2D(sLightmapTex0, fLightmapTex0.xy).rgb * fLightmapTex0.z;\n"
"	lightmap += texture2D(sLightmapTex1, fLightmapTex1.xy).rgb * fLightmapTex1.z;\n"
"	lightmap += texture2D(sLightmapTex2, fLightmapTex2.xy).rgb * fLightmapTex2.z;\n"
"	lightmap += texture2D(sLightmapTex3, fLightmapTex3.xy).rgb * fLightmapTex3.z;\n"
"	vec3 color = texel.rgb * lightmap * fColor.rgb;\n"

"	gl_FragColor = vec4(pow(color, vec3(1.0/gamma)), fColor.a*texel.a);\n"
"}\n";

const char* g_shader_mdl_fragment =
"#version 120\n"
"varying vec2 fTex;\n"
"varying vec4 fColor;\n"

"uniform vec4 colorMult; \n"
"uniform sampler2D sTex;\n"

"void main()\n"
"{\n"
"	vec4 texel = texture2D(sTex, fTex);\n"
"	if (texel.a < 0.5) discard;\n" // GL_ALPHA_TEST alternative for WebGL (TODO: expensive. Don't use this for every mdl draw call)
"	gl_FragColor = texel * fColor * colorMult;\n"
"}\n";


const char* g_shader_mdl_vertex =
// transformation matrix
"#version 120\n"
"#define STUDIO_NF_CHROME 0x02\n"
"#define STUDIO_NF_ADDITIVE 0x20\n"

"uniform mat4 modelViewProjection;\n"

// Lighting uniforms
"uniform mat4 modelView;\n"
"uniform mat4 normalMat;\n"
"uniform mat3 lights[4];\n"
"uniform int elights; \n"
"uniform vec3 ambient; \n"

// skeleton
// 3D texture as an array of mat4 (poor man's UBO). Vertex Texture Fetch requires GL 3.0 or 2.1 w/ ARB
// Can't use UBO without upgrading to GL 3.1. Can't have 128 mat4 uniforms for all GPUs.
"uniform sampler3D boneMatrixTexture; \n"

// render flags
"uniform int chromeEnable; \n"
"uniform int additiveEnable; \n"
"uniform int flatshadeEnable; \n"

// chrome uniforms
"uniform vec3 viewerOrigin; \n"
"uniform vec3 viewerRight; \n"
"uniform vec2 textureST; \n"

// vertex variables
"attribute vec3 vPosition; \n"
"attribute vec3 vNormal; \n"
"attribute vec2 vTex; \n"
"attribute float vBone; \n"

// fragment variables
"varying vec2 fTex; \n"
"varying vec4 fColor; \n"

"vec4 lighting(inout vec3 tNormal); \n"
"vec3 rotateVector(vec3 v, inout mat4 mat); \n"
"vec3 irotateVector(vec3 v, mat4 mat); \n"
"vec2 chrome(vec3 tNormal, inout mat4 bone);\n"

"void main()\n"
"{\n"
"mat4 bone; \n"
"float boneCoord = (vBone / 128.0) + (1.0 / 512.0);"
"bone[0] = texture3D(boneMatrixTexture, vec3(0.00 + (1.0 / 8.0), 0, boneCoord)); \n"
"bone[1] = texture3D(boneMatrixTexture, vec3(0.25 + (1.0 / 8.0), 0, boneCoord)); \n"
"bone[2] = texture3D(boneMatrixTexture, vec3(0.50 + (1.0 / 8.0), 0, boneCoord)); \n"
"bone[3] = texture3D(boneMatrixTexture, vec3(0.75 + (1.0 / 8.0), 0, boneCoord)); \n"


"vec3 pos = rotateVector(vPosition, bone) + vec3(bone[0][3], bone[2][3], -bone[1][3]); \n"
"vec3 tNormal = rotateVector(vNormal, bone); \n"

"gl_Position = modelViewProjection * vec4(pos, 1); \n"

"if (chromeEnable != 0) { \n"
"	fTex = chrome(vNormal, bone); \n"
"} else { \n"
"	fTex = vTex; \n"
"} \n"

// TODO: compile multiple shaders and control this if #ifdef
"if (additiveEnable != 0) { \n"
"	fColor = vec4(1, 1, 1, 0.5); \n"
"} else if (flatshadeEnable == 1) { \n"
"	fColor = vec4(ambient, 1); \n"
"} else if (flatshadeEnable == 2) { \n"
"	fColor = vec4(1, 1, 1, 1); \n"
"} else { \n"
"	fColor = lighting(tNormal); \n"
"} \n"
"}\n"

"vec3 rotateVector(vec3 v, inout mat4 mat)\n"
"{\n"
"	vec3 vout; \n"
"	vout.x = dot(v, mat[0].xyz); \n"
"	vout.z = -(dot(v, mat[1].xyz)); \n"
"	vout.y = dot(v, mat[2].xyz); \n"
"	return vout; \n"
"}\n"

"vec3 irotateVector(vec3 v, mat4 mat)\n"
"{\n"
"	vec3 vout; \n"
"	vout.x = v.x * mat[0][0] + v.y * mat[1][0] + v.z * mat[2][0];\n"
"	vout.y = v.x * mat[0][1] + v.y * mat[1][1] + v.z * mat[2][1];\n"
"	vout.z = v.x * mat[0][2] + v.y * mat[1][2] + v.z * mat[2][2];\n"
"	return vout; \n"
"}\n"

"vec2 chrome(vec3 tNormal, inout mat4 bone)\n"
"{\n"
"	vec3 dir = normalize(viewerOrigin + vec3(bone[0][3], bone[2][3], -bone[1][3])); \n"

"	vec3 chromeupvec = normalize(cross(dir, viewerRight)); \n"
"	vec3 chromerightvec = normalize(cross(dir, chromeupvec)); \n"

"	vec3 chromeup = irotateVector(chromeupvec, bone); \n"
"	vec3 chromeright = irotateVector(chromerightvec, bone); \n"

"	vec2 chrome;\n"

// calc s coord
"	float n = dot(tNormal, chromeright);\n"
"	chrome.x = ((n + 1.0) * 32.0) * textureST.x;\n"

// calc t coord
"	n = dot(tNormal, chromeup);\n"
"	chrome.y = ((n + 1.0) * 32.0) * textureST.y;\n"

"	return chrome;\n"
"}\n"

"vec4 lighting(inout vec3 tNormal)\n"
"{\n"
"	vec3 finalColor = ambient; \n"
"	for (int i = 0; i < 4; ++i)\n"
"	{\n"
"		if (i == elights)\n" // Webgl won't let us use variables in our loop condition. So we have this.
"			break; \n"
"		vec3 lightDirection = normalize(lights[i][0].xyz); \n"
"		vec3 diffuse = lights[i][1].xyz; \n"

"		finalColor += diffuse * max(0.0, dot(tNormal, lightDirection)); \n"
"	}\n"
"	return vec4(clamp(finalColor, vec3(0, 0, 0), vec3(1, 1, 1)), 1); \n"
"}\n";
#version 120
varying vec4 fColor;

void main()
{
	float gamma = 1.5;
	gl_FragColor = vec4(pow(fColor.rgb, vec3(1.0/gamma)), fColor.a);
}
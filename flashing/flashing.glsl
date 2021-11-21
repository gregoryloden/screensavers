uniform sampler2D tex;
uniform float frame;
void main() {
	vec4 color = texture2D(tex, gl_TexCoord[0].st);
	vec3 upColor = mod(color.rgb + vec3(frame), 2);
	vec3 downColor = mod(vec3(4 - frame) - color.rgb, 2);
	gl_FragColor = vec4(min(upColor, downColor), 1);
}

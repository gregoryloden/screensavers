uniform sampler2D tex;
uniform float frame;
void main() {
	vec3 color = texture2D(tex, gl_TexCoord[0].st).rgb + vec3(frame);
	gl_FragColor = vec4(min(mod(color, 2), mod(-color, 2)), 1);
}

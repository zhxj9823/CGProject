#version 450

layout(location = 0) out vec4 frag_color;

uniform sampler2D HDR_buffer;
uniform sampler2D bloom_blur;

vec3 ACESToneMapping(vec3 color, float adapted_lum)
{
    const float A = 2.51f;
    const float B = 0.03f;
    const float C = 2.43f;
    const float D = 0.59f;
    const float E = 0.14f;
    color *= adapted_lum;
    return (color * (A * color + B)) / (color * (C * color + D) + E);
}

void main() {
	const float gamma = 2.2;
	const float exposure = 0.1;
	
	vec3 hdrColor = texelFetch(HDR_buffer, ivec2(gl_FragCoord.xy), 0).rgb;
	vec3 bloomColor = texelFetch(bloom_blur, ivec2(gl_FragCoord.xy), 0).rgb;
	hdrColor += bloomColor;
	
	vec3 mapped = ACESToneMapping(hdrColor, 0.3);
	// Exposure tone mapping
	//vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
	// Gamma correction 
	mapped = pow(mapped, vec3(1.0 / gamma));
	
	frag_color = vec4(mapped, 1.0);
}
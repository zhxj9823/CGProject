#version 450

layout(location = 0) in vec3 vertex_pos;

layout(location = 0) out vec4 fcolor;
layout(location = 1) out vec4 bright_color;

uniform sampler1D mie_texture;
uniform sampler3D cloud_texture;
uniform sampler3D cloud_structure;

uniform sampler2D diffuse_buffer;
uniform sampler2D depth_buffer;

uniform vec2 view_port;
uniform vec3 camera_pos;
uniform vec3 sun_pos;

uniform mat4 view;
uniform mat4 proj;
uniform mat4 inv_view;
uniform mat4 inv_proj;

float PI = 3.1415962;
float PI_r = 0.3183098;

float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float HG(float costheta) {
	float g = 0.9;
	return 1.25 * PI_r * (1 - pow(g, 2.0)) / pow((1 + pow(g, 2.0) - 2 * g * costheta), 1.5) + 0.5;
}

float Mie(float costheta) {
	float angle = acos(costheta);
	return 10 * texture(mie_texture, (PI - angle) * PI_r).r + 0.5;
}

float phase(vec3 v1, vec3 v2, float t) {
	float costheta = dot(v1, v2) / length(v1) / length(v2);
	return HG(-costheta);
	//return Mie(costheta);
}

float coverage(float t) {
	/* The lower level must be same as the value in the preprocessors structure function */
	return smoothstep(0.35, 0.4, t) * t;
}

float cloud_sampling_lowres(vec3 v, float delta) {
	v.y -= 80;
	vec4 texture = texture(cloud_structure, v / 600);
	return texture.r;
}

float cloud_sampling(vec3 v, float delta) {

	v.y -= 80;

	vec4 textureA = texture(cloud_texture, v / 600);

	float coverage = coverage(textureA.r);
	float bottom = smoothstep(0, 80, v.y);

	return textureA.r * coverage * bottom * delta * pow(textureA.b, 0.3) * pow(textureA.a, 0.4);
}


float cloud_sampling_lowres1(vec3 v, float delta) {
	if (v.x > 0 && v.x < 70 && v.z > 0 && v.z < 70 && v.y > 20 && v.y < 90) {
		return 1.0;
	}
	if (length(v - vec3(-20, 25, 0)) < 30) {
		return 1.0;
	}
	return 0.0;
}

float cloud_sampling1(vec3 v, float delta) {
	if (v.x > 10 && v.x < 60 && v.z > 10 && v.z < 60 && v.y > 30 && v.y < 80) {
		return 0.1;
	}
	if (length(v - vec3(-20, 25, 0)) < 25) {
		return 0.04;
	}
	return 0.0;
}

float cast_scatter_ray(vec3 origin, vec3 dir, float t) {
	float delta = 10.0;
	float end = 50.0;

	vec3 sample_point = vec3(0.0);
	float inside = 0.0;

	float phase = phase(dir, vec3(camera_pos - origin), t);

	for (float t = 0.0; t < end; t += delta) {
		sample_point = origin + dir * t;
		inside += cloud_sampling(sample_point, delta);
	}

	float scatter = 3 * exp(-0.7 * inside) * (1.0 - exp(-0.8 * inside));

	float value = scatter * phase;
	return value;
}	

vec4 cast_ray(vec3 origin, vec3 dir) {
	float delta_large = 20.0;
	float delta_small = 1.0 + 0.01 * rand(gl_FragCoord.xy);
	float start = gl_DepthRange.near;
	float end = 500.0;

	vec4 value = vec4(0.0);
	vec3 cloud_bright = vec3(254, 254, 250) * 0.03;//vec3(4.95, 4.8, 4.75);
	vec3 cloud_dark = vec3(0.6558441558, 0.9465648855, 1.55);//vec3(0.624, 0.777, 0.8805);//vec3(0.416, 0.518, 0.694); //vec3(0.671, 0.725, 0.753);
	value.rgb = cloud_dark;

	float length_inside = 0.0;
	bool inside = false;
	bool looking_for_new_inside = true;
	int points_inside = 0;
	vec3 sample_point = origin;

	bool first = false;
	float delta = delta_large;
	for (float t = start; t < end; t += delta) {
		sample_point = origin + dir * t;

		if (first) {
			 delta_small = 1.0;
		}

		/* Stop rays that are going below ground */
		if (sample_point.y < 0.0) {
			break;
		}

		/* Stop rays that already reached full opacity */
		if (value.a == 1.0) {
			break;
		}

		float alpha = 0.0;
		float alpha_lowres = 0.0;
		if (!inside) {
			alpha_lowres = cloud_sampling_lowres(sample_point, delta);
			if (alpha_lowres > 0.001) {
				inside = true;
			} else {
				looking_for_new_inside = true;
			}
		}

		if (inside) {
			/* Start of a new inside session? */
			if (looking_for_new_inside) {
				/* Move the starting point a large delta backwards */
				t -= delta_large;
				if (t < gl_DepthRange.near) {
					t = gl_DepthRange.near;
				}
				sample_point = origin + dir * t;
				delta = delta_small;
				looking_for_new_inside = false;
				points_inside = 0;
				first = true;
			}
			
			alpha = cloud_sampling(sample_point, delta); /* Comment this line to see cloud structure */
			value.a += alpha;
			value.a = clamp(value.a, 0.0, 1.0);
			points_inside += 1;

			/* Calculate the shadows and the scattering */
			float energy = cast_scatter_ray(sample_point, normalize(sun_pos - sample_point), t);
			float inside_weight = smoothstep(5, 0, points_inside);
			value.rgb += cloud_bright * energy * alpha;
		}

		/* Check next structure block if we are still inside */
		if (inside && (points_inside * delta_small) > delta_large) {
			alpha_lowres = cloud_sampling_lowres(sample_point, delta);
			if (alpha_lowres == 0.0) {
				inside = false;
				looking_for_new_inside = true;
				delta = delta_large;
			}
			points_inside = 0;
		}

		/* Adaptive step length */
		//delta_small = t > 100? 0.01 * t : 1.0;
	}

	return value;
}

void main() {
	/* Calculate the ray */
	float x = 2.0 * gl_FragCoord.x / view_port.x - 1.0;
	float y = 2.0 * gl_FragCoord.y / view_port.y - 1.0;
	vec2 ray_nds = vec2(x, y);
	vec4 ray_clip = vec4(ray_nds, -1.0, 1.0);
	vec4 ray_view = inv_proj * ray_clip;
	ray_view = vec4(ray_view.xy, -1.0, 0.0);
	vec3 ray_world = (inv_view * ray_view).xyz;
	ray_world = normalize(ray_world);

	vec4 cloud_color = cast_ray(camera_pos, ray_world);

	vec4 diffuse_color = texelFetch(diffuse_buffer, ivec2(gl_FragCoord.xy), 0);
	//float depth = pow(texelFetch(depth_buffer, ivec2(gl_FragCoord.xy), 0).x, 128.0);

	fcolor.a = 1.0;
	fcolor.rgb = mix(diffuse_color.rgb, cloud_color.rgb, cloud_color.a);

	float brightness = dot(fcolor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 500.0) {
        bright_color = vec4(fcolor.rgb, 1.0);
	}

	//fcolor = bright_color;

	//fcolor.rgb = vec3(ray_world);

	//fcolor = vec4(vec3(texture(perlin1, vec3(x, y, 0.0)).r), 1.0);
	//fcolor = vec4(vec3(texture(terrain_texture, vec2(gl_FragCoord.x / view_port.x, gl_FragCoord.y / view_port.y) * 6)), 1.0);
	vec3 s = vec3(texture(cloud_structure, vec3(gl_FragCoord.x / view_port.x, gl_FragCoord.y / view_port.y, 0.5) * 2).r);
	vec3 t = texture(cloud_texture, vec3(gl_FragCoord.x / view_port.x, gl_FragCoord.y / view_port.y, 0.59)).rrr * 1.2;
	t = vec3(coverage(t.r));
	//fcolor = vec4(t, 1.0);
}
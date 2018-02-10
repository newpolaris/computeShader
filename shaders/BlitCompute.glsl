-- Compute

layout(local_size_x = 32, local_size_y = 32) in;

layout(rgba8, binding = 0) uniform readonly image2D uInput;
layout(rgba8, binding = 1) uniform writeonly image2D uOutput;

void main(void)
{
	vec4 texel;
	ivec2 p = ivec2(gl_GlobalInvocationID.xy);
	texel = imageLoad(uInput, p);
	imageStore(uOutput, p, texel);
}
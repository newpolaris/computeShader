-- Compute

layout(local_size_x = 32, local_size_y = 32) in;

layout(rgba8, binding = 0) uniform writeonly image2D uOutput;

void main(void)
{
	vec4 texel;
	ivec2 p = ivec2(gl_GlobalInvocationID.xy);
	imageStore(uOutput, p, vec4(1, 0, 0, 1));
}
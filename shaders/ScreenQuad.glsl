-- Vertex

// IN
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexcoords;

// Out
out vec2 vTexcoords;

void main()
{
	vTexcoords = inTexcoords;
	gl_Position = vec4(inPosition, 1.0);
}

-- Fragment

// IN
in vec2 vTexcoords;

// OUT
layout(location = 0) out vec4 fragColor;

uniform sampler2D uTexSource;

// ----------------------------------------------------------------------------
void main() 
{
  	vec3 color = texture( uTexSource, vTexcoords).rgb; 
    fragColor = vec4(color, 1.0);
}
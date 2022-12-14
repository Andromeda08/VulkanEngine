#version 460

layout (set = 0, binding = 0) uniform Camera
{
    mat4 view;
    mat4 projection;
    mat4 view_inverse;
    mat4 proj_inverse;
    vec4 camera_pos;
} ubo;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec2 inUV;

layout (location = 4) in vec3 instanceTranslate;
layout (location = 5) in vec3 instanceColor;

layout (location = 0) out vec3 fragPos;
layout (location = 1) out vec3 fragColor;
layout (location = 2) out vec3 fragNormal;
layout (location = 3) out vec2 fragUV;

mat4 translate(vec3 delta)
{
    return mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(delta, 1.0)
    );
}

void main()
{
    gl_Position = ubo.projection * ubo.view * translate(instanceTranslate) * vec4(inPosition, 1.0);
    fragPos     = (translate(instanceTranslate) * vec4(inPosition, 1.0)).xyz;
    fragColor   = instanceColor;
    fragNormal  = inNormal;
    fragUV      = inUV;
}

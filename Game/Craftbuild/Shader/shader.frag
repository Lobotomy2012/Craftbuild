#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D tex_sampler[12];

layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec2 frag_tex_coord;
layout(location = 2) flat in float frag_tex_id;

layout(location = 0) out vec4 out_color;

void main() {
    int tex_index = int(frag_tex_id + 0.5);

    if (tex_index < 0 || tex_index > 13) {
        out_color = vec4(1.0, 0.0, 1.0, 1.0);
        return;
    }

    vec4 tex_color = texture(tex_sampler[tex_index], frag_tex_coord);

    if (tex_color.a < 0.05) {
        discard;
    }

    out_color = tex_color * vec4(frag_color, tex_color.a);
}
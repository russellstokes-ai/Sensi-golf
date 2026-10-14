shader_type canvas_item;
// Place ONLY on water-masked surfaces; never on whole course texture or colliders.
uniform vec2 visual_flow_direction = vec2(1.0, 0.25);
uniform float speed_texels_per_second : hint_range(0.0, 8.0) = 1.2;
uniform float presentation_seconds = 0.0;
void fragment() {
    vec2 dir = normalize(visual_flow_direction + vec2(0.00001, 0.0));
    vec2 shifted = dir * floor(presentation_seconds * speed_texels_per_second) * TEXTURE_PIXEL_SIZE;
    COLOR = texture(TEXTURE, fract(UV + shifted)) * COLOR;
}

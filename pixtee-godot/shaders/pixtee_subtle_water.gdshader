shader_type canvas_item;

// Put ONLY on Pixtee-authored water-surface sprites or masked Polygon2D.
// UV shifts in EXACT texel increments; NEVER resizes an approved source image.
// Water motion is *cosmetic only*, unrelated to native hazard physics.
uniform vec2 flow_direction = vec2(1.0, 0.0);
uniform float flow_texels_per_second : hint_range(0.0, 4.0) = 0.35;
uniform float shimmer_strength : hint_range(0.0, 0.12) = 0.025;
uniform float quality_enabled : hint_range(0.0, 1.0) = 1.0;

void fragment() {
    vec2 pixel_size = TEXTURE_PIXEL_SIZE;
    vec2 drift = normalize(flow_direction + vec2(0.0001, 0.0))
        * floor(TIME * flow_texels_per_second) * pixel_size * quality_enabled;
    vec2 sampled_uv = UV + drift;
    // Clamp inside original approved sprite edges, avoid bleed onto grass.
    sampled_uv = clamp(sampled_uv, pixel_size * 0.5, vec2(1.0) - pixel_size * 0.5);
    vec4 pixel = texture(TEXTURE, sampled_uv);
    float slow_glint = sin(TIME * 0.32 + floor(UV.y / max(pixel_size.y, 0.00001)) * 0.10);
    pixel.rgb *= 1.0 + slow_glint * shimmer_strength * quality_enabled;
    COLOR = pixel * COLOR;
}

shader_type canvas_item;
uniform int part = 1;
uniform vec2 body_centre = vec2(0.64,0.55);
uniform vec2 body_extent = vec2(0.23,0.13);
void fragment() {
    vec4 pixel = texture(TEXTURE,UV);
    bool body = length((UV-body_centre)/body_extent) <= 1.0;
    if ((part == 1 && !body) || (part == 2 && body)) pixel.a = 0.0;
    COLOR = pixel;
}

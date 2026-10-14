shader_type canvas_item;
uniform vec4 atlas_region = vec4(0.0,0.0,1.0,1.0);
uniform vec2 head_centre = vec2(0.64,0.25);
uniform int part = 1;
void fragment() {
    vec4 pixel = texture(TEXTURE,UV);
    vec2 cell = (UV-atlas_region.xy)/atlas_region.zw;
    vec2 centre = head_centre;
    bool head = length((cell-centre)/vec2(0.29,0.31)) < 1.0;
    if ((part == 1 && head) || (part == 2 && !head)) pixel.a = 0.0;
    COLOR = pixel;
}

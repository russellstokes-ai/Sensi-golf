shader_type canvas_item;
uniform vec4 atlas_region = vec4(0.0,0.0,1.0,1.0);
uniform bool facing_right = true;
uniform int part = 1;
void fragment() {
    vec4 pixel = texture(TEXTURE,UV);
    vec2 cell = (UV-atlas_region.xy)/atlas_region.zw;
    vec2 centre = vec2(facing_right ? 0.64 : 0.36,0.25);
    bool head = length((cell-centre)/vec2(0.29,0.31)) < 1.0;
    if ((part == 1 && head) || (part == 2 && !head)) pixel.a = 0.0;
    COLOR = pixel;
}

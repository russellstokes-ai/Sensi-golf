shader_type canvas_item;
uniform vec4 shirt_colour : source_color;
uniform vec4 trouser_colour : source_color;
uniform vec4 cap_colour : source_color;
uniform bool use_atlas_rect = false;
uniform vec2 atlas_origin = vec2(0.0);
uniform vec2 atlas_extent = vec2(1.0);
uniform int layer_part = 0;
uniform vec2 atlas_grid = vec2(6.0,4.0);
uniform bool shirt_enabled = false;
uniform bool trouser_enabled = false;
uniform bool cap_enabled = false;
varying vec4 vertex_tint;
void vertex() { vertex_tint = COLOR; }
void fragment() {
    vec4 pixel = texture(TEXTURE,UV);
    vec2 cell = use_atlas_rect ? (UV-atlas_origin)/atlas_extent : fract(UV*atlas_grid);
    // Cosmetic palette masks retain the source alpha, shading and silhouette.
    if (shirt_enabled && pixel.r>pixel.g*2.4 && pixel.r>pixel.b*2.0 && pixel.r>0.18)
        pixel.rgb=shirt_colour.rgb*pixel.r;
    else if (trouser_enabled && cell.y>0.38 && pixel.b>pixel.r*1.5 && pixel.b>pixel.g*1.15)
        pixel.rgb=trouser_colour.rgb*pixel.b;
    else if (cap_enabled && cell.y<0.28 && cell.x>0.37 && cell.x<0.68 && pixel.b>pixel.r*0.95 && pixel.g>0.2)
        pixel.rgb=cap_colour.rgb*max(pixel.r,max(pixel.g,pixel.b));
    if (layer_part == 1 && cell.y < 0.58) pixel.a=0.0;
    if (layer_part == 2 && cell.y >= 0.58) pixel.a=0.0;
    COLOR=pixel*vertex_tint;
}

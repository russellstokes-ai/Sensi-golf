shader_type canvas_item;
uniform vec4 shirt_colour : source_color;
uniform vec4 trouser_colour : source_color;
uniform vec4 cap_colour : source_color;
uniform vec4 skin_colour : source_color = vec4(1.0);
uniform vec4 hair_colour : source_color = vec4(1.0);
uniform bool skin_enabled = false;
uniform bool hair_enabled = false;
uniform bool use_atlas_rect = false;
uniform vec2 atlas_origin = vec2(0.0);
uniform vec2 atlas_extent = vec2(1.0);
uniform int layer_part = 0;
uniform int face_style = 0;
uniform int beard_style = 0;
uniform int hairstyle_style = 0;
uniform vec4 face_region = vec4(0.0);

uniform vec2 atlas_grid = vec2(6.0,4.0);
uniform bool shirt_enabled = false;
uniform bool trouser_enabled = false;
uniform bool cap_enabled = false;
varying vec4 vertex_tint;
void vertex() { vertex_tint = COLOR; }
void fragment() {
    vec4 pixel = texture(TEXTURE,UV);
    vec3 source_rgb = pixel.rgb;
    vec2 cell = use_atlas_rect ? (UV-atlas_origin)/atlas_extent : fract(UV*atlas_grid);
    // Cosmetic palette masks retain the source alpha, shading and silhouette.
    if (shirt_enabled && pixel.r>pixel.g*3.5 && pixel.r>pixel.b*3.0 && pixel.r>0.25)
        pixel.rgb=shirt_colour.rgb*pixel.r;
    else if (trouser_enabled && cell.y>0.38 && pixel.b>pixel.r*1.5 && pixel.b>pixel.g*1.15)
        pixel.rgb=trouser_colour.rgb*pixel.b;
    else if (cap_enabled && cell.y<0.28 && cell.x>0.37 && cell.x<0.68 && pixel.b>pixel.r*0.95 && pixel.g>0.2)
        pixel.rgb=cap_colour.rgb*max(pixel.r,max(pixel.g,pixel.b));
    // Warm source pigments only: preserve outlines, eyes, glove and clothing.
    bool warm = source_rgb.r > source_rgb.g*1.3 && source_rgb.g > source_rgb.b*1.35 && source_rgb.g > 0.06 && source_rgb.r < source_rgb.g*3.5;
    bool hair = warm && cell.y < 0.50 && source_rgb.g < 0.29;
    if (hair_enabled && hair)
        pixel.rgb = hair_colour.rgb * clamp(source_rgb.r/0.55,0.35,1.15);
    else if (skin_enabled && warm && !hair && source_rgb.g >= 0.29)
        pixel.rgb = skin_colour.rgb * clamp(source_rgb.r/0.90,0.35,1.1);
    // Per-pose authored face regions restrict details to exposed facial skin.
    vec2 face_uv = (cell-face_region.xy)/max(face_region.zw,vec2(0.0001));
    bool facial_skin = face_region.z > 0.0 && warm && !hair && all(greaterThanEqual(face_uv,vec2(0.0))) && all(lessThanEqual(face_uv,vec2(1.0)));
    if (facial_skin) {
        if (face_style == 1) {
            bool freckles = length((face_uv-vec2(0.26,0.56))/vec2(0.075,0.045)) < 1.0 || length((face_uv-vec2(0.73,0.56))/vec2(0.075,0.045)) < 1.0;
            if (freckles) pixel.rgb *= 0.64;
        } else if (face_style == 2) {
            float cheek = max(0.0,1.0-length((face_uv-vec2(0.25,0.59))/vec2(0.20,0.17))) + max(0.0,1.0-length((face_uv-vec2(0.74,0.59))/vec2(0.20,0.17)));
            pixel.rgb = mix(pixel.rgb,vec3(0.72,0.23,0.20),min(0.24,cheek*0.24));
        }
        if (beard_style > 0 && face_uv.y > (beard_style == 1 ? 0.76 : 0.64)) {
            float shade = beard_style == 1 ? 0.34 : 0.83;
            pixel.rgb = mix(pixel.rgb,hair_colour.rgb*0.72,shade);
        }
        if (hairstyle_style == 1 && face_uv.y < 0.14+sin(face_uv.x*15.0)*0.04)
            pixel.rgb = hair_colour.rgb*0.85;
    }
    if (layer_part == 1 && cell.y < 0.58) pixel.a=0.0;
    if (layer_part == 2 && cell.y >= 0.58) pixel.a=0.0;
    COLOR=pixel*vertex_tint;
}

shader_type canvas_item;
uniform float presentation_time = 0.0;
uniform float ready = 0.0;
uniform float transition = 0.0;
uniform float motion_enabled = 1.0;
void fragment() {
    float edge = min(min(UV.x,1.0-UV.x),min(UV.y,1.0-UV.y));
    float halo = 1.0-smoothstep(0.0,0.14,edge);
    float breathe = 0.55+0.20*sin(presentation_time*2.4)*motion_enabled;
    COLOR = vec4(1.0,0.78,0.25,halo*(ready*breathe*0.20+transition*0.18));
}

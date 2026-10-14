shader_type canvas_item;
uniform float visual_time = 0.0;
uniform float sun_strength = 0.65;
uniform float fog_density = 0.0;
uniform float cloud_cover = 0.4;
void fragment() {
    vec2 p = UV*vec2(544.0,912.0);
    float t = visual_time*0.012;
    float mist = 0.5+0.20*sin(p.x*0.019+p.y*0.009-t)+0.17*sin(p.x*0.010-p.y*0.016+t*0.6);
    mist = smoothstep(0.30,0.79,mist);
    float shaft = pow(max(0.0,0.5+0.5*sin(p.x*0.026+p.y*0.015+t*0.25)),9.0);
    float haze = fog_density*(0.028+0.10*mist);
    float light = sun_strength*(1.0-cloud_cover*0.45)*(0.014+0.055*shaft);
    float alpha = clamp(haze+light,0.0,0.16);
    vec3 tint = mix(vec3(1.0,0.93,0.72),vec3(0.80,0.87,0.90),haze/max(alpha,0.0001));
    COLOR = vec4(tint,alpha);
}

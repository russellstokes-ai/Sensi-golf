shader_type canvas_item;
uniform float visual_time = 0.0;
uniform float sun_strength = 0.65;
uniform float fog_density = 0.0;
uniform float cloud_cover = 0.4;
uniform vec2 wind_direction = vec2(0.95,0.3);
uniform vec4 focus_points = vec4(256.0,850.0,364.0,168.0);
float hash_point(vec2 p) { return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453); }
float noise(vec2 p) {
 vec2 c=floor(p), f=fract(p); f=f*f*(3.0-2.0*f);
 return mix(mix(hash_point(c),hash_point(c+vec2(1,0)),f.x),mix(hash_point(c+vec2(0,1)),hash_point(c+vec2(1)),f.x),f.y);
}
float field(vec2 p) { return noise(p)*0.62+noise(p*2.03+vec2(17,41))*0.27+noise(p*4.09+23.0)*0.11; }
void fragment() {
 vec2 p=UV*vec2(544,912); float t=visual_time;
 float cloud=smoothstep(0.38,0.69,field((p-wind_direction*t*0.44)/175.0));
 float shade=cloud*cloud_cover*0.15;
 float edge=smoothstep(55.0,160.0,abs(p.x-272.0));
 float mist=smoothstep(0.37,0.68,field((p-vec2(t*0.57,-t*0.12))/vec2(130,78)));
 float back=smoothstep(0.41,0.73,field((p-vec2(-t*0.23,t*0.1)+97.0)/vec2(180,100)));
 float focus=min(smoothstep(10.0,28.0,distance(p,focus_points.xy)),smoothstep(10.0,28.0,distance(p,focus_points.zw)));
 float haze=fog_density*(mist*0.095+back*0.04)*(0.35+edge*0.65)*focus;
 float beams=pow(smoothstep(0.46,0.79,field(vec2(p.x*0.007+p.y*0.006,p.y*0.0017)+vec2(t*0.001,5))),2.0);
 float sunlight=sun_strength*(1.0-cloud_cover*0.65)*(0.008+beams*0.065)*focus;
 float alpha=clamp(shade+haze+sunlight,0.0,0.22);
 vec3 tint=(vec3(0.045,0.10,0.105)*shade+vec3(0.83,0.9,0.92)*haze+vec3(1.0,0.93,0.69)*sunlight)/max(alpha,0.0001);
 COLOR=vec4(tint,alpha);
}

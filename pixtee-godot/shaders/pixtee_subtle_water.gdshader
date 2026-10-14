shader_type canvas_item;
uniform vec2 flow_direction=vec2(1,0);
uniform float flow_texels_per_second=0.35;
uniform float shimmer_strength=0.025;
uniform float quality_enabled=1.0;
uniform float presentation_time=0.0;
uniform float sun_strength=0.65;
uniform bool repeat_surface=false;
uniform bool use_flow_map=false;
uniform sampler2D bank_mask : filter_linear, repeat_disable;
uniform vec2 spark_positions[14];
uniform int spark_count=4;
varying vec2 world_position;
varying vec2 stream_direction;
varying vec4 sprite_tint;
void vertex() {
 world_position=VERTEX;
 stream_direction=use_flow_map ? COLOR.rg*2.0-1.0 : flow_direction;
 sprite_tint=use_flow_map ? vec4(1,1,1,COLOR.a) : COLOR;
}
void fragment() {
 vec2 direction=normalize(stream_direction+vec2(0.0001,0));
 float interior=use_flow_map ? texture(bank_mask,world_position/vec2(544,912)).r : 1.0;
 vec2 drift=direction*floor(presentation_time*flow_texels_per_second)*TEXTURE_PIXEL_SIZE*quality_enabled*interior;
 vec2 uv=UV+drift;
 uv=repeat_surface ? fract(uv) : clamp(uv,TEXTURE_PIXEL_SIZE*0.5,vec2(1)-TEXTURE_PIXEL_SIZE*0.5);
 vec4 pixel=texture(TEXTURE,uv);
 vec4 second=texture(TEXTURE,fract(UV*0.73+vec2(0.31,0.67)-drift*0.35));
 pixel.rgb=mix(pixel.rgb,second.rgb,0.16*interior*quality_enabled);
 float ripple=sin(world_position.x*0.19+world_position.y*0.12-presentation_time*0.8);
 float reflection=(0.5+0.5*sin(world_position.x*0.011+world_position.y*0.02+sin(presentation_time*0.07)))*interior;
 pixel.rgb=mix(pixel.rgb,vec3(0.55,0.71,0.77),reflection*0.07);
 pixel.rgb*=1.0+ripple*shimmer_strength*quality_enabled;
 float sparkle=0.0;
 for(int i=0;i<14;i++) {
  if(i>=spark_count) break;
  float age=mod(presentation_time+float(i)*0.63,4.0);
  float envelope=smoothstep(0.0,0.25,age)*(1.0-smoothstep(0.45,0.85,age));
  vec2 d=abs(world_position-spark_positions[i]);
  float core=(1.0-smoothstep(0.2,0.8,d.x))*(1.0-smoothstep(0.2,0.8,d.y));
  float rays=max((1.0-smoothstep(0.0,0.25,d.x))*(1.0-smoothstep(0.2,1.8,d.y)),(1.0-smoothstep(0.0,0.25,d.y))*(1.0-smoothstep(0.2,1.8,d.x)));
  sparkle+=max(core,rays*0.6)*envelope;
 }
 pixel.rgb+=vec3(0.88,0.95,0.9)*sparkle*sun_strength*interior;
 COLOR=pixel*sprite_tint;
}

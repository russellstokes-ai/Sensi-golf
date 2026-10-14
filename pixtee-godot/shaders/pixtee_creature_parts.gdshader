shader_type canvas_item;
uniform vec4 atlas_region=vec4(0,0,1,1);
uniform int part=1;
uniform bool squirrel=false;
uniform bool frog=false;
varying vec4 sprite_tint;
void vertex() {sprite_tint=COLOR;}
void fragment() {
 vec4 pixel=texture(TEXTURE,UV);
 vec2 p=(UV-atlas_region.xy)/atlas_region.zw;
 bool tail=squirrel && length((p-vec2(0.25,0.6))/vec2(0.26,0.24))<1.0;
 bool limbs=squirrel ? (length((p-vec2(0.79,0.48))/vec2(0.20,0.13))<1.0 || length((p-vec2(0.77,0.76))/vec2(0.18,0.14))<1.0) : p.y<0.43;
 int section=tail?3:limbs?2:1;
 if(frog){
  bool throat=length((p-vec2(0.63,0.55))/vec2(0.13,0.1))<1.0;
  bool feet=p.y>0.61 && p.x<0.56;
  section=throat?3:feet?2:1;
 }
 if(part!=section)pixel.a=0.0;
 if(pixel.a<0.1)pixel.a=0.0;
 COLOR=pixel*sprite_tint;
}

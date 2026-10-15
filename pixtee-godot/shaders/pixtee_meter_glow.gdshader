shader_type canvas_item;
uniform vec2 control_size = vec2(360.0,64.0);
uniform float presentation_time = 0.0;
uniform float ready = 0.0;
uniform float phase = 0.0;
uniform float transition = 0.0;
uniform float motion_enabled = 1.0;
uniform vec3 target = vec3(0.86777,0.03967,0.05289);
uniform float rain = 0.0;
uniform float fog = 0.0;
uniform float golden = 0.0;
uniform float marker = 0.86777;
float capsule(vec2 p, vec2 half_size, float r) {
    vec2 q = abs(p) - half_size + r;
    return length(max(q,vec2(0.0))) + min(max(q.x,q.y),0.0) - r;
}
void fragment() {
    vec2 p = UV * control_size;
    float width = max(1.0,control_size.x-24.0);
    vec2 centre = vec2(control_size.x*0.5,34.0);
    float d = capsule(p-centre,vec2(width*0.5,12.0),11.0);
    float u = clamp((p.x-12.0)/width,0.0,1.0);
    float t = presentation_time;
    float pulse = 0.55+0.20*sin(t*2.6)*motion_enabled;
    float hot = smoothstep(target.x-target.y-target.z-.035,target.x-target.y-target.z+.015,u);
    vec3 edge = mix(vec3(.04,.68,1.0),vec3(1.0,.85,.10),hot);
    edge = mix(edge,vec3(1.0,.63,.20),golden*.30);
    float aura = exp(-max(d,0.0)*.30)*(.26+ready*pulse*.32+transition*.18);
    float rim = exp(-abs(d)*1.8);
    float inside = 1.0-smoothstep(-.5,.5,d);
    vec3 body = mix(vec3(.025,.09,.16),vec3(.035,.17,.23),u);
    body += vec3(.035,.07,.10)*exp(-pow((p.y-26.0)/8.0,2.0));
    float yellow = step(abs(u-target.x),target.y+target.z);
    float red = step(abs(u-target.x),target.y);
    body = mix(body,mix(vec3(1.0,.92,.04),vec3(1.0,.56,.02),smoothstep(target.x,1.0,u)),yellow);
    body = mix(body,vec3(1.0,.015,.025),red);
    // Travelling charge ribbons live inside the blue region; timing remains native.
    float flow = exp(-pow((p.y-34.0-sin(u*10.0+t*3.0)*5.0)/1.4,2.0));
    float energy = (0.5+0.5*sin(u*21.0+t*5.0));
    body += vec3(.02,.37,.65)*flow*energy*float(phase==1.0)*(1.0-yellow)*motion_enabled;
    float top_glint = exp(-abs(p.y-23.3)*1.3)*.18;
    body += edge*top_glint;
    float vapour = fog * (0.5+0.5*sin(p.x*.032+t*.22+sin(p.x*.067-t*.31))) * exp(-pow((p.y-27.0)/16.0,2.0))*.15;
    float end_mask = smoothstep(0.0,16.0,p.x)*smoothstep(0.0,16.0,control_size.x-p.x);
    vapour *= end_mask;
    float near_target = 1.0-smoothstep(target.y,target.y+.025,abs(marker-target.x));
    float spark = exp(-abs(p.x-(12.0+target.x*width))*1.2) * exp(-abs(p.y-34.0)*.055) * near_target * float(phase==2.0) * motion_enabled * .8;
    float alpha = max(inside,max(aura+rim*.65+spark,vapour));
    vec3 colour = mix(edge,body,inside);
    colour = mix(colour,edge+vec3(.22),rim*.8);
    colour = mix(colour,vec3(.7,.88,.94),vapour*2.0);
    colour += vec3(1.0,.58,.12)*spark;
    COLOR = vec4(colour,clamp(alpha,0.0,1.0));
}

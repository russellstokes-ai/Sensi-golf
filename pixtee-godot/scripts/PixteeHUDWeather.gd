extends Control
## Layered miniature landscape. All motion is cosmetic; lie comes from native data.
var weather := {"sun":0.8,"cloud":0.2,"rain":0.0,"fog":0.0,"wind":0.3}
var preset := "sunny"
var clock := 0.0
var lie := {"surface":3,"tee":true,"occlusion":0.0}
var ball: Texture2D

func _ready() -> void:
	mouse_filter = Control.MOUSE_FILTER_IGNORE
	clip_contents = true
	texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	ball = load("res://courses/lakewood-hole-1/visual/ball.png")

func cloud(at: Vector2, extent: Vector2, tint: Color) -> void:
	# Stepped pixel silhouette, shaded underside; independent drifting layers.
	var points := PackedVector2Array([Vector2(0,.65),Vector2(.10,.65),Vector2(.10,.38),Vector2(.26,.38),Vector2(.26,.17),Vector2(.48,.17),Vector2(.48,0),Vector2(.64,0),Vector2(.64,.21),Vector2(.80,.21),Vector2(.80,.47),Vector2(.94,.47),Vector2(.94,.65),Vector2(1,.65),Vector2(1,1),Vector2(0,1)])
	for i in points.size(): points[i] = (at+points[i]*extent).floor()
	draw_colored_polygon(points,tint)
	draw_rect(Rect2(at+Vector2(extent.x*.10,extent.y*.82),Vector2(extent.x*.84,extent.y*.18)),tint.darkened(.14))

func blade(at: Vector2, height: float, seed: float, colour: Color) -> void:
	var bend := sin(clock*1.8+seed)*float(weather.get("wind",0.3))*2.1
	draw_colored_polygon(PackedVector2Array([at+Vector2(-1,0),at+Vector2(bend-1,-height*.6),at+Vector2(bend+1,-height),at+Vector2(2,0)]),colour)

func _draw() -> void:
	var rain := float(weather.get("rain",0.0))
	var fog := float(weather.get("fog",0.0))
	var cover := float(weather.get("cloud",0.2))
	var golden := preset in ["golden_hour","sunset"]
	var horizon := size.y*.63
	var top := Color("238bd0").lerp(Color("253c59"),cover*.72)
	var bottom := Color("a6e6f4").lerp(Color("80999e"),cover*.8)
	if golden: top = Color("635182"); bottom = Color("ffb052")
	for row in range(0,int(horizon)+2,2):
		draw_rect(Rect2(0,row,size.x,2),top.lerp(bottom,float(row)/horizon))
	var sun_at := Vector2(size.x*.76,18.0+sin(clock*.13)*.5)
	if cover<.8:
		for radius in [15.0,11.0,8.0]:
			draw_circle(sun_at,radius,Color(1,.86,.37,(1.0-cover)*.06),true,-1,false)
		draw_circle(sun_at,5,Color("ffed9b"),true,-1,false)
		for ray in 8:
			var dir := Vector2.from_angle(ray*TAU/8)
			draw_line((sun_at+dir*7).round(),(sun_at+dir*(8.5+sin(clock*1.2+ray)*.5)).round(),Color("ffe38b"),1)
	for layer in 2:
		for i in 5:
			var width := 30.0+fmod(i*17+layer*13,32)
			var x := fposmod(i*51.0+layer*19.0+clock*(1.1+layer*.7+float(weather.get("wind",.3))*2),size.x+70)-60
			var y := 6.0+fmod(i*7+layer*13,17)
			var tint := Color("e5f3ef").lerp(Color("718396"),cover*.8+layer*.08)
			tint.a = .55+layer*.2
			cloud(Vector2(x,y),Vector2(width,10+layer*5),tint)
	# Distant treeline has small parallax-free sway; never moves the ground.
	for i in range(0,int(size.x)+8,5):
		var height := 3+int(fmod(i*7,9))
		var tint := Color("407664").lerp(Color("303f53"),cover*.6)
		if golden: tint = Color("394c48")
		var x := float(i)
		draw_colored_polygon(PackedVector2Array([Vector2(x-3,horizon),Vector2(x-2,horizon-4),Vector2(x-4,horizon-4),Vector2(x,horizon-height-3),Vector2(x+4,horizon-4),Vector2(x+2,horizon-4),Vector2(x+4,horizon)]),tint)
		draw_line(Vector2(x,horizon-3),Vector2(x,horizon+1),tint.darkened(.15),1)
	var surface := int(lie.get("surface",3))
	var sand := surface==7
	var rough := surface in [5,6]
	var ground := Color("c8a569") if sand else Color("4b842a") if rough else Color("64a435")
	ground = ground.lerp(Color("324e43"),rain*.25)
	draw_rect(Rect2(0,horizon,size.x,size.y-horizon),ground)
	for i in 170:
		var at := Vector2(fmod(i*37.0,size.x),horizon+fmod(i*13.7,maxf(1,size.y-horizon)))
		draw_rect(Rect2(at.floor(),Vector2(2,1)),ground.lightened(.12) if i%3==0 else ground.darkened(.08))
	var centre := Vector2(size.x*.28,size.y-10)
	var diameter := 34.0
	var tee := bool(lie.get("tee",false))
	var bottom_y := centre.y-6 if tee else centre.y+float(lie.get("occlusion",0))
	draw_set_transform(centre+Vector2(1,1),0,Vector2(1,.23))
	draw_circle(Vector2.ZERO,15,Color(0.02,.08,.02,.34))
	draw_set_transform(Vector2.ZERO)
	if tee:
		draw_colored_polygon(PackedVector2Array([Vector2(centre.x-4,bottom_y-1),Vector2(centre.x+4,bottom_y-1),Vector2(centre.x+1,centre.y+2),Vector2(centre.x-1,centre.y+2)]),Color("c88e3d"))
		draw_line(Vector2(centre.x-3,bottom_y),Vector2(centre.x+3,bottom_y),Color("ffe3a1"),2)
	if ball:
		var dims := ball.get_size()*(diameter/ball.get_height())
		draw_texture_rect(ball,Rect2(Vector2(centre.x-dims.x/2,bottom_y-diameter),dims),false)
	# Foreground genuinely occludes the ball; no shrinking or squashing the sphere.
	if sand:
		var points := PackedVector2Array([centre+Vector2(-22,0),centre+Vector2(-14,-3),centre+Vector2(-8,0),centre+Vector2(0,1),centre+Vector2(10,-1),centre+Vector2(20,-3),centre+Vector2(25,7),centre+Vector2(-24,7)])
		draw_colored_polygon(points,ground.lightened(.15))
		for i in 30:
			draw_rect(Rect2(centre+Vector2(fmod(i*11.3,42)-21,fmod(i*3.1,6)),Vector2(1,1)),ground.darkened(.15) if i%2 else ground.lightened(.3))
	elif rough:
		for i in 24:
			var at := centre+Vector2(i*2-23,3+fmod(i*3,4))
			blade(at,5+fmod(i*7,9)+(3 if surface==6 else 0),i*.7,Color("99b631") if i%3==0 else Color("527b24") if i%2==0 else Color("3b611e"))
	else:
		for i in 25:
			blade(Vector2(i*size.x/24,size.y),3+fmod(i*5,4),i,ground.lightened(.13))
	# Weather remains behind the card's data; only this clipped sky/lie miniature gets rain.
	if rain>.01:
		for i in int(12+rain*24):
			var at := Vector2(fposmod(i*31.7-clock*13,size.x+10)-5,fposmod(i*17.3+clock*77,size.y+12)-8)
			draw_line(at,at+Vector2(-2,5),Color(.7,.86,1,rain*.46),1)
	if fog>.01:
		for i in 4:
			var at := Vector2(fposmod(i*57+clock*(1.2+i*.25),size.x+75)-65,22+i*7)
			cloud(at,Vector2(80,10),Color(.83,.9,.89,fog*.14))
	if preset=="breezy" or golden:
		for i in 3:
			var at := Vector2(fposmod(i*64+clock*12,size.x+12)-6,12+sin(clock+i*2)*5+i*4)
			draw_set_transform(at,sin(clock+i)*.6)
			draw_colored_polygon(PackedVector2Array([Vector2(-3,0),Vector2(0,-2),Vector2(4,0),Vector2(0,2)]),Color("e5b443") if golden else Color("d6c24c"))
			draw_set_transform(Vector2.ZERO)

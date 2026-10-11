package com.pixtee.golf

import kotlin.math.atan2

/**
 * Live opening-screen golfers run real independent PixteeCore shots.
 *
 * Unlike a looping decorative animation, each autonomous player uses the
 * normal three-click swing, ball flight, landing, green slope and collisions.
 * These actors never modify the actual human player's round, scores or saves.
 * Rendering requires approved production golfer assets: NO fake sprites.
 */
class MenuAttractMode(
    val hole: HoleLayout = PixteeCourseCatalog.hole(0,4)
) {
    data class Actor(val player: PixteeCore, val index: Int, var idleTicks: Int = 0,
                     var powerTicks: Int = 0, var accuracyTicks: Int = 0)

    private fun makeActor(i: Int): Actor {
        val player=PixteeCore()
        player.startHole(hole)
        val y=when(i) {
            0 -> hole.teeY
            1 -> hole.pinY+(hole.teeY-hole.pinY)*0.43f
            else -> hole.pinY+18f
        }
        val x=when(i) {
            0 -> hole.teeX
            1 -> hole.fairwayCentre(y)
            else -> hole.pinX+12f
        }
        val club=when(i) { 0->0; 1->8; else->12 }
        val aim=Math.toDegrees(atan2(
            (hole.pinX-x).toDouble(),(y-hole.pinY).toDouble()
        )).toFloat().coerceIn(-85f,85f)
        player.restoreBall(StableBall(x,y,0,0,0,club,aim))
        return Actor(player,i,idleTicks = i*34)
    }

    val actors: List<Actor> = (0..2).map(::makeActor)
    var ticks: Long = 0L
        private set

    fun tick() {
        ticks++
        actors.forEach { actor ->
            val p=actor.player
            when(p.stage) {
                GameStage.READY -> {
                    actor.idleTicks++
                    if(actor.idleTicks>=70+(actor.index*11)) {
                        actor.idleTicks=0
                        actor.powerTicks=0
                        p.whack()
                    }
                }
                GameStage.POWER -> {
                    p.tick()
                    actor.powerTicks++
                    // Vary power in a documented reproducible cycle. Third
                    // party advertisements cannot affect shot selection.
                    val target=when(actor.index) { 0->32; 1->25; else->18 }
                    if(actor.powerTicks>=target) {
                        actor.accuracyTicks=0
                        p.whack()
                    }
                }
                GameStage.ACCURACY -> {
                    p.tick()
                    actor.accuracyTicks++
                    if(p.stage==GameStage.ACCURACY && actor.accuracyTicks>=16)
                        p.whack()
                }
                GameStage.FLIGHT, GameStage.ROLL -> p.tick()
                GameStage.HOLED -> resetActor(actor)
            }
            if(p.stage==GameStage.READY && p.strokes>=3) resetActor(actor)
        }
    }

    private fun resetActor(actor: Actor) {
        val fresh=makeActor(actor.index)
        actor.player.startHole(hole)
        val state=fresh.player.stableBall()
            ?: error("Demo golfer must reset to an address position")
        actor.player.restoreBall(state)
        actor.idleTicks=actor.index*17
        actor.powerTicks=0
        actor.accuracyTicks=0
    }

    fun trackedBallPositions(): List<Pair<Float,Float>> =
        actors.map { it.player.x to it.player.y }
}

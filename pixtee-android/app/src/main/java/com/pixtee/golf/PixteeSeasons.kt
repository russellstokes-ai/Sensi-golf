package com.pixtee.golf

/**
 * Replayable OFFLINE tour career after (and alongside) five introductory tiers.
 *
 * Rival rounds are fixed deterministic CPU scores, never presented as online or
 * human participants. This mode cannot grant better ball physics or power.
 * No device clock or network dependence: a season can be finished at any pace.
 */
data class TourRival(val name: String, val toPar: Int)
data class TourEvent(
    val season: Int, val index: Int, val courseIndex: Int, val holes: Int,
    val rivals: List<TourRival>
) {
    init {
        require(season >= 1 && index in 0 until PixteeSeasons.EVENTS_PER_SEASON)
        require(courseIndex in PixteeCourseCatalog.courses.indices)
        require(holes in listOf(3,9,18))
        require(rivals.isNotEmpty())
    }
    val course: PixteeCourse get()=PixteeCourseCatalog.courses[courseIndex]

    /** Equal scores share a position; no fabricated tie-break shot data. */
    fun playerPosition(toPar: Int): Int = 1+rivals.count { it.toPar < toPar }
    fun tourPoints(toPar: Int): Int = when(playerPosition(toPar)) {
        1 -> 25
        2 -> 18
        3 -> 12
        4 -> 8
        else -> 5
    }
}
data class TourResult(val toPar: Int, val points: Int, val placing: Int)
data class TourSeason(
    val number: Int = 1,
    val results: List<TourResult> = emptyList(),
    val championships: Int = 0
) {
    init {
        require(number in 1..100000 && championships in 0..number)
        require(results.size<=PixteeSeasons.EVENTS_PER_SEASON)
        require(results.all { it.points in setOf(5,8,12,18,25) && it.placing in 1..5 &&
            it.toPar in -54..216 })
    }
    val finished: Boolean get()=results.size==PixteeSeasons.EVENTS_PER_SEASON
    val points: Int get()=results.sumOf { it.points }
    val next: TourEvent? get()=if(finished) null else
        PixteeSeasons.event(number,results.size)

    fun record(toPar: Int): TourSeason {
        val event=next ?: throw IllegalStateException("Tour season finished")
        val position=event.playerPosition(toPar)
        return copy(results=results+TourResult(toPar,event.tourPoints(toPar),position))
    }

    /** Optional subsequent season, preserving the career's accumulated titles. */
    fun advance(): TourSeason {
        check(finished) { "Cannot skip unfinished tour events" }
        // Championship requires first place by points in this offline tour.
        val rivalsTotal=PixteeSeasons.rivalSeasonTotals(number)
        val champion=points>=rivalsTotal.maxOrNull()!!
        return TourSeason(number+1,emptyList(),championships+if(champion) 1 else 0)
    }
}

object PixteeSeasons {
    const val EVENTS_PER_SEASON=12
    private val rivalNames=listOf("MAYA", "COOPER", "JULES", "REMI")
    fun event(season: Int, index: Int): TourEvent {
        require(season>=1 && index in 0 until EVENTS_PER_SEASON)
        // Calendar rotates through every course, no artificial day/week expiry.
        val courseIndex=(((season-1)*EVENTS_PER_SEASON.toLong()+index) %
            PixteeCourseCatalog.courses.size).toInt()
        val holes=when(index%4) { 0 -> 3; 1 -> 9; else -> 18 }
        val seed=season*29L+index*37L+courseIndex*11L
        val rivals=rivalNames.mapIndexed { i,name ->
            // Published as simulated benchmark scores, never live leaderboard.
            val baseline=((seed+i*41L)%7L).toInt()-3
            val modifier=if(holes==3) 1 else if(holes==9) 0 else -2
            TourRival(name,baseline+modifier-(i%2))
        }
        return TourEvent(season,index,courseIndex,holes,rivals)
    }
    fun rivalSeasonTotals(season: Int): List<Int> =
        (0 until 4).map { rival ->
            // Deterministic independent standing against other CPU rivals.
            (0 until EVENTS_PER_SEASON).map { index ->
                val game=event(season,index)
                val score=game.rivals[rival].toPar
                val place=1+game.rivals.count { it.toPar < score }
                when(place) { 1 -> 25; 2 -> 18; 3 -> 12; 4 -> 8; else -> 5 }
            }.sum()
        }
}

/** Tiny validated save separate from an active round's existing checksummed save. */
object PixteeSeasonCodec {
    private const val VERSION="PXTOUR1"
    fun encode(s: TourSeason): String =
        listOf(VERSION,s.number,s.championships,s.results.joinToString(";") {
            "${it.toPar},${it.points},${it.placing}"
        }).joinToString("|")

    fun decode(value: String?): TourSeason? {
        if(value==null || value.length>1500) return null
        return try {
            val p=value.split('|')
            if(p.size!=4 || p[0]!=VERSION) return null
            val results=if(p[3].isBlank()) emptyList() else p[3].split(';').map {
                val v=it.split(',')
                require(v.size==3)
                TourResult(v[0].toInt(),v[1].toInt(),v[2].toInt())
            }
            val decoded=TourSeason(p[1].toInt(),results,p[2].toInt())
            // Validate all stored points/places against fixed CPU opponents.
            if(results.anyIndexed { i,r ->
                val event=PixteeSeasons.event(decoded.number,i)
                event.tourPoints(r.toPar)!=r.points ||
                    event.playerPosition(r.toPar)!=r.placing
            }) null else decoded
        } catch (_: Exception) { null }
    }
    private inline fun <T> List<T>.anyIndexed(block:(Int,T)->Boolean): Boolean =
        this.withIndex().any { (i,value) -> block(i,value) }
}

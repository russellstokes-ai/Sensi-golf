package com.pixtee.golf

/**
 * Lifetime course mastery. Records are based on finished, actually played
 * 18-hole rounds; practice and shortened rounds never earn course medals.
 *
 * Pure gameplay data; does not introduce graphics or change ball physics.
 */
enum class MasteryMedal { UNPLAYED, BRONZE, SILVER, GOLD, PLATINUM }

data class CourseRecord(
    val completedRounds: Int,
    val bestRelativeToPar: Int,
    val bestPutts: Int,
    val bestPenalties: Int
) {
    init {
        require(completedRounds in 1..100000)
        require(bestRelativeToPar in -90..200)
        require(bestPutts in 0..216)
        require(bestPenalties in 0..216)
    }
    val medal: MasteryMedal get() = when {
        bestRelativeToPar <= -9 -> MasteryMedal.PLATINUM
        bestRelativeToPar <= 0 -> MasteryMedal.GOLD
        bestRelativeToPar <= 12 -> MasteryMedal.SILVER
        else -> MasteryMedal.BRONZE
    }
}

data class PixteeCourseMastery(val records: Map<Int, CourseRecord> = emptyMap()) {
    init {
        require(records.keys.all { it in PixteeCourseCatalog.courses.indices })
    }

    val masteredCourses: Int get() =
        records.values.count { it.medal==MasteryMedal.PLATINUM }
    val coursesPlayed: Int get() = records.size
    val totalCompletedRounds: Int get() = records.values.sumOf { it.completedRounds }

    fun medal(courseIndex: Int): MasteryMedal =
        records[courseIndex]?.medal ?: MasteryMedal.UNPLAYED

    /**
     * Return a new immutable progress snapshot, preserving existing records.
     * A caller must commit at the same moment it commits the real scorecard.
     */
    fun record(round: PixteeRound): PixteeCourseMastery {
        if (!round.isComplete || round.length!=18 ||
            round.mode==RoundMode.PRACTICE) return this

        val previous = records[round.courseIndex]
        val relative = round.relativeToPar
        val data = if (previous == null) {
            CourseRecord(1,relative,round.totalPutts,round.totalPenalties)
        } else {
            CourseRecord(
                (previous.completedRounds+1).coerceAtMost(100000),
                minOf(previous.bestRelativeToPar,relative),
                minOf(previous.bestPutts,round.totalPutts),
                minOf(previous.bestPenalties,round.totalPenalties)
            )
        }
        return copy(records=records+(round.courseIndex to data))
    }
}

/**
 * Strict offline persistence with a stable version and small data footprint.
 * Corrupted or unsupported records are rejected, never silently interpreted
 * as a different course or unlocked medal.
 */
object PixteeCourseMasteryCodec {
    private const val VERSION="PXMASTERY1"
    fun encode(mastery: PixteeCourseMastery): String {
        val records=mastery.records.toSortedMap().entries.joinToString(";") { (course,r) ->
            "${course},${r.completedRounds},${r.bestRelativeToPar},${r.bestPutts},${r.bestPenalties}"
        }
        return "$VERSION|$records"
    }
    fun decode(text: String?): PixteeCourseMastery? {
        if(text==null || text.length>3000) return null
        return try {
            val chunks=text.split('|')
            if(chunks.size!=2 || chunks[0]!=VERSION) return null
            if(chunks[1].isEmpty()) return PixteeCourseMastery()
            val rows=chunks[1].split(';')
            if(rows.size>PixteeCourseCatalog.courses.size) return null
            val values=rows.map {
                val p=it.split(',')
                require(p.size==5)
                p[0].toInt() to CourseRecord(
                    p[1].toInt(),p[2].toInt(),p[3].toInt(),p[4].toInt()
                )
            }
            if(values.map { it.first }.distinct().size != values.size) null
            else PixteeCourseMastery(values.toMap())
        } catch (_: Exception) { null }
    }
}

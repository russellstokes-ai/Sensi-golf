package com.pixtee.golf

import java.util.zip.CRC32

data class StableBall(val x: Float, val y: Float, val strokes: Int,
                      val penalties: Int, val putts: Int, val club: Int,
                      val aim: Float) {
    init {
        require(x in 0f..PixteeCore.WIDTH && y in 0f..PixteeCore.HEIGHT)
        require(strokes in 0..12 && penalties in 0..12 && putts in 0..12)
        require(club in 0..12 && aim in -85f..85f)
    }
}
data class SavedPixteeGame(val round: PixteeRound, val ball: StableBall?,
    val careerEventIndex: Int = -1)

/**
 * Strict, checksummed offline save. Only save settled positions: a backgrounded
 * mid-flight ball resumes at its last completed shot, never an arbitrary physics
 * half-frame. Invalid or partial persisted data is rejected, not applied.
 */
object PixteeSaveCodec {
    private const val MAGIC="PX3"
    private const val LEGACY_MAGIC="PX2"
    private fun checksum(data: String): String {
        val crc=CRC32()
        crc.update(data.toByteArray(Charsets.UTF_8))
        return crc.value.toString(16)
    }
    fun encode(round: PixteeRound, ball: StableBall?,
               careerEventIndex: Int = -1): String {
        require(careerEventIndex in -1..24)
        if (careerEventIndex >= 0) {
            require(round.mode == RoundMode.CAREER)
            val event=PixteeCareer.events[careerEventIndex]
            require(event.course==round.courseIndex && event.holes==round.length)
        }
        val holeRows=round.results.joinToString(";") {
            "${it.strokes},${it.penalties},${it.putts}"
        }
        val position=if(ball==null) "-" else listOf(ball.x,ball.y,ball.strokes,
            ball.penalties,ball.putts,ball.club,ball.aim).joinToString(",")
        val raw=listOf(MAGIC,round.courseIndex,round.length,
            round.mode.name,holeRows,position,careerEventIndex).joinToString("|")
        return raw+"|"+checksum(raw)
    }
    fun decode(value: String?): SavedPixteeGame? {
        if(value==null || value.length>4096) return null
        return try {
            val parts=value.split('|')
            val current=parts.size==8 && parts[0]==MAGIC
            val legacy=parts.size==7 && parts[0]==LEGACY_MAGIC
            if(!current && !legacy) return null
            val raw=parts.dropLast(1).joinToString("|")
            if(checksum(raw)!=parts.last()) return null
            val course=parts[1].toInt()
            val length=parts[2].toInt()
            val mode=RoundMode.valueOf(parts[3])
            val round=PixteeRound(course,length,mode)
            if(parts[4].isNotEmpty()) {
                val rows=parts[4].split(';')
                if(rows.size>length) return null
                for(row in rows) {
                    val a=row.split(',')
                    if(a.size!=3) return null
                    val strokes=a[0].toInt()
                    if(strokes !in 1..12) return null
                    round.record(strokes,a[1].toInt(),a[2].toInt())
                }
            }
            val ball=if(parts[5]=="-") null else {
                if(round.isComplete) return null
                val a=parts[5].split(',')
                if(a.size!=7) return null
                StableBall(a[0].toFloat(),a[1].toFloat(),a[2].toInt(),
                    a[3].toInt(),a[4].toInt(),a[5].toInt(),a[6].toFloat())
            }
            val careerEventIndex=if(current) parts[6].toInt() else -1
            if(careerEventIndex !in -1..24) return null
            if(careerEventIndex>=0) {
                if(mode!=RoundMode.CAREER) return null
                val event=PixteeCareer.events[careerEventIndex]
                if(event.course!=course || event.holes!=length) return null
            }
            SavedPixteeGame(round,ball,careerEventIndex)
        } catch (_: Exception) { null }
    }
}

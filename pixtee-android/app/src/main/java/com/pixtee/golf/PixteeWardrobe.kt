package com.pixtee.golf

/** Cosmetic-only equipment. No cosmetic ever changes ball flight or scoring. */
enum class StyleSlot { HAT, TOP, TROUSERS, SKIN, CLUB, BAG, BALL, ACCESSORY }
data class StyleItem(
    val id: String, val slot: StyleSlot, val name: String,
    val colour: Int, val level: Int=1,
    val achievement: String?=null, val careerWins: Int=0
) {
    fun unlocked(s: ReaderStats): Boolean =
        s.level >= level &&
        PixteeCareer.titles(s.careerMask) >= careerWins &&
        (achievement == null || PixteeRewards.achievements(s).any{
            it.id == achievement && it.unlocked
        })
}
object PixteeWardrobe {
    val items: List<StyleItem> = buildList {
        val colours=mapOf(
            StyleSlot.HAT to listOf(0xF3D243,0xFDF4D0,0x14285D,0xD73C36,0x19845D,0x9D52BA,0x2A9FC1,0xE19A3E,0xC16A8C,0x3D774C,0xF0E0AD,0xCCAD42,0x6295DC,0xE96B36,0x5F4C87,0xBCA7D1),
            StyleSlot.TOP to listOf(0x286ED2,0xC7354B,0xEBAC36,0x2D9446,0xEADAD2,0x5541A8,0xE1694B,0x32AFA0,0xAF4EA8,0xE0CE6C,0x484D5C,0xEFECE4,0xB64A22,0x2E6EAD,0x6FBF92,0xE2B8C8),
            StyleSlot.TROUSERS to listOf(0x1D2633,0xEDE1BE,0x2E6051,0x283E77,0x624D37,0xE3E5D4,0x1B1B1B,0x53568E,0x614F4B,0x3C7663,0x99772C,0x4C7FA4,0xCD8B5E,0xC8B8CC),
            StyleSlot.SKIN to listOf(0xEDBE89,0xA26E4B,0x543C30,0xB88762,0xF0D3AE,0x845637),
            StyleSlot.CLUB to listOf(0xD3D8DD,0xE6B940,0x29939A,0x292929,0xB45768,0xC6AE8B,0x7956A4,0x4DA3CF),
            StyleSlot.BAG to listOf(0x4D2F27,0xDE8E36,0x366A9C,0xB83D50,0x13826C,0xE1C46D,0x9551BA,0x252930),
            StyleSlot.BALL to listOf(0xFFFFFF,0xFFCA38,0xF5A7A2,0x91CDF0,0xE4DBAD,0xA7E8CD,0xBCB0EE,0xE17455),
            StyleSlot.ACCESSORY to listOf(0xFFFFFF,0xD6BE6C,0x73D6DC,0xDB9EC8,0xE38B51,0xBDAAE7)
        )
        colours.forEach { (slot, palette) ->
            palette.forEachIndexed { i,col ->
                val badge=if(slot==StyleSlot.BALL) when(i) {
                    4->"birdie";5->"eagle";6->"ten-rounds";7->"tour-title"
                    else->null
                } else null
                val wins=when {
                    slot==StyleSlot.CLUB && i>=6 -> (i-5)*4
                    slot==StyleSlot.BAG && i>=6 -> 5
                    slot==StyleSlot.ACCESSORY && i>=4 -> 3
                    else -> 0
                }
                val level=if(slot==StyleSlot.SKIN) 1
                    else 1+(i*26/(palette.size-1).coerceAtLeast(1))
                add(StyleItem("${slot.name.lowercase()}-$i",slot,
                    if(i==0) "CLASSIC" else "${slot.name} ${i+1}",
                    col,level,badge,wins))
            }
        }
    }
    private val byId=items.associateBy{it.id}
    val defaults: Map<StyleSlot,String> = StyleSlot.values().associateWith{slot->
        items.first{it.slot==slot}.id
    }
    fun item(id:String): StyleItem?=byId[id]
    fun available(slot:StyleSlot,s:ReaderStats): List<StyleItem> =
        items.filter { it.slot==slot && it.unlocked(s) }
    fun equippedItem(slot:StyleSlot,equipment:Map<StyleSlot,String>,
                     s:ReaderStats): StyleItem {
        val selected=byId[equipment[slot]]
        return selected?.takeIf{it.slot==slot && it.unlocked(s)}
            ?: byId.getValue(defaults.getValue(slot))
    }
    fun equip(equipment:Map<StyleSlot,String>,id:String,s:ReaderStats): Map<StyleSlot,String> {
        val selected=byId[id] ?: return equipment
        return if(selected.unlocked(s)) equipment+(selected.slot to id) else equipment
    }
    fun encode(equipment:Map<StyleSlot,String>,s:ReaderStats): String =
        StyleSlot.values().joinToString(";") {slot->
            "${slot.name}=${equippedItem(slot,equipment,s).id}"
        }
    fun decode(value:String?,s:ReaderStats): Map<StyleSlot,String> {
        val equipment=defaults.toMutableMap()
        if(value==null || value.length>1024) return equipment
        value.split(';').forEach{record->
            val pair=record.split('=')
            if(pair.size!=2) return@forEach
            val slot=StyleSlot.values().firstOrNull{it.name==pair[0]} ?:return@forEach
            val candidate=byId[pair[1]] ?:return@forEach
            if(candidate.slot==slot && candidate.unlocked(s)) equipment[slot]=candidate.id
        }
        return equipment
    }
}

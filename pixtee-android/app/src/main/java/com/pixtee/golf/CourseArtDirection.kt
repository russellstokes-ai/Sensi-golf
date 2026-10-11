package com.pixtee.golf

/**
 * Original Pixtee palette direction.
 * Provisional development colours; final sprite/art assets require publisher
 * approval before release. Rendering assets are separate from collision data.
 */
data class CoursePalette(
    val name: String,
    val rough: Int,
    val fairway: Int,
    val green: Int,
    val sand: Int,
    val water: Int,
    val accent: Int
)

object CourseArtDirection {
    private fun rgb(value: Long): Int = (0xFF000000L or value).toInt()
    val PALETTES = listOf(
        CoursePalette("PARKLAND",rgb(0x337C25),rgb(0x55AE32),rgb(0x8ED058),rgb(0xD9C88C),rgb(0x2765B2),rgb(0xEFDAB4)),
        CoursePalette("WOODLAND",rgb(0x2B622D),rgb(0x4F9135),rgb(0x91C35B),rgb(0xCAB282),rgb(0x39789F),rgb(0xF4C5DB)),
        CoursePalette("HEATH",rgb(0x627440),rgb(0x83984D),rgb(0xB4C571),rgb(0xCFBF98),rgb(0x476982),rgb(0xCB90D0)),
        CoursePalette("LINKS",rgb(0x758E48),rgb(0x9DAF63),rgb(0xCADE89),rgb(0xE5D39D),rgb(0x2476A6),rgb(0xF8EBC9)),
        CoursePalette("MARSH",rgb(0x486843),rgb(0x789C58),rgb(0xA8CB7E),rgb(0xB9AA7F),rgb(0x28748E),rgb(0xF9DB7D)),
        CoursePalette("DESERT",rgb(0xAF905A),rgb(0xC0B473),rgb(0xDADE97),rgb(0xE8C88A),rgb(0x3B8EB0),rgb(0xF7B65A)),
        CoursePalette("HIGHLAND",rgb(0x47684B),rgb(0x739765),rgb(0xB0C982),rgb(0xBDB49E),rgb(0x2C658C),rgb(0xECE6F9)),
        CoursePalette("COASTAL",rgb(0x528F71),rgb(0x83BA8A),rgb(0xC0E0AD),rgb(0xE8D9B1),rgb(0x148DBB),rgb(0xF3EDC6)),
        CoursePalette("AUTUMN",rgb(0x66763A),rgb(0x8EA04C),rgb(0xBDD379),rgb(0xD2BB88),rgb(0x3D718F),rgb(0xFFCD71)),
        CoursePalette("FROST",rgb(0x829D9C),rgb(0xA8C1B1),rgb(0xD4E5CD),rgb(0xDED6BD),rgb(0x4C84B3),rgb(0xF8F6E8))
    )

    fun palette(theme: Int): CoursePalette =
        PALETTES[theme.coerceIn(PALETTES.indices)]
}

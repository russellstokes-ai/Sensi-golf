# Sensi Golf — original-engine Android touch preview

This is the real recovered `sensigolf_core` compiled via Android NDK and controlled with on-screen controls. This is a **test harness APK**, not a final art-complete port.

It contains **no commercial original assets**. On first launch the user must select a folder containing legitimately obtained and extracted original Windows v1.014 EPF resources. At minimum: `MAPI01.RAW`, `MAPI02.RAW`, `MAPM42.MAP`, `MAPM42.SPT`, `MAPS42.MAP` (and analogous resources for subsequent holes). The UI imports by Android Storage Access Framework; it does not fetch or redistribute original content.

**Known fidelity boundary:** touch layout and map colours are temporary debugging visuals, not recovered original artwork or measured original camera scale. Original meter extreme-accuracy behaviour and code9/10 special-green continuation remain incomplete; the test meter limits its accuracy sweep to the previously validated range rather than fabricating missing physics. The original C++ simulation is unchanged.

Android debug build uses Gradle 8.9 / Android Gradle Plugin 8.7.3 / SDK35 / pinned NDK 27.2 / CMake 3.22.1.

Install the unique APK from the GitHub workflow artifact; it is `app-debug.apk` internally, with distinct artifact name and package `com.russellstokes.sensigolf.preview`.

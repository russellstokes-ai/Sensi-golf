package com.russellstokes.sensigolf.fullgame

import android.app.AlertDialog
import android.net.Uri
import android.widget.LinearLayout
import android.widget.Button
import androidx.activity.result.contract.ActivityResultContracts
import java.io.ByteArrayOutputStream
import java.util.zip.ZipInputStream
import java.util.zip.ZipOutputStream
import java.util.zip.ZipEntry

import android.content.res.Configuration
import android.graphics.Color
import android.os.Bundle
import android.os.Build
import android.view.Gravity
import android.view.KeyEvent
import android.view.MotionEvent
import android.widget.TextView
import android.graphics.drawable.GradientDrawable
import android.view.View
import android.view.ViewGroup
import android.view.WindowManager
import android.view.WindowInsets
import android.widget.FrameLayout
import android.widget.ImageView
import androidx.activity.ComponentActivity
import com.swordfish.libretrodroid.GLRetroView
import com.swordfish.libretrodroid.GLRetroViewData
import com.swordfish.libretrodroid.ShaderConfig
import com.swordfish.libretrodroid.Variable
import java.io.File
import java.io.FileOutputStream

/**
 * Dedicated single-game Android shell.
 * Original DOS game logic, menus, courses, graphics, music and sound run in
 * the integrated GPL LibretroDroid/DOSBox Pure modules, NOT in a rewritten game.
 * No DOSBox UI, file picker, display-mode switch or runtime branding is exposed.
 *
 * This source is NOT evidence that original rights have been cleared, all
 * game screens work or an actual device playthrough has passed.
 */
class FullGameActivity : ComponentActivity() {
    private lateinit var gameView: GLRetroView
    private var initialized = false
    private var pausedByMenu = false
    private lateinit var gameFrame: FrameLayout
    private var cutoutLeft = 0
    private var cutoutTop = 0
    private var cutoutRight = 0
    private var cutoutBottom = 0
    // Distributable frontend carries no commercial game binary. The owner's
    // original ZIP is imported locally once. The CI compatibility build may
    // still use an injected private archive without showing this picker.
    private val chooseOriginalArchive = registerForActivityResult(
        ActivityResultContracts.OpenDocument()
    ) { uri: Uri? ->
        if (uri != null) {
            try {
                importOriginalGameArchive(uri)
                recreate()
            } catch (error: Exception) {
                AlertDialog.Builder(this)
                    .setTitle("Cannot import game")
                    .setMessage(error.message ?: "Select a ZIP containing the original DOS game files.")
                    .setPositiveButton("OK", null)
                    .show()
            }
        }
    }


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        window.navigationBarColor = Color.BLACK
        window.statusBarColor = Color.BLACK
        enableFullScreen()

        // Licensed game archive is committed nowhere in the public repository.
        // It is injected into the APK only during a rights-approved build.
        val archive = File(filesDir, "SensibleGolf.dosz")
        if (!archive.isFile || archive.length() == 0L) {
            // The private CI reference build supplies a prepacked archive.
            // Public test APKs deliberately omit it; show a one-time import
            // rather than crashing or making users use the command line.
            val packaged = try {
                assets.open("game/SensibleGolf.dosz")
            } catch (_: java.io.FileNotFoundException) {
                null
            }
            if (packaged == null) {
                showOriginalGameImportScreen()
                return
            }
            FileOutputStream(archive).use { dst -> packaged.use { src -> src.copyTo(dst) } }
        }

        val config = GLRetroViewData(this).apply {
            coreFilePath = "libdosbox_pure_libretro_android.so"
            gameFilePath = archive.absolutePath
            systemDirectory = filesDir.absolutePath
            savesDirectory = File(filesDir, "saves").apply { mkdirs() }.absolutePath

            // Single fixed graphics path, preserving original 4:3/asset aspect.
            // Gentle hardware bilinear smoothing: no user-facing graphics mode.
            shader = ShaderConfig.Default

            // Internal config. Never show core menus, keyboard or emulation UI.
            variables = arrayOf(
                // Absolute touchscreen presses must select the button under the
                // finger. "pad" is *relative* touchpad mode: its tap clicks
                // at the old cursor (observed over Play Season while the
                // original menu was visible), even when the finger is on
                // Play Round. DOSBox Pure "direct" consumes LibretroDroid's
                // normalized on-screen pointer coordinates instead.
                // This is an Android-host input fix only: game code is intact.
                Variable("dosbox_pure_mouse_input", "direct"),
                Variable("dosbox_pure_on_screen_keyboard", "false"),
                Variable("dosbox_pure_menu_time", "0"),
                Variable("dosbox_pure_auto_mapping", "false")
            )
            rumbleEventsEnabled = false
            preferLowLatencyAudio = true
        }

        gameView = GLRetroView(this, config)
        lifecycle.addObserver(gameView)
        val frame = FrameLayout(this)
        // The original video is 4:3. Side/top margins are deliberate: never
        // crop course edges or warp golfer, text and scorecard on wide phones.
        frame.setBackgroundColor(Color.rgb(13, 27, 23))
        gameFrame = frame
        frame.addView(
            gameView,
            FrameLayout.LayoutParams(1, 1, Gravity.TOP or Gravity.LEFT)
        )
        frame.addOnLayoutChangeListener { _, _, _, _, _, _, _, _, _ ->
            fitGameInsideAvailableScreen()
        }
        frame.setOnApplyWindowInsetsListener { _, insets ->
            // Fold / display-cutout safe bounds, even while bars are hidden.
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
                @Suppress("DEPRECATION")
                val cutout = insets.displayCutout
                cutoutLeft = cutout?.safeInsetLeft ?: 0
                cutoutTop = cutout?.safeInsetTop ?: 0
                cutoutRight = cutout?.safeInsetRight ?: 0
                cutoutBottom = cutout?.safeInsetBottom ?: 0
            }
            fitGameInsideAvailableScreen()
            insets
        }
        // Branded launch overlay masks all internal runtime initialization.
        // The original game and its own menu are the first thing the player sees.
        val splash = FrameLayout(this).apply { setBackgroundColor(Color.rgb(15, 34, 30)) }
        val logo = ImageView(this).apply {
            setImageResource(R.drawable.approved_eagle)
            scaleType = ImageView.ScaleType.FIT_CENTER
            contentDescription = "Sensible Golf"
        }
        val iconSize = (144 * resources.displayMetrics.density).toInt()
        splash.addView(logo, FrameLayout.LayoutParams(iconSize, iconSize, Gravity.CENTER))
        frame.addView(splash, FrameLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT
        ))
        setContentView(frame)
        // Runtime screen capture validation must confirm the core has reached the
        // actual game before uncovering it; this delay alone is NOT that proof.
        splash.postDelayed({
            splash.animate().alpha(0f).setDuration(250).withEndAction {
                frame.removeView(splash)
                // Reveal the optional controller toggle only AFTER the
                // untouched eagle splash has completely faded.
                attachOptionalTouchControls(frame)
            }.start()
        }, 3200L)
        initialized = true
    }

    /**
     * Import any *operator-supplied* ZIP containing the original DOS game.
     * Like tools/build_original_dosz.py, flatten a one-level or nested ZIP,
     * omit the Windows executable and package only the DOS game support files.
     * No network connection, uploads, emulation change or game rewrite.
     */
    private fun importOriginalGameArchive(uri: Uri) {
        val archive = File(filesDir, "SensibleGolf.dosz")
        val temp = File(filesDir, "SensibleGolf.dosz.tmp")
        val entries = linkedMapOf<String, ByteArray>()
        var total = 0L
        var count = 0
        try {
            val stream = contentResolver.openInputStream(uri)
                ?: throw IllegalArgumentException("Cannot open the selected ZIP.")
            ZipInputStream(stream.buffered()).use { zip ->
                var item = zip.nextEntry
                while (item != null) {
                    if (!item.isDirectory) {
                        count++
                        if (count > 1000) throw IllegalArgumentException("Too many files in ZIP.")
                        val name = item.name.replace('\\', '/').substringAfterLast('/').uppercase()
                        if (name.isNotBlank() && name != "." && name != ".." &&
                            name != "GOLFWIN.EXE" &&
                            (name == "GOLFDOS.EXE" ||
                             !(name.endsWith(".EXE") || name.endsWith(".COM") ||
                               name.endsWith(".BAT")))) {
                            if (entries.containsKey(name)) {
                                throw IllegalArgumentException("Duplicate game file: $name")
                            }
                            val bytes = ByteArrayOutputStream()
                            val buffer = ByteArray(8192)
                            while (true) {
                                val read = zip.read(buffer)
                                if (read < 0) break
                                bytes.write(buffer, 0, read)
                                if (bytes.size() > 40 * 1024 * 1024) {
                                    throw IllegalArgumentException("Game file too large: $name")
                                }
                            }
                            total += bytes.size().toLong()
                            if (total > 80L * 1024 * 1024) {
                                throw IllegalArgumentException("Game archive exceeds 80 MB.")
                            }
                            entries[name] = bytes.toByteArray()
                        }
                    }
                    zip.closeEntry()
                    item = zip.nextEntry
                }
            }
            val missing = listOf("GOLFDOS.EXE", "GOLF.EPF").filterNot { entries.containsKey(it) }
            if (missing.isNotEmpty()) {
                throw IllegalArgumentException("This is not the original DOS Sensible Golf archive. Missing: ${missing.joinToString()}")
            }
            ZipOutputStream(FileOutputStream(temp).buffered()).use { zip ->
                for ((name, bytes) in entries.toSortedMap()) {
                    zip.putNextEntry(ZipEntry(name))
                    zip.write(bytes)
                    zip.closeEntry()
                }
            }
            if (archive.exists() && !archive.delete()) {
                throw java.io.IOException("Cannot replace previous game archive.")
            }
            if (!temp.renameTo(archive)) {
                temp.copyTo(archive, overwrite = true)
                temp.delete()
            }
        } catch (e: Exception) {
            temp.delete()
            throw e
        }
    }

    private fun showOriginalGameImportScreen() {
        val layout = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER
            setBackgroundColor(Color.rgb(13, 27, 23))
            setPadding(30, 30, 30, 30)
        }
        val title = TextView(this).apply {
            text = "Sensible Golf"
            textSize = 28f
            gravity = Gravity.CENTER
            setTextColor(Color.WHITE)
        }
        val description = TextView(this).apply {
            text = "Choose your own original DOS Sensible Golf ZIP file to play. " +
                "Your game stays on this device."
            gravity = Gravity.CENTER
            setTextColor(Color.LTGRAY)
            textSize = 15f
            setPadding(0, 24, 0, 24)
        }
        val button = Button(this).apply {
            text = "Choose original game ZIP"
            setOnClickListener {
                chooseOriginalArchive.launch(arrayOf(
                    "application/zip", "application/octet-stream", "*/*"
                ))
            }
        }
        layout.addView(title)
        layout.addView(description)
        layout.addView(button)
        setContentView(layout)
    }

    /**
     * This changes only the Android video surface size, never the internal
     * game resolution or logic. GLSurfaceView pointer normalization therefore
     * uses the actual content bounds, not the letterbox/gutter width.
     */
    private fun fitGameInsideAvailableScreen() {
        if (!::gameFrame.isInitialized || !::gameView.isInitialized) return
        val w = gameFrame.width
        val h = gameFrame.height
        if (w <= 0 || h <= 0) return

        val left = cutoutLeft.coerceAtMost(w / 4)
        val right = cutoutRight.coerceAtMost(w / 4)
        val top = cutoutTop.coerceAtMost(h / 4)
        val bottom = cutoutBottom.coerceAtMost(h / 4)
        val rect = ViewportPolicy.fit(w, h, left, top, right, bottom)
        val existing = gameView.layoutParams as FrameLayout.LayoutParams
        if (existing.width != rect.width() || existing.height != rect.height() ||
            existing.leftMargin != rect.left || existing.topMargin != rect.top) {
            gameView.layoutParams = FrameLayout.LayoutParams(
                rect.width(), rect.height(), Gravity.TOP or Gravity.LEFT
            ).apply {
                leftMargin = rect.left
                topMargin = rect.top
            }
        }
    }

    /**
     * One small touch-controller toggle outside the classic content on wide
     * screens. Original keyboard gameplay needs arrows + CTRL as the fire
     * control. All input still reaches the unmodified original executable.
     *
     * Foldable devices have less gutter space than ordinary 16:9 phones.
     * In that case these semitransparent buttons may overlay video only while
     * explicitly expanded by the player, never by default.
     */
    private fun attachOptionalTouchControls(frame: FrameLayout) {
        fun dp(value: Int) = (value * resources.displayMetrics.density).toInt()
        val controls = FrameLayout(this).apply {
            visibility = View.GONE
            isClickable = false
            contentDescription = "Sensible Golf touch gameplay controls"
        }
        frame.addView(controls, FrameLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT
        ))

        fun shape(): GradientDrawable = GradientDrawable().apply {
            setColor(Color.argb(164, 14, 31, 27))
            setStroke(dp(1), Color.argb(180, 237, 230, 191))
            cornerRadius = dp(13).toFloat()
        }

        fun gameButton(
            title: String, keyCode: Int,
            anchorGravity: Int, xMargin: Int, bottomInset: Int, width: Int = 50, height: Int = 50
        ) {
            val button = TextView(this).apply {
                text = title
                gravity = Gravity.CENTER
                textSize = if (title == "SWING") 13f else 24f
                setTextColor(Color.WHITE)
                background = shape()
                contentDescription = "Golf: $title"
                isClickable = true
                // Hold-to-aim/reselect club, or quick fire/three-click swing.
                setOnTouchListener { _, event ->
                    when (event.actionMasked) {
                        MotionEvent.ACTION_DOWN -> {
                            gameView.sendKeyEvent(KeyEvent.ACTION_DOWN, keyCode)
                            true
                        }
                        MotionEvent.ACTION_UP, MotionEvent.ACTION_CANCEL -> {
                            gameView.sendKeyEvent(KeyEvent.ACTION_UP, keyCode)
                            true
                        }
                        else -> true
                    }
                }
            }
            val lp = FrameLayout.LayoutParams(dp(width), dp(height), anchorGravity).apply {
                bottomMargin = dp(bottomInset)
                if ((anchorGravity and Gravity.RIGHT) == Gravity.RIGHT || (anchorGravity and Gravity.END) == Gravity.END) {
                    rightMargin = dp(xMargin)
                } else {
                    leftMargin = dp(xMargin)
                }
            }
            controls.addView(button, lp)
        }

        gameButton("↑", KeyEvent.KEYCODE_DPAD_UP, Gravity.LEFT or Gravity.BOTTOM, 72, 122)
        gameButton("↓", KeyEvent.KEYCODE_DPAD_DOWN, Gravity.LEFT or Gravity.BOTTOM, 72, 20)
        gameButton("←", KeyEvent.KEYCODE_DPAD_LEFT, Gravity.LEFT or Gravity.BOTTOM, 20, 71)
        gameButton("→", KeyEvent.KEYCODE_DPAD_RIGHT, Gravity.LEFT or Gravity.BOTTOM, 124, 71)
        gameButton("SWING", KeyEvent.KEYCODE_CTRL_LEFT, Gravity.RIGHT or Gravity.BOTTOM, 30, 45, 86, 86)

        val toggle = TextView(this).apply {
            text = "Pad"
            gravity = Gravity.CENTER
            textSize = 13f
            setTextColor(Color.WHITE)
            background = shape()
            contentDescription = "Show or hide original golf controls"
            setOnClickListener {
                controls.visibility = if (controls.visibility == View.VISIBLE) View.GONE else View.VISIBLE
                text = if (controls.visibility == View.VISIBLE) "Hide" else "Pad"
            }
        }
        frame.addView(toggle, FrameLayout.LayoutParams(dp(55), dp(34), Gravity.RIGHT or Gravity.TOP).apply {
            rightMargin = dp(14)
            topMargin = dp(18)
        })
    }

    private fun enableFullScreen() {
        @Suppress("DEPRECATION")
        window.decorView.systemUiVisibility =
            View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY or
            View.SYSTEM_UI_FLAG_FULLSCREEN or
            View.SYSTEM_UI_FLAG_HIDE_NAVIGATION or
            View.SYSTEM_UI_FLAG_LAYOUT_STABLE or
            View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN or
            View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) enableFullScreen()
    }

    override fun onConfigurationChanged(newConfig: Configuration) {
        super.onConfigurationChanged(newConfig)
        enableFullScreen()
        // Same instance + same game session survive Fold open/close/rotation.
        if (initialized) {
            gameView.requestLayout()
            gameFrame.post { fitGameInsideAvailableScreen() }
        }
    }

    @Deprecated("Back action is a game pause dialog, never the emulator menu.")
    override fun onBackPressed() {
        if (!initialized || pausedByMenu) return
        pausedByMenu = true
        gameView.onPause()
        AlertDialog.Builder(this)
            .setTitle("Paused")
            .setItems(arrayOf("Resume game", "Restart game", "Exit")) { dialog, index ->
                when (index) {
                    0 -> gameView.onResume()
                    1 -> { gameView.reset(); gameView.onResume() }
                    2 -> { finish(); return@setItems }
                }
                pausedByMenu = false
                enableFullScreen()
            }
            .setOnCancelListener {
                pausedByMenu = false
                gameView.onResume()
                enableFullScreen()
            }
            .show()
    }

    override fun onKeyDown(keyCode: Int, event: KeyEvent): Boolean {
        // Android Back is our own pause action. ESC, Space, Enter, arrows and
        // physical controller keys must reach the ORIGINAL DOS game, not be
        // intercepted by Android or lost through an unfocused SurfaceView.
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            onBackPressed()
            return true
        }
        if (keyCode == KeyEvent.KEYCODE_VOLUME_UP ||
            keyCode == KeyEvent.KEYCODE_VOLUME_DOWN) {
            return super.onKeyDown(keyCode, event)
        }
        if (initialized && !pausedByMenu) {
            gameView.sendKeyEvent(KeyEvent.ACTION_DOWN, keyCode)
            return true
        }
        return super.onKeyDown(keyCode, event)
    }

    override fun onKeyUp(keyCode: Int, event: KeyEvent): Boolean {
        if (keyCode == KeyEvent.KEYCODE_BACK) return true
        if (keyCode == KeyEvent.KEYCODE_VOLUME_UP ||
            keyCode == KeyEvent.KEYCODE_VOLUME_DOWN) {
            return super.onKeyUp(keyCode, event)
        }
        if (initialized && !pausedByMenu) {
            gameView.sendKeyEvent(KeyEvent.ACTION_UP, keyCode)
            return true
        }
        return super.onKeyUp(keyCode, event)
    }
}

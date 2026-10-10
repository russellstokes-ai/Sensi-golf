package com.russellstokes.sensigolf.fullgame

import android.app.AlertDialog
import android.content.res.Configuration
import android.graphics.Color
import android.os.Bundle
import android.os.Build
import android.view.Gravity
import android.view.KeyEvent
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
            val packaged = assets.open("game/SensibleGolf.dosz")
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
                Variable("dosbox_pure_mouse_input", "pad"),
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
            }.start()
        }, 3200L)
        initialized = true
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

package com.russellstokes.sensigolf;

/** Device-independent layout policy, checked in desktop and Android CI. */
public final class GolfLayout {
    public enum Mode { PHONE_PORTRAIT, PHONE_LANDSCAPE, EXPANDED_FOLD_TABLET }
    private GolfLayout() {}

    public static Mode mode(float widthDp, float heightDp) {
        if (widthDp <= 0 || heightDp <= 0) {
            throw new IllegalArgumentException("available viewport must be positive");
        }
        if (widthDp < heightDp) return Mode.PHONE_PORTRAIT;
        if (widthDp >= 680f && heightDp >= 480f) return Mode.EXPANDED_FOLD_TABLET;
        return Mode.PHONE_LANDSCAPE;
    }

    /** Controls are at least 48dp and are not scaled down on large tablets. */
    public static float controlTargetDp(float widthDp, float heightDp) {
        return mode(widthDp,heightDp) == Mode.EXPANDED_FOLD_TABLET ? 56f : 48f;
    }

    /** Sidebar on landscape phones and unfolded Fold/tablet. */
    public static float sidebarDp(float widthDp, float heightDp) {
        switch(mode(widthDp,heightDp)) {
        case PHONE_PORTRAIT: return 0f;
        case EXPANDED_FOLD_TABLET: return Math.min(270f,Math.max(204f,widthDp*0.275f));
        default: return Math.min(204f,Math.max(150f,widthDp*0.30f));
        }
    }

    /** Portrait: accessible bottom dock; other modes: side panel. */
    public static float bottomDockDp(float widthDp, float heightDp) {
        return mode(widthDp,heightDp)==Mode.PHONE_PORTRAIT
                ? Math.min(heightDp*0.43f,224f) : 0f;
    }
}

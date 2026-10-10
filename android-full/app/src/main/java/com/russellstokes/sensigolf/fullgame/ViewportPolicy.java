package com.russellstokes.sensigolf.fullgame;

/**
 * Pure pixel-layout calculator for classic DOS video. Never stretches or crops.
 * Unlike an Android density-dependent dp rule, video projection is determined
 * entirely in display pixels from the measured usable bounds.
 */
public final class ViewportPolicy {
    public static final int CONTENT_ASPECT_W = 4;
    public static final int CONTENT_ASPECT_H = 3;

    private ViewportPolicy() {}

    public static final class Rect {
        public final int left, top, right, bottom;
        Rect(int left, int top, int right, int bottom) {
            this.left=left;this.top=top;this.right=right;this.bottom=bottom;
        }
        public int width() { return right-left; }
        public int height() { return bottom-top; }
        @Override public String toString() {
            return "["+left+","+top+","+right+","+bottom+"]";
        }
    }

    /**
     * Fits an entire 4:3 original image in the safe window bounds.
     * Edges are inside the safe area even for portrait, landscape and cutouts.
     * Modest integer rounding to the nearest pixel is unavoidable.
     */
    public static Rect fit(int width, int height, int insetLeft, int insetTop,
                           int insetRight, int insetBottom) {
        if(width<=0 || height<=0 || insetLeft<0 || insetTop<0
                || insetRight<0 || insetBottom<0
                || insetLeft+insetRight>=width || insetTop+insetBottom>=height) {
            throw new IllegalArgumentException("Invalid game window / safe insets");
        }
        int availableW=width-insetLeft-insetRight;
        int availableH=height-insetTop-insetBottom;

        // Integer comparisons prevent floating-point overshoot at unusual ratios.
        int fitW,fitH;
        if((long)availableW*CONTENT_ASPECT_H <=
                (long)availableH*CONTENT_ASPECT_W) {
            fitW=availableW;
            fitH=(int)((long)availableW*CONTENT_ASPECT_H/CONTENT_ASPECT_W);
        } else {
            fitH=availableH;
            fitW=(int)((long)availableH*CONTENT_ASPECT_W/CONTENT_ASPECT_H);
        }
        fitW=Math.max(1,fitW);
        fitH=Math.max(1,fitH);
        int left=insetLeft+(availableW-fitW)/2;
        int top=insetTop+(availableH-fitH)/2;
        return new Rect(left,top,left+fitW,top+fitH);
    }
}

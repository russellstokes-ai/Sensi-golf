import com.russellstokes.sensigolf.GolfLayout;

public final class GolfLayoutSmokeTest {
    private static void check(boolean ok,String msg) {
        if(!ok)throw new AssertionError(msg);
    }
    public static void main(String[] args) {
        check(GolfLayout.mode(360,740)==GolfLayout.Mode.PHONE_PORTRAIT,
            "closed Fold / phone portrait");
        check(GolfLayout.mode(740,360)==GolfLayout.Mode.PHONE_LANDSCAPE,
            "phone landscape retains compact UI at limited height");
        check(GolfLayout.mode(600,340)==GolfLayout.Mode.PHONE_LANDSCAPE,
            "compact landscape phone");
        check(GolfLayout.mode(860,620)==GolfLayout.Mode.EXPANDED_FOLD_TABLET,
            "opened Fold inner screen");
        check(GolfLayout.mode(1280,800)==GolfLayout.Mode.EXPANDED_FOLD_TABLET,
            "landscape tablet");
        check(GolfLayout.mode(800,1280)==GolfLayout.Mode.PHONE_PORTRAIT,
            "tablet portrait bottom dock");
        check(GolfLayout.sidebarDp(600,340)>=150f,
            "compact landscape controls remain usable");
        check(GolfLayout.sidebarDp(860,620)>=204f,
            "opened Fold has full right-hand controls");
        check(GolfLayout.controlTargetDp(360,740)>=48f,
            "smallest phone retains accessible 48dp targets");
        check(GolfLayout.controlTargetDp(1280,800)>=56f,
            "large tablet uses larger targets");
        check(GolfLayout.bottomDockDp(360,740)>0f,
            "portrait has lower control dock");
        check(GolfLayout.bottomDockDp(1280,800)==0f,
            "tablet has independent course and sidebar");
        System.out.println("PASS: closed Fold, open Fold, phone, tablet, portrait, landscape");
    }
}

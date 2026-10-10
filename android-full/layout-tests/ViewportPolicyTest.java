import com.russellstokes.sensigolf.fullgame.ViewportPolicy;

public final class ViewportPolicyTest {
    private static void check(boolean value,String label) {
        if(!value)throw new AssertionError(label);
    }
    private static void caseCheck(String name,int w,int h,int l,int t,int r,int b) {
        ViewportPolicy.Rect game=ViewportPolicy.fit(w,h,l,t,r,b);
        check(game.left>=l&&game.top>=t&&game.right<=w-r&&game.bottom<=h-b,
            name+": cropped by safe area "+game);
        check(game.width()>0&&game.height()>0,name+": empty viewport");
        check(Math.abs(game.width()*3-game.height()*4)<=4,
            name+": stretched ratio "+game);
        check(game.width()==w-l-r || game.height()==h-t-b,
            name+": fails to use available size");
        System.out.println("PASS "+name+" "+w+"x"+h+" -> "+game);
    }
    public static void main(String[] args) {
        caseCheck("folded phone landscape",740,360,0,0,0,0);
        caseCheck("small phone landscape",640,360,0,0,0,0);
        caseCheck("phone landscape cutout",740,360,30,0,0,0);
        caseCheck("phone portrait",360,740,0,30,0,20);
        caseCheck("unfolded fold",840,620,0,0,0,0);
        caseCheck("unfolded fold cutout",840,620,0,24,0,0);
        caseCheck("large tablet landscape",1280,800,0,0,0,0);
        caseCheck("tablet portrait",800,1280,0,24,0,24);
        caseCheck("full-HD phone",1920,1080,0,0,0,0);
        caseCheck("QHD foldable",2176,1812,0,0,0,0);
        for(int w=320;w<=2200;w+=51)for(int h=300;h<=1800;h+=43) {
            ViewportPolicy.Rect v=ViewportPolicy.fit(w,h,0,0,0,0);
            check(v.left>=0&&v.top>=0&&v.right<=w&&v.bottom<=h,
                "random bounds "+w+"x"+h);
            check(Math.abs(v.width()*3-v.height()*4)<=4,
                "random aspect "+w+"x"+h);
        }
        System.out.println("PASS full matrix of landscape, portrait, fold and tablet aspect/bounds checks");
    }
}

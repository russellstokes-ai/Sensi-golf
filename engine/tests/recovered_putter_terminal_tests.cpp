#include <cassert>

#include "sensigolf/recovered_putter_terminal.hpp"

using namespace sensigolf::recovered;

int main() {
    PutterTerminalState hole{};
    hole.counter_52=7;
    hole.counter_56=11;
    hole.player_mode_5a=0;
    assert(apply_putter_terminal(8,hole));
    assert(hole.kind==PutterTerminalKind::Holed);
    assert(hole.player_flags==4);
    assert(hole.counter_52==8);
    assert(hole.counter_56==12);
    assert(hole.pause==100);
    assert(hole.event_id==1);
    assert(hole.terminal_flag==1);
    assert(hole.ui_gate==1);

    PutterTerminalState code9{};
    code9.counter_52=7;
    code9.counter_56=11;
    assert(apply_putter_terminal(9,code9));
    assert(code9.kind==PutterTerminalKind::SpecialGreenStop);
    assert(code9.player_flags==4);
    assert(code9.counter_52==7);
    assert(code9.counter_56==11);
    assert(code9.pause==100);
    assert(code9.event_id==11);
    assert(code9.terminal_flag==1);
    assert(code9.ui_gate==0);

    PutterTerminalState code10=code9;
    code10.kind=PutterTerminalKind::None;
    assert(apply_putter_terminal(10,code10));
    assert(code10.kind==PutterTerminalKind::SpecialGreenStop);
    assert(code10.event_id==11);

    PutterTerminalState normal{};
    assert(!apply_putter_terminal(1,normal));
    assert(normal.kind==PutterTerminalKind::None);
    return 0;
}

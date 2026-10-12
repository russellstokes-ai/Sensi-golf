#include "original_cpu_search_grid.hpp"
#include <cassert>
int main() {
    using pixtee::original_cpu_scan_grid_from_original_table;
    const auto x=original_cpu_scan_grid_from_original_table(4,0);
    assert(x.table_word_0==32 && x.table_word_1==512 && x.table_word_2==32 && x.table_word_3==2);
    assert(x.spacing==4 && x.initial_remainder==128);
    const auto y=original_cpu_scan_grid_from_original_table(9,3);
    assert(y.table_word_0==35 && y.table_word_1==2030 && y.table_word_2==116 && y.table_word_3==3);
    assert(y.spacing==4 && y.initial_remainder==140);
    for(unsigned band=0;band<4;++band) for(unsigned mode=4;mode<=9;++mode){
        const auto p=original_cpu_scan_grid_from_original_table(mode,band);
        assert(p.table_word_0!=0 && p.spacing==154/p.table_word_0);
        assert(p.initial_remainder==154-(154%p.table_word_0));
    }
}

#include "toolbox.h"

namespace RP {
    String Toolbox::beautifyDuration(int seconds) {        
        String result;
        // compute h, m, s
        String h = String(seconds / 3600);
        String m = String((seconds % 3600) / 60);
        String s = String(seconds % 60);
        // add leading zero if needed
        String hh = String((h.length() < 2) ? ('0' + h) : h);
        String mm = String((m.length() < 2) ? ('0' + m) : m);
        String ss = String((s.length() < 2) ? ('0' + s) : s);
            
        return  result = hh + ':' + mm + ":" + ss;        
    }
}

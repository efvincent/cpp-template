export module app;

import core_console;
import core_string;

using namespace base;
using namespace str;

export namespace app {
  inline int run() {
    console::printl(Str8("hello from {{project-name}}"));
    return 0;
  }
}

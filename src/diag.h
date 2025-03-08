#pragma once

#include "token.h"
#include "util.h"

namespace swan {

template <typename... Args>
void error(SourcePos pos, Args... args)
{
    print("error:", pos.line, ":", pos.col, ": ", args...);
    print("\n");
}

template <typename... Args>
void warning(SourcePos pos, Args... args)
{
    print("warning:", pos.line, ":", pos.col, args...);
    print("\n");
}

} // namespace swan
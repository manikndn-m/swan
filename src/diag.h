#pragma once

#include "token.h"
#include "util.h"

namespace swan {

struct HaltToken {};

template <typename... Args>
void err(SourceRange srange, Args... args)
{
    print("error:", srange.start.line, ":", srange.start.col, ": ", args...);
    print("\n");
    throw HaltToken{};
}

} // namespace swan

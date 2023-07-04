
#pragma once

#include <display/Style.h>

namespace bbq::ui
{
    using namespace display;

    class StyleSheet
    {
    public:
        StylePtr Default;
        StylePtr Title;

        StylePtr SmallText;

        StylePtr SmallMonospace;
        StylePtr LargeMonospace;
    };
}

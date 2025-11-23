#ifndef LAYOUT_H
#define LAYOUT_H

#include <memory>
#include <vector>
#include "Style.h"

struct LayoutRect {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

class LayoutBox {
public:
    std::shared_ptr<StyledNode> styled;
    LayoutRect rect;
    std::vector<std::shared_ptr<LayoutBox>> children;

    LayoutBox(std::shared_ptr<StyledNode> n)
        : styled(n) {
    }
};

class LayoutEngine {
public:
    static std::shared_ptr<LayoutBox> Build(std::shared_ptr<StyledNode> root, int viewportWidth);
};

#endif
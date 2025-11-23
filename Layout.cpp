#include "Layout.h"
#include <string>

static int parsePx(const std::string* v, int defaultValue)
{
    if (!v || v->empty())
        return defaultValue;

    std::string s = *v;
    size_t pos = 0;
    while (pos < s.size() && s[pos] == ' ')
        pos++;

    int num = 0;
    bool hasNum = false;

    while (pos < s.size() && s[pos] >= '0' && s[pos] <= '9')
    {
        hasNum = true;
        num = num * 10 + (s[pos] - '0');
        pos++;
    }

    if (!hasNum)
        return defaultValue;

    return num;
}

static int measureTextHeight(const std::string& text, int width)
{
    int lineHeight = 16;
    int charsPerLine = width / 8;
    if (charsPerLine <= 0)
        charsPerLine = 1;

    int len = (int)text.size();
    int lines = len / charsPerLine + 1;
    if (len == 0)
        lines = 0;

    return lines * lineHeight;
}

static void layoutChildren(std::shared_ptr<LayoutBox> box, int contentX, int contentY, int contentWidth)
{
    int cursorY = contentY;

    for (auto& childStyled : box->styled->children)
    {
        auto childBox = std::make_shared<LayoutBox>(childStyled);

        const std::string* disp = childStyled->styles.get("display");
        std::string display = disp ? *disp : "block";

        if (childStyled->dom->type == NodeType::Text)
        {
            int h = measureTextHeight(childStyled->dom->name, contentWidth);
            childBox->rect.x = contentX;
            childBox->rect.y = cursorY;
            childBox->rect.width = contentWidth;
            childBox->rect.height = h;
            cursorY += h;
        }
        else
        {
            int marginTop = parsePx(childStyled->styles.get("margin-top"), 0);
            int marginBottom = parsePx(childStyled->styles.get("margin-bottom"), 0);
            int paddingTop = parsePx(childStyled->styles.get("padding-top"), 0);
            int paddingBottom = parsePx(childStyled->styles.get("padding-bottom"), 0);
            int paddingLeft = parsePx(childStyled->styles.get("padding-left"), 0);
            int paddingRight = parsePx(childStyled->styles.get("padding-right"), 0);

            int boxX = contentX;
            int boxY = cursorY + marginTop;
            int boxWidth = contentWidth;

            const std::string* wProp = childStyled->styles.get("width");
            int explicitWidth = parsePx(wProp, -1);
            if (explicitWidth > 0 && explicitWidth < contentWidth)
                boxWidth = explicitWidth;

            int childContentX = boxX + paddingLeft;
            int childContentY = boxY + paddingTop;
            int childContentWidth = boxWidth - paddingLeft - paddingRight;
            if (childContentWidth < 0)
                childContentWidth = 0;

            childBox->rect.x = boxX;
            childBox->rect.y = boxY;
            childBox->rect.width = boxWidth;

            layoutChildren(childBox, childContentX, childContentY, childContentWidth);

            int childrenBottom = childContentY;
            for (auto& c : childBox->children)
            {
                int bottom = c->rect.y + c->rect.height;
                if (bottom > childrenBottom)
                    childrenBottom = bottom;
            }

            int contentHeight = childrenBottom - childContentY;
            int totalHeight = paddingTop + contentHeight + paddingBottom;
            if (totalHeight < 20)
                totalHeight = 20;

            childBox->rect.height = totalHeight;
            cursorY = boxY + totalHeight + marginBottom;
        }

        box->children.push_back(childBox);
    }
}

std::shared_ptr<LayoutBox> LayoutEngine::Build(std::shared_ptr<StyledNode> root, int viewportWidth)
{
    auto rootBox = std::make_shared<LayoutBox>(root);
    rootBox->rect.x = 0;
    rootBox->rect.y = 0;
    rootBox->rect.width = viewportWidth;

    int contentX = 0;
    int contentY = 0;
    int contentWidth = viewportWidth;

    layoutChildren(rootBox, contentX, contentY, contentWidth);

    int maxBottom = 0;
    for (auto& c : rootBox->children)
    {
        int bottom = c->rect.y + c->rect.height;
        if (bottom > maxBottom)
            maxBottom = bottom;
    }
    rootBox->rect.height = maxBottom;

    return rootBox;
}
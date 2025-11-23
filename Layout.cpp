#include "Layout.h"
#include <string>
#include <algorithm>
#include <cctype>

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

static bool isWhitespaceText(const std::string& s)
{
    for (unsigned char c : s)
        if (!std::isspace(c))
            return false;
    return true;
}

static bool isNonRenderedTag(const std::string& tag)
{
    if (tag == "html") return true;
    if (tag == "head") return true;
    if (tag == "meta") return true;
    if (tag == "style") return true;
    if (tag == "script") return true;
    if (tag == "link") return true;
    if (tag == "title") return true;
    return false;
}

static bool isBrTag(const std::string& tag)
{
    return tag == "br";
}

static int measureTextHeightApprox(const std::string& text, int width)
{
    if (text.empty())
        return 0;

    int lineHeight = 18;
    int charsPerLine = width / 8;
    if (charsPerLine <= 0)
        charsPerLine = 1;

    int len = (int)text.size();
    int lines = len / charsPerLine + 1;

    return lines * lineHeight;
}

static void layoutChildren(std::shared_ptr<LayoutBox> box, int contentX, int contentY, int contentWidth)
{
    int cursorY = contentY;

    for (auto& childStyled : box->styled->children)
    {
        auto dom = childStyled->dom;

        if (dom->type == NodeType::Element && isNonRenderedTag(dom->name))
        {
            auto tmpBox = std::make_shared<LayoutBox>(childStyled);
            layoutChildren(tmpBox, contentX, cursorY, contentWidth);
            continue;
        }

        if (dom->type == NodeType::Element && isBrTag(dom->name))
        {
            int lh = 18;
            cursorY += lh;
            continue;
        }

        if (dom->type == NodeType::Text)
        {
            if (isWhitespaceText(dom->name))
                continue;

            auto childBox = std::make_shared<LayoutBox>(childStyled);

            int h = measureTextHeightApprox(dom->name, contentWidth);
            if (h <= 0) h = 18;

            childBox->rect.x = contentX;
            childBox->rect.y = cursorY;
            childBox->rect.width = contentWidth;
            childBox->rect.height = h;

            cursorY += h;
            box->children.push_back(childBox);
            continue;
        }

        auto childBox = std::make_shared<LayoutBox>(childStyled);

        int marginTop = parsePx(childStyled->styles.get("margin-top"), 8);
        int marginBottom = parsePx(childStyled->styles.get("margin-bottom"), 8);
        int paddingTop = parsePx(childStyled->styles.get("padding-top"), 4);
        int paddingBottom = parsePx(childStyled->styles.get("padding-bottom"), 4);
        int paddingLeft = parsePx(childStyled->styles.get("padding-left"), 8);
        int paddingRight = parsePx(childStyled->styles.get("padding-right"), 8);

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
        if (totalHeight < 24)
            totalHeight = 24;

        childBox->rect.height = totalHeight;
        cursorY = boxY + totalHeight + marginBottom;

        box->children.push_back(childBox);
    }
}

static std::shared_ptr<StyledNode> findBody(std::shared_ptr<StyledNode> root)
{
    if (!root) return nullptr;

    if (root->dom->type == NodeType::Element && root->dom->name == "body")
        return root;

    for (auto& ch : root->children)
    {
        auto r = findBody(ch);
        if (r) return r;
    }

    return root;
}

std::shared_ptr<LayoutBox> LayoutEngine::Build(std::shared_ptr<StyledNode> root, int viewportWidth)
{
    auto bodyStyled = findBody(root);
    if (!bodyStyled)
        bodyStyled = root;

    auto rootBox = std::make_shared<LayoutBox>(bodyStyled);
    rootBox->rect.x = 10;
    rootBox->rect.y = 0;
    rootBox->rect.width = viewportWidth - 20;
    if (rootBox->rect.width < 100)
        rootBox->rect.width = viewportWidth;

    int contentX = rootBox->rect.x;
    int contentY = rootBox->rect.y;
    int contentWidth = rootBox->rect.width;

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
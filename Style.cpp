#include "Style.h"

static bool matchesRule(const CSSRule& rule, const DomNode& node)
{
    if (node.type != NodeType::Element)
        return false;

    const std::string& sel = rule.selector;
    if (sel.empty())
        return false;

    if (sel[0] == '#')
    {
        const DomAttribute* idAttr = node.getAttribute("id");
        if (!idAttr) return false;
        return idAttr->value == sel.substr(1);
    }
    else if (sel[0] == '.')
    {
        return node.hasClass(sel.substr(1));
    }
    else
    {
        return node.name == sel;
    }
}

static void applyUserAgentDefaults(const DomNode& dom, StyleMap& styles)
{
    if (dom.type != NodeType::Element)
        return;

    const std::string& tag = dom.name;

    if (!styles.has("display"))
    {
        if (tag == "span" || tag == "a" || tag == "b" || tag == "i" || tag == "strong" || tag == "em")
            styles.setIfEmpty("display", "inline");
        else
            styles.setIfEmpty("display", "block");
    }
}

static void applyRules(const CSSStyleSheet& sheet, StyledNode& styled)
{
    if (styled.dom->type != NodeType::Element)
        return;

    for (const auto& rule : sheet.rules) {
        if (!matchesRule(rule, *styled.dom))
            continue;

        for (const auto& decl : rule.declarations) {
            styled.styles.setFromCascade(decl.property, decl.value, rule.specificity, rule.order);
        }
    }
}

static void applyInheritance(std::shared_ptr<StyledNode> parent, StyledNode& child)
{
    if (!parent)
        return;

    const char* inheritable[] = {
        "color",
        "font-size",
        "font-family"
    };

    for (const char* name : inheritable)
    {
        if (!child.styles.has(name))
        {
            const std::string* pv = parent->styles.get(name);
            if (pv)
                child.styles.setIfEmpty(name, *pv);
        }
    }
}

static std::shared_ptr<StyledNode> buildStyledRecursive(std::shared_ptr<DomNode> domNode,
    const CSSStyleSheet& sheet,
    std::shared_ptr<StyledNode> parent)
{
    auto styled = std::make_shared<StyledNode>(domNode);
    if (parent)
        styled->parent = parent;

    applyUserAgentDefaults(*domNode, styled->styles);
    applyRules(sheet, *styled);
    applyInheritance(parent, *styled);

    for (auto& childDom : domNode->children)
    {
        auto childStyled = buildStyledRecursive(childDom, sheet, styled);
        styled->children.push_back(childStyled);
    }

    return styled;
}

std::shared_ptr<StyledNode> StyleEngine::BuildTree(std::shared_ptr<DomNode> domRoot,
    const CSSStyleSheet& sheet)
{
    return buildStyledRecursive(domRoot, sheet, nullptr);
}
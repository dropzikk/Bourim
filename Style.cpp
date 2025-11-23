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

static void applyRules(const CSSStyleSheet& sheet, StyledNode& styled) {
    if (styled.dom->type != NodeType::Element)
        return;

    for (const auto& rule : sheet.rules) {
        if (!matchesRule(rule, *styled.dom))
            continue;

        for (const auto& decl : rule.declarations) {
            styled.styles.set(decl.property, decl.value);
        }
    }
}

static std::shared_ptr<StyledNode> buildStyledRecursive(std::shared_ptr<DomNode> domNode,
    const CSSStyleSheet& sheet) {
    auto styled = std::make_shared<StyledNode>(domNode);

    applyRules(sheet, *styled);

    for (auto& childDom : domNode->children) {
        auto childStyled = buildStyledRecursive(childDom, sheet);
        styled->children.push_back(childStyled);
    }

    return styled;
}

std::shared_ptr<StyledNode> StyleEngine::BuildTree(std::shared_ptr<DomNode> domRoot,
    const CSSStyleSheet& sheet) {
    return buildStyledRecursive(domRoot, sheet);
}
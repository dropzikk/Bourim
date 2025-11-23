#include "CssExtractor.h"

static void walk(std::shared_ptr<DomNode> node, std::vector<std::string>& out)
{
    if (node->type == NodeType::Element && node->name == "style")
    {
        for (auto& child : node->children)
        {
            if (child->type == NodeType::Text)
                out.push_back(child->name);
        }
    }

    for (auto& child : node->children)
        walk(child, out);
}

std::vector<std::string> CssExtractor::ExtractStyles(std::shared_ptr<DomNode> root)
{
    std::vector<std::string> styles;
    walk(root, styles);
    return styles;
}
#include "CssExtractor.h"

static void walkStyles(std::shared_ptr<DomNode> node, std::vector<std::string>& out)
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
        walkStyles(child, out);
}

std::vector<std::string> CssExtractor::ExtractStyles(std::shared_ptr<DomNode> root)
{
    std::vector<std::string> res;
    walkStyles(root, res);
    return res;
}

static void walkLinks(std::shared_ptr<DomNode> node, std::vector<std::string>& out)
{
    if (node->type == NodeType::Element && node->name == "link")
    {
        const DomAttribute* rel = node->getAttribute("rel");
        const DomAttribute* href = node->getAttribute("href");

        if (rel && href)
        {
            std::string r = rel->value;
            for (auto& c : r) c = (char)tolower((unsigned char)c);
            if (r == "stylesheet")
                out.push_back(href->value);
        }
    }

    for (auto& child : node->children)
        walkLinks(child, out);
}

std::vector<std::string> CssExtractor::ExtractStyleLinks(std::shared_ptr<DomNode> root)
{
    std::vector<std::string> res;
    walkLinks(root, res);
    return res;
}
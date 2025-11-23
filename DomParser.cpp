#include "DomParser.h"
#include <stack>

std::shared_ptr<DomNode> DomParser::Parse(const std::vector<HtmlToken>& tokens)
{
    std::shared_ptr<DomNode> root = std::make_shared<DomNode>(NodeType::Element, "document");
    std::stack<std::shared_ptr<DomNode>> stack;
    stack.push(root);

    for (const auto& token : tokens)
    {
        switch (token.type)
        {
        case HtmlTokenType::OpenTag:
        {
            auto node = std::make_shared<DomNode>(NodeType::Element, token.data);
            for (const auto& a : token.attributes)
                node->attributes.push_back({ a.name, a.value });

            stack.top()->children.push_back(node);
            stack.push(node);
        }
        break;

        case HtmlTokenType::CloseTag:
        {
            if (!stack.empty())
                stack.pop();
        }
        break;

        case HtmlTokenType::SelfClosingTag:
        {
            auto node = std::make_shared<DomNode>(NodeType::Element, token.data);
            for (const auto& a : token.attributes)
                node->attributes.push_back({ a.name, a.value });

            stack.top()->children.push_back(node);
        }
        break;

        case HtmlTokenType::Text:
        {
            auto node = std::make_shared<DomNode>(NodeType::Text, token.data);
            stack.top()->children.push_back(node);
        }
        break;
        }
    }

    return root;
}
#include "HtmlTokenizer.h"
#include <cctype>

std::vector<HtmlToken> HtmlTokenizer::Tokenize(const std::string& html)
{
    std::vector<HtmlToken> tokens;
    size_t i = 0;
    size_t len = html.size();

    while (i < len)
    {
        if (html[i] != '<')
        {
            std::string text;
            while (i < len && html[i] != '<')
                text.push_back(html[i++]);

            if (!text.empty())
                tokens.push_back({ HtmlTokenType::Text, text });

            continue;
        }

        i++;

        bool isClose = false;
        if (i < len && html[i] == '/')
        {
            isClose = true;
            i++;
        }

        std::string tag;

        while (i < len && html[i] != '>' && !isspace(html[i]))
            tag.push_back(html[i++]);

        while (i < len && html[i] != '>')
            i++;

        if (i < len) i++;

        if (tag.empty()) continue;

        if (isClose)
            tokens.push_back({ HtmlTokenType::CloseTag, tag });
        else if (!tag.empty() && tag.back() == '/')
        {
            tag.pop_back();
            tokens.push_back({ HtmlTokenType::SelfClosingTag, tag });
        }
        else
            tokens.push_back({ HtmlTokenType::OpenTag, tag });
    }

    return tokens;
}
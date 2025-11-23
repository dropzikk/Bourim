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
                tokens.push_back({ HtmlTokenType::Text, text, {} });

            continue;
        }

        i++;

        bool isClose = false;
        if (i < len && html[i] == '/')
        {
            isClose = true;
            i++;
        }

        while (i < len && std::isspace((unsigned char)html[i]))
            i++;

        std::string tagName;
        while (i < len &&
            !std::isspace((unsigned char)html[i]) &&
            html[i] != '>' &&
            html[i] != '/')
        {
            tagName.push_back(html[i++]);
        }

        std::vector<HtmlAttribute> attrs;

        while (i < len && html[i] != '>' && html[i] != '/')
        {
            while (i < len && std::isspace((unsigned char)html[i]))
                i++;

            if (i >= len || html[i] == '>' || html[i] == '/')
                break;

            std::string attrName;
            while (i < len &&
                !std::isspace((unsigned char)html[i]) &&
                html[i] != '=' &&
                html[i] != '>' &&
                html[i] != '/')
            {
                attrName.push_back(html[i++]);
            }

            while (i < len && std::isspace((unsigned char)html[i]))
                i++;

            std::string attrValue;

            if (i < len && html[i] == '=')
            {
                i++;
                while (i < len && std::isspace((unsigned char)html[i]))
                    i++;

                if (i < len && (html[i] == '"' || html[i] == '\''))
                {
                    char quote = html[i++];
                    while (i < len && html[i] != quote)
                        attrValue.push_back(html[i++]);
                    if (i < len && html[i] == quote)
                        i++;
                }
                else
                {
                    while (i < len &&
                        !std::isspace((unsigned char)html[i]) &&
                        html[i] != '>' &&
                        html[i] != '/')
                    {
                        attrValue.push_back(html[i++]);
                    }
                }
            }

            if (!attrName.empty())
                attrs.push_back({ attrName, attrValue });
        }

        bool selfClosing = false;
        if (i < len && html[i] == '/')
        {
            selfClosing = true;
            while (i < len && html[i] != '>')
                i++;
        }

        if (i < len && html[i] == '>')
            i++;

        if (tagName.empty())
            continue;

        if (isClose)
            tokens.push_back({ HtmlTokenType::CloseTag, tagName, {} });
        else if (selfClosing)
            tokens.push_back({ HtmlTokenType::SelfClosingTag, tagName, attrs });
        else
            tokens.push_back({ HtmlTokenType::OpenTag, tagName, attrs });
    }

    return tokens;
}
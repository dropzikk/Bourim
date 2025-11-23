#include "CssParser.h"
#include <stdexcept>

CSSStyleSheet CssParser::Parse(const std::vector<CSSToken>& tokens)
{
    CSSStyleSheet sheet;
    size_t i = 0;
    size_t n = tokens.size();

    auto expect = [&](CSSTokenType type) {
        if (i >= n || tokens[i].type != type)
            throw std::runtime_error("CSS parse error");
        return tokens[i++];
        };

    while (i < n)
    {
        if (tokens[i].type == CSSTokenType::EOFToken)
            break;
        if (tokens[i].type != CSSTokenType::Ident &&
            tokens[i].type != CSSTokenType::Hash &&
            tokens[i].type != CSSTokenType::Delim)
        {
            i++;
            continue;
        }
        std::string selector = tokens[i].value;
        i++;
        if (tokens[i].type != CSSTokenType::CurlyOpen) {
            i++;
            continue;
        }
        i++;
        CSSRule rule;
        rule.selector = selector;
        while (i < n && tokens[i].type != CSSTokenType::CurlyClose)
        {
            if (tokens[i].type != CSSTokenType::Ident) {
                i++;
                continue;
            }

            std::string property = tokens[i].value;
            i++;

            expect(CSSTokenType::Colon);

            std::string value;

            while (i < n && tokens[i].type != CSSTokenType::Semicolon &&
                tokens[i].type != CSSTokenType::CurlyClose)
            {
                value += tokens[i].value;
                if (tokens[i].type == CSSTokenType::Ident ||
                    tokens[i].type == CSSTokenType::Number)
                    value += " ";

                i++;
            }

            rule.declarations.push_back({ property, value });

            if (tokens[i].type == CSSTokenType::Semicolon)
                i++;
        }

        if (tokens[i].type == CSSTokenType::CurlyClose)
            i++;

        sheet.rules.push_back(rule);
    }

    return sheet;
}
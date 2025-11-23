#include "CssParser.h"
#include <stdexcept>

static int ComputeSpecificity(const std::string& sel)
{
    if (sel.empty()) return 0;
    if (sel[0] == '#') return 100;
    if (sel[0] == '.') return 10;
    return 1;
}

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

        CSSRule rule;
        bool hasSelector = false;

        if (tokens[i].type == CSSTokenType::Ident)
        {
            rule.selector = tokens[i].value;
            i++;
            hasSelector = true;
        }
        else if (tokens[i].type == CSSTokenType::Hash)
        {
            rule.selector = "#" + tokens[i].value;
            i++;
            hasSelector = true;
        }
        else if (tokens[i].type == CSSTokenType::Delim && tokens[i].value == ".")
        {
            i++;
            if (i < n && tokens[i].type == CSSTokenType::Ident)
            {
                rule.selector = "." + tokens[i].value;
                i++;
                hasSelector = true;
            }
        }

        if (!hasSelector)
        {
            i++;
            continue;
        }

        rule.specificity = ComputeSpecificity(rule.selector);

        if (i >= n || tokens[i].type != CSSTokenType::CurlyOpen)
        {
            i++;
            continue;
        }

        i++;

        while (i < n && tokens[i].type != CSSTokenType::CurlyClose)
        {
            if (tokens[i].type != CSSTokenType::Ident)
            {
                i++;
                continue;
            }

            std::string property = tokens[i].value;
            i++;

            expect(CSSTokenType::Colon);

            std::string value;

            while (i < n &&
                tokens[i].type != CSSTokenType::Semicolon &&
                tokens[i].type != CSSTokenType::CurlyClose)
            {
                value += tokens[i].value;
                if (tokens[i].type == CSSTokenType::Ident ||
                    tokens[i].type == CSSTokenType::Number)
                    value += " ";

                i++;
            }

            rule.declarations.push_back({ property, value });

            if (i < n && tokens[i].type == CSSTokenType::Semicolon)
                i++;
        }

        if (i < n && tokens[i].type == CSSTokenType::CurlyClose)
            i++;

        sheet.rules.push_back(rule);
    }

    return sheet;
}
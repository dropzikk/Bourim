#include "CssTokenizer.h"
#include <cctype>

std::vector<CSSToken> CssTokenizer::Tokenize(const std::string& css)
{
    std::vector<CSSToken> tokens;
    size_t i = 0;
    size_t n = css.size();

    auto add = [&](CSSTokenType t, std::string v) {
        tokens.push_back({ t, v });
        };

    while (i < n)
    {
        char c = css[i];
        if (isspace(c)) { i++; continue; }
        if (isalpha(c) || c == '-' || c == '_')
        {
            std::string ident;
            while (i < n && (isalnum(css[i]) || css[i] == '-' || css[i] == '_'))
                ident.push_back(css[i++]);

            add(CSSTokenType::Ident, ident);
            continue;
        }
        if (isdigit(c))
        {
            std::string num;
            while (i < n && isdigit(css[i]))
                num.push_back(css[i++]);

            add(CSSTokenType::Number, num);
            continue;
        }
        if (c == '#')
        {
            i++;
            std::string name;
            while (i < n && (isalnum(css[i]) || css[i] == '-' || css[i] == '_'))
                name.push_back(css[i++]);

            add(CSSTokenType::Hash, name);
            continue;
        }
        switch (c)
        {
        case '{': add(CSSTokenType::CurlyOpen, "{"); break;
        case '}': add(CSSTokenType::CurlyClose, "}"); break;
        case ':': add(CSSTokenType::Colon, ":"); break;
        case ';': add(CSSTokenType::Semicolon, ";"); break;
        default:
            add(CSSTokenType::Delim, std::string(1, c));
            break;
        }
        i++;
    }

    add(CSSTokenType::EOFToken, "");
    return tokens;
}
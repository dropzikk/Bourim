#ifndef CSS_TOKENIZER_H
#define CSS_TOKENIZER_H

#include <string>
#include <vector>

enum class CSSTokenType {
    Ident,
    Hash,
    String,
    Number,
    Delim,
    Colon,
    Semicolon,
    CurlyOpen,
    CurlyClose,
    EOFToken
};

struct CSSToken {
    CSSTokenType type;
    std::string value;
};

class CssTokenizer {
public:
    static std::vector<CSSToken> Tokenize(const std::string& css);
};

#endif
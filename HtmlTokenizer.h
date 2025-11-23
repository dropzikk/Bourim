#ifndef HTML_TOKENIZER_H
#define HTML_TOKENIZER_H

#include <string>
#include <vector>

enum class HtmlTokenType {
    Text,
    OpenTag,
    CloseTag,
    SelfClosingTag
};

struct HtmlToken {
    HtmlTokenType type;
    std::string value;
};

class HtmlTokenizer {
public:
    static std::vector<HtmlToken> Tokenize(const std::string& html);
};

#endif
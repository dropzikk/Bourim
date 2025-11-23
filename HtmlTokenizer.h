#ifndef HTML_TOKENIZER_H
#define HTML_TOKENIZER_H

#include <string>
#include <vector>

struct HtmlAttribute {
    std::string name;
    std::string value;
};

enum class HtmlTokenType {
    Text,
    OpenTag,
    CloseTag,
    SelfClosingTag
};

struct HtmlToken {
    HtmlTokenType type;
    std::string data;
    std::vector<HtmlAttribute> attributes;
};

class HtmlTokenizer {
public:
    static std::vector<HtmlToken> Tokenize(const std::string& html);
};

#endif
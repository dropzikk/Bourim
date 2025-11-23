#ifndef CSS_PARSER_H
#define CSS_PARSER_H

#include "CssTokenizer.h"
#include "CssOM.h"

class CssParser {
public:
    static CSSStyleSheet Parse(const std::vector<CSSToken>& tokens);
};

#endif
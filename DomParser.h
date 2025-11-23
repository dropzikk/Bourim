#ifndef DOM_PARSER_H
#define DOM_PARSER_H

#include "HtmlTokenizer.h"
#include "Dom.h"

class DomParser {
public:
    static std::shared_ptr<DomNode> Parse(const std::vector<HtmlToken>& tokens);
};

#endif
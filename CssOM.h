#ifndef CSS_OM_H
#define CSS_OM_H

#include <string>
#include <vector>

struct CSSDeclaration {
    std::string property;
    std::string value;
};

struct CSSRule {
    std::string selector;
    std::vector<CSSDeclaration> declarations;
    int specificity = 0;
    int order = 0;
};

class CSSStyleSheet {
public:
    std::vector<CSSRule> rules;
};

#endif
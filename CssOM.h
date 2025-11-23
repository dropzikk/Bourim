#ifndef CSS_OM_H
#define CSS_OM_H

#include <string>
#include <vector>
#include <map>

struct CSSDeclaration {
    std::string property;
    std::string value;
};

struct CSSRule {
    std::string selector;
    std::vector<CSSDeclaration> declarations;
};

class CSSStyleSheet {
public:
    std::vector<CSSRule> rules;
};

#endif
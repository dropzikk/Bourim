#ifndef CSS_EXTRACTOR_H
#define CSS_EXTRACTOR_H

#include "Dom.h"
#include <vector>
#include <string>
#include <memory>

class CssExtractor {
public:
    static std::vector<std::string> ExtractStyles(std::shared_ptr<DomNode> root);
    static std::vector<std::string> ExtractStyleLinks(std::shared_ptr<DomNode> root);
};

#endif
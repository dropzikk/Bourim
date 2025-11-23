#ifndef CSS_EXTRACTOR_H
#define CSS_EXTRACTOR_H

#include "Dom.h"
#include <vector>
#include <string>

class CssExtractor {
public:
    static std::vector<std::string> ExtractStyles(std::shared_ptr<DomNode> root);
};

#endif
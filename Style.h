#ifndef STYLE_H
#define STYLE_H

#include <string>
#include <map>
#include <memory>
#include <vector>
#include "Dom.h"
#include "CssOM.h"

struct StyleMap {
    std::map<std::string, std::string> properties;

    void set(const std::string& name, const std::string& value) {
        properties[name] = value;
    }

    const std::string* get(const std::string& name) const {
        auto it = properties.find(name);
        if (it == properties.end()) return nullptr;
        return &it->second;
    }
};

class StyledNode {
public:
    std::shared_ptr<DomNode> dom;
    StyleMap styles;
    std::vector<std::shared_ptr<StyledNode>> children;

    StyledNode(std::shared_ptr<DomNode> node)
        : dom(node) {
    }
};

class StyleEngine {
public:
    static std::shared_ptr<StyledNode> BuildTree(std::shared_ptr<DomNode> domRoot,
        const CSSStyleSheet& sheet);
};

#endif
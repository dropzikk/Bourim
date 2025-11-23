#ifndef STYLE_H
#define STYLE_H

#include <string>
#include <map>
#include <memory>
#include <vector>
#include "Dom.h"
#include "CssOM.h"

struct StyleValue {
    std::string value;
    int specificity;
    int order;
};

struct StyleMap {
    std::map<std::string, StyleValue> properties;

    void setFromCascade(const std::string& name, const std::string& value, int specificity, int order) {
        auto it = properties.find(name);
        if (it == properties.end() ||
            specificity > it->second.specificity ||
            (specificity == it->second.specificity && order >= it->second.order))
        {
            properties[name] = { value, specificity, order };
        }
    }

    void setIfEmpty(const std::string& name, const std::string& value) {
        if (properties.find(name) == properties.end()) {
            properties[name] = { value, -1, -1 };
        }
    }

    const std::string* get(const std::string& name) const {
        auto it = properties.find(name);
        if (it == properties.end()) return nullptr;
        return &it->second.value;
    }

    bool has(const std::string& name) const {
        return properties.find(name) != properties.end();
    }
};

class StyledNode {
public:
    std::shared_ptr<DomNode> dom;
    StyleMap styles;
    std::vector<std::shared_ptr<StyledNode>> children;
    std::weak_ptr<StyledNode> parent;

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
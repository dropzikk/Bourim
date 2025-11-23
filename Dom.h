#ifndef DOM_H
#define DOM_H

#include <string>
#include <vector>
#include <memory>

enum class NodeType {
    Element,
    Text
};

struct DomAttribute {
    std::string name;
    std::string value;
};

class DomNode {
public:
    NodeType type;
    std::string name;
    std::vector<DomAttribute> attributes;
    std::vector<std::shared_ptr<DomNode>> children;

    DomNode(NodeType t, const std::string& n)
        : type(t), name(n) {
    }

    const DomAttribute* getAttribute(const std::string& key) const
    {
        for (const auto& a : attributes)
            if (a.name == key)
                return &a;
        return nullptr;
    }

    bool hasClass(const std::string& className) const
    {
        const DomAttribute* attr = getAttribute("class");
        if (!attr) return false;

        const std::string& v = attr->value;
        size_t start = 0;

        while (start < v.size())
        {
            while (start < v.size() && v[start] == ' ')
                start++;
            if (start >= v.size())
                break;

            size_t end = start;
            while (end < v.size() && v[end] != ' ')
                end++;

            if (v.substr(start, end - start) == className)
                return true;

            start = end;
        }

        return false;
    }
};

#endif
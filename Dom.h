#ifndef DOM_H
#define DOM_H

#include <string>
#include <vector>
#include <memory>

enum class NodeType {
    Element,
    Text
};

class DomNode {
public:
    NodeType type;
    std::string name;
    std::vector<std::shared_ptr<DomNode>> children;

    DomNode(NodeType t, const std::string& n)
        : type(t), name(n) {
    }
};

#endif
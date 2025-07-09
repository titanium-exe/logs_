#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>


enum DecafType {
    TYPE_INT,
    TYPE_BOOL,
    TYPE_STRING,
    TYPE_VOID,
    TYPE_UNKNOWN
};

inline std::string typeToString(DecafType type) {
    switch (type) {
        case TYPE_INT: return "int";
        case TYPE_BOOL: return "bool";
        case TYPE_STRING: return "string";
        case TYPE_VOID: return "void";
        default: return "unknown";
    }
}


struct SymDescriptor {
    std::string name;
    DecafType   type;
    int         lineDeclared;

    SymDescriptor() : name(""), type(TYPE_UNKNOWN), lineDeclared(-1) {}

    SymDescriptor(std::string n, DecafType t, int line)
        : name(std::move(n)), type(t), lineDeclared(line) {}
};




class SymbolTable {
    std::unordered_map<std::string, SymDescriptor> table;

public:
    bool insert(const std::string &name, DecafType type, int line) {
        if (table.count(name)) return false;  
        table[name] = SymDescriptor(name, type, line);
        return true;
    }

    void overwrite(const std::string &name, DecafType type, int line) {
        table[name] = SymDescriptor(name, type, line);
    }

    SymDescriptor* lookup(const std::string &name) {
        if (table.count(name)) return &table[name];
        return nullptr;
    }

    void print() const {
        for (const auto &pair : table) {
            std::cout << "  " << pair.first << " : "
                      << typeToString(pair.second.type)
                      << " (declared on line " << pair.second.lineDeclared << ")\n";
        }
    }
};



class SymbolStack {
    std::vector<SymbolTable> scopes;

public:
    void push() {
        scopes.push_back(SymbolTable());
    }

    void pop() {
        if (!scopes.empty()) {
            scopes.pop_back();
        } else {
            std::cerr << "Warning: tried to pop empty symbol stack\n";
        }
    }

    bool insert(const std::string &name, DecafType type, int line) {
        if (scopes.empty()) push(); // ensure at least one scope
        return scopes.back().insert(name, type, line);
    }

    SymDescriptor* lookup(const std::string &name) {
        for (int i = scopes.size() - 1; i >= 0; --i) {
            SymDescriptor* result = scopes[i].lookup(name);
            if (result) return result;
        }
        return nullptr;
    }
    void overwrite(const std::string &name, DecafType type, int line) {
        if (scopes.empty()) push();
        scopes.back().overwrite(name, type, line);
    }
    
    void print() const {
        std::cout << "===== Symbol Table Stack =====\n";
        for (int i = scopes.size() - 1; i >= 0; --i) {
            std::cout << "Scope " << i << ":\n";
            scopes[i].print();
        }
        std::cout << "==============================\n";
    }
};





extern SymbolStack gSym;

#endif // SYMBOLTABLE_H

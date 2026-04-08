#ifndef SKIP_LIST_HPP
#define SKIP_LIST_HPP

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <climits>
#include <new>

template<typename T>
class SkipList {
private:
    struct Node {
        int level;
        bool has_value;
        alignas(T) unsigned char value_data[sizeof(T)];
        Node* forward[1]; // Flexible array member

        T& getValue() {
            return *reinterpret_cast<T*>(value_data);
        }

        const T& getValue() const {
            return *reinterpret_cast<const T*>(value_data);
        }

        // For head node
        Node(int lvl) : level(lvl), has_value(false) {
            for (int i = 0; i < lvl; ++i) forward[i] = nullptr;
        }

        // For normal nodes
        Node(const T& val, int lvl) : level(lvl), has_value(true) {
            new (value_data) T(val);
            for (int i = 0; i < lvl; ++i) forward[i] = nullptr;
        }

        ~Node() {
            if (has_value) {
                getValue().~T();
            }
        }
    };

    Node* head;
    int maxLevel;
    int currentLevel;
    unsigned int seed;

    Node* createNode(int lvl) {
        void* mem = ::operator new(sizeof(Node) + sizeof(Node*) * (lvl - 1));
        return new (mem) Node(lvl);
    }

    Node* createNode(const T& val, int lvl) {
        void* mem = ::operator new(sizeof(Node) + sizeof(Node*) * (lvl - 1));
        return new (mem) Node(val, lvl);
    }

    void destroyNode(Node* node) {
        node->~Node();
        ::operator delete(node);
    }

    int randomLevel() {
        int lvl = 1;
        while (lvl < maxLevel) {
            seed = seed * 1103515245 + 12345;
            if ((seed >> 16) & 1) {
                lvl++;
            } else {
                break;
            }
        }
        return lvl;
    }

    bool isEqual(const T& a, const T& b) const {
        return !(a < b) && !(b < a);
    }

public:
    SkipList() : maxLevel(32), currentLevel(1), seed(123456789) {
        head = createNode(maxLevel);
    }

    ~SkipList() {
        Node* current = head;
        while (current != nullptr) {
            Node* next = current->forward[0];
            destroyNode(current);
            current = next;
        }
    }

    void insert(const T & item) {
        Node* update[32];
        for (int i = 0; i < 32; ++i) update[i] = nullptr;
        
        Node* current = head;

        for (int i = currentLevel - 1; i >= 0; i--) {
            while (current->forward[i] != nullptr && current->forward[i]->getValue() < item) {
                current = current->forward[i];
            }
            update[i] = current;
        }

        current = current->forward[0];

        if (current != nullptr && isEqual(current->getValue(), item)) {
            return; // Item already exists
        }

        int lvl = randomLevel();

        if (lvl > currentLevel) {
            for (int i = currentLevel; i < lvl; i++) {
                update[i] = head;
            }
            currentLevel = lvl;
        }

        Node* newNode = createNode(item, lvl);

        for (int i = 0; i < lvl; i++) {
            newNode->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = newNode;
        }
    }

    bool search(const T & item) {
        Node* current = head;

        for (int i = currentLevel - 1; i >= 0; i--) {
            while (current->forward[i] != nullptr && current->forward[i]->getValue() < item) {
                current = current->forward[i];
            }
        }

        current = current->forward[0];

        if (current != nullptr && isEqual(current->getValue(), item)) {
            return true;
        }

        return false;
    }

    void deleteItem(const T & item) {
        Node* update[32];
        for (int i = 0; i < 32; ++i) update[i] = nullptr;
        
        Node* current = head;

        for (int i = currentLevel - 1; i >= 0; i--) {
            while (current->forward[i] != nullptr && current->forward[i]->getValue() < item) {
                current = current->forward[i];
            }
            update[i] = current;
        }

        current = current->forward[0];

        if (current != nullptr && isEqual(current->getValue(), item)) {
            for (int i = 0; i < currentLevel; i++) {
                if (update[i]->forward[i] != current) {
                    break;
                }
                update[i]->forward[i] = current->forward[i];
            }

            destroyNode(current);

            while (currentLevel > 1 && head->forward[currentLevel - 1] == nullptr) {
                currentLevel--;
            }
        }
    }
};

#endif
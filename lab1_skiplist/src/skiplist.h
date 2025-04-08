#include <stdlib.h>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <xmmintrin.h>
#include <immintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include <bit>
#include <functional>
#include <mutex>
#include <vector>
#include <random>

#include <atomic>

typedef std::chrono::high_resolution_clock Clock;

// Key is an 8-byte integer
typedef uint64_t Key;

int compare_(const Key& a, const Key& b) {
    if (a < b) {
        return -1;
    } else if (a > b) {
        return +1;
    } else {
        return 0;
    }
}

template<typename Key>
class SkipList {
   private:
    struct Node;

   public:
    SkipList(int max_level = 16, float probability = 0.5);

    void Insert(const Key& key); // Insertion function (to be implemented by students)
    bool Contains(const Key& key) const; // Lookup function (to be implemented by students)
    std::vector<Key> Scan(const Key& key, const int scan_num); // Range query function (to be implemented by students)
    bool Delete(const Key& key); // Delete function (to be implemented by students)
    void Print() const;

private:
    int RandomLevel(); // Generates a random level for new nodes (to be implemented by students)

    Node* head; // Head node (starting point of the SkipList)
    int max_level; // Maximum level in the SkipList
    float probability; // Probability factor for level increase
};

// SkipList Node structure
template<typename Key>
struct SkipList<Key>::Node {
    Key key;
    std::vector<Node*> next; // Pointer array for multiple levels

    // Constructor for Node
    Node(Key key, int level);
};

template <typename Key>
SkipList<Key>::Node::Node(Key key, int level)
    : key(key), next(level, nullptr) {}

// Generate a random level for new nodes
template<typename Key>
int SkipList<Key>::RandomLevel() {
    int level = 1;

    static thread_local std::default_random_engine engine(std::random_device{}()); // 난수생성
    static thread_local std::uniform_real_distribution<float> dist(0.0,1.0); // 0~1 사이로 제한

    while (dist(engine) < probability && level < max_level){
        level++; // 난수가 0.5 미만이며 level이 max_level 미만일경우 레벨 증가
    }
    
    return level; // Default return value (students should modify this)
}

// Constructor for SkipList
template<typename Key>
SkipList<Key>::SkipList(int max_level, float probability)
    : max_level(max_level), probability(probability) {
        head = new Node(Key(), max_level);
}

// Insert function (inserts a key into SkipList)
template<typename Key>
void SkipList<Key>::Insert(const Key& key) {
    Node* current = head;
    std::vector<Node*> Update(max_level, nullptr);

    for (int level = max_level-1; level >= 0; --level){
        while(current->next[level] != nullptr && current->next[level]->key < key){
            current = current->next[level];
        }
        Update[level] = current;
    }
    if(current->next[0] != nullptr && current->next[0]->key == key){
        return;
    }

    int node_level = RandomLevel();

    Node* new_node = new Node(key, node_level);

    for (int i = 0; i < node_level; ++i){
        new_node->next[i] = Update[i]->next[i];
        Update[i]->next[i] = new_node;
    }
}

// Delete function (removes a key from SkipList)
template<typename Key>
bool SkipList<Key>::Delete(const Key& key)  {
    Node* current = head;
    std::vector<Node *> Update(max_level, nullptr);

    for (int level = max_level - 1; level >= 0; --level)
    {
        while (current->next[level] != nullptr && current->next[level]->key < key)
        {
            current = current->next[level];
        }
        Update[level] = current;
    }
    if(current->next[0] != nullptr && current->next[0]->key == key){
        Node *target = current->next[0];
        for(int i = 0; i < target->next.size(); ++i){
            Update[i]->next[i] = target->next[i];
        }
        delete target;
        return true;
    } 
    return false;
}

// Lookup function (checks if a key exists in SkipList)
template<typename Key>  
bool SkipList<Key>::Contains(const Key& key) const {
    Node* current = head;

    for (int level = max_level - 1; level >= 0; --level)
    {
        while (current->next[level] != nullptr && current->next[level]->key < key)
        {
            current = current->next[level];
        }
    }
    if(current->next[0] != nullptr &&current->next[0]->key == key){
        return true;
    }
    return false;
}

// Range query function (retrieves scan_num keys starting from key)
template<typename Key>
std::vector<Key> SkipList<Key>::Scan(const Key& key, const int scan_num) {
    Node* current = head;
    std::vector<Key> result;

    for (int level = max_level - 1; level >= 0; --level)
    {
        while (current->next[level] != nullptr && current->next[level]->key < key)
        {
            current = current->next[level];
        }
    }
    current = current->next[0];
    while(current != nullptr && result.size() < scan_num){
        result.push_back(current->key);
        current = current->next[0];
    }
    return result;
}

template <typename Key>
void SkipList<Key>::Print() const
{
    std::cout << "SkipList Structure:\n";

    for (int level = max_level - 1; level >= 0; --level)
    {
        Node *node = head->next[level];
        std::cout << "Level " << level << ": ";
        while (node != nullptr)
        {
            std::cout << node->key << " ";
            node = node->next[level];
        }
        std::cout << "\n";
    }
}
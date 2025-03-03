#pragma once
#include <vector>
#include <map>
#include <iostream>
#include <cstdlib> // For malloc

namespace entities {

template<typename T>
class Pool {
public:
    Pool(size_t size) : pool_size(size) {
        // Allocate memory for the pool
        memory_pool = static_cast<T*>(malloc(sizeof(T) * size));
        first_free_index = 0;
        first_unallocated_index = 0;
    }

    ~Pool() {
        // Call destructors for all active items
        for (auto& pair : active_items) {
            pair.second->~T();
        }
        // Free the allocated memory
        free(memory_pool);
    }

    template<typename... Args>
    T* Create(Args&&... args) {
        std::cout << "Creating item with args. first_free_index: " << first_free_index 
                  << ", first_unallocated_index: " << first_unallocated_index 
                  << ", active_items.size(): " << active_items.size() << std::endl;
        
        T* new_item = nullptr;
        
        // Check if we have free slots before the unallocated area
        if (first_free_index < first_unallocated_index) {
            // Reuse a previously freed slot
            new_item = &memory_pool[first_free_index];
            std::cout << "Reusing item" << std::endl;
            
            // Construct the item in place with arguments
            new (new_item) T(std::forward<Args>(args)...);
            
            // Add to active items
            active_items[first_free_index] = new_item;
            
            // Find the next free index
            first_free_index++;
            while (first_free_index < first_unallocated_index && 
                   active_items.find(first_free_index) != active_items.end()) {
                first_free_index++;
            }
            
            std::cout << "Reused item at index " << (first_free_index - 1) << std::endl;
        } 
        else if (first_unallocated_index < pool_size) {
            // Use an unallocated slot
            new_item = &memory_pool[first_unallocated_index];
            
            // Construct the item in place with arguments
            new (new_item) T(std::forward<Args>(args)...);
            
            // Add to active items
            active_items[first_unallocated_index] = new_item;
            
            // Update indices
            first_free_index = first_unallocated_index + 1;
            first_unallocated_index++;
            
            std::cout << "Created new item at index " << (first_unallocated_index - 1) << std::endl;
        } 
        else {
            std::cout << "Pool is full" << std::endl;
        }
        
        return new_item;
    }

    T* Create() {
        std::cout << "Creating item. first_free_index: " << first_free_index 
                  << ", first_unallocated_index: " << first_unallocated_index 
                  << ", active_items.size(): " << active_items.size() << std::endl;
        
        T* new_item = nullptr;
        
        // Check if we have free slots before the unallocated area
        if (first_free_index < first_unallocated_index) {
            // Reuse a previously freed slot
            new_item = &memory_pool[first_free_index];
            std::cout << "Reusing item" << std::endl;
            
            // Construct the item in place
            new (new_item) T();
            
            // Add to active items
            active_items[first_free_index] = new_item;
            
            // Find the next free index
            first_free_index++;
            while (first_free_index < first_unallocated_index && 
                   active_items.find(first_free_index) != active_items.end()) {
                first_free_index++;
            }
            
            std::cout << "Reused item at index " << (first_free_index - 1) << std::endl;
        } 
        else if (first_unallocated_index < pool_size) {
            // Use an unallocated slot
            new_item = &memory_pool[first_unallocated_index];
            
            // Construct the item in place
            new (new_item) T();
            
            // Add to active items
            active_items[first_unallocated_index] = new_item;
            
            // Update indices
            first_free_index = first_unallocated_index + 1;
            first_unallocated_index++;
            
            std::cout << "Created new item at index " << (first_unallocated_index - 1) << std::endl;
        } 
        else {
            std::cout << "Pool is full" << std::endl;
        }
        
        return new_item;
    }

    void Destroy(T* item) {
        if (!item) return;
        
        // Find the index of the item in the pool
        size_t index = static_cast<size_t>(item - memory_pool);
        
        // Verify the item is within our pool
        if (index >= pool_size) {
            std::cerr << "Warning: Trying to destroy an item not in the pool" << std::endl;
            return;
        }
        
        // Check if the item is active
        if (active_items.find(index) == active_items.end()) {
            std::cerr << "Warning: Trying to destroy an inactive item" << std::endl;
            return;
        }
        
        std::cout << "Destroying item at index " << index 
                  << ", active_items.size() before: " << active_items.size() << std::endl;
        
        // Call the destructor
        item->~T();
        
        // Remove from active items
        active_items.erase(index);
        
        // Update first_free_index if needed
        if (index < first_free_index) {
            first_free_index = index;
        }
        
        // Update first_unallocated_index if needed
        if (index == first_unallocated_index - 1) {
            first_unallocated_index--;
            
            // Find the new first_unallocated_index
            while (first_unallocated_index > 0 && 
                   active_items.find(first_unallocated_index - 1) == active_items.end()) {
                first_unallocated_index--;
            }
        }
        
        std::cout << "After destroy: first_free_index: " << first_free_index 
                  << ", first_unallocated_index: " << first_unallocated_index
                  << ", active_items.size(): " << active_items.size() << std::endl;
    }

    void Clear() {
        // Call destructors for all active items
        for (auto& pair : active_items) {
            pair.second->~T();
        }
        
        // Clear the active items map
        active_items.clear();
        
        // Reset indices
        first_free_index = 0;
        first_unallocated_index = 0;
    }

    bool IsActive(T* item) const {
        if (!item) return false;
        
        // Find the index of the item in the pool
        size_t index = static_cast<size_t>(item - memory_pool);
        
        // Verify the item is within our pool
        if (index >= pool_size) {
            return false;
        }
        
        // Check if the item is in the active_items map
        return active_items.find(index) != active_items.end();
    }

    size_t GetActiveCount() const {
        return active_items.size();
    }

    T* Get(size_t index) const {
        auto it = active_items.find(index);
        if (it != active_items.end()) {
            return it->second;
        }
        return nullptr;
    }

    std::vector<T*> GetAll() const {
        std::vector<T*> result;
        result.reserve(active_items.size());
        
        for (const auto& pair : active_items) {
            result.push_back(pair.second);
        }
        
        return result;
    }

    size_t Size() const {
        return active_items.size();
    }

    T* GetPtr() const {
        return memory_pool;
    }

private:
    T* memory_pool;
    size_t pool_size;
    size_t first_free_index;       // Index of the first free slot
    size_t first_unallocated_index; // Index of the first unallocated slot
    std::map<size_t, T*> active_items; // Map of active items by index
};

} // namespace entities 
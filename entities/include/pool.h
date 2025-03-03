#pragma once
#include <vector>
#include <map>
#include <iostream>
#include <cstdlib> // For malloc
#include "utils/logger.h"
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
        utils::Logger::log("Creating item with args. first_free_index: " +
         std::to_string(first_free_index) + ", first_unallocated_index: " +
          std::to_string(first_unallocated_index) + ", active_items.size(): " +
           std::to_string(active_items.size()), utils::Logger::WARN_LEVEL::INFO);
        
        T* new_item = nullptr;
        
        // Check if we have free slots before the unallocated area
        if (first_free_index < first_unallocated_index) {
            // Reuse a previously freed slot
            new_item = &memory_pool[first_free_index];
            utils::Logger::log("Reusing item", utils::Logger::WARN_LEVEL::INFO);
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
            
            utils::Logger::log("Reused item at index " + std::to_string(first_free_index - 1),
             utils::Logger::WARN_LEVEL::INFO);
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
            
            utils::Logger::log("Created new item at index " + std::to_string(first_unallocated_index - 1),
             utils::Logger::WARN_LEVEL::INFO);
        } 
        else {
            utils::Logger::log("Pool is full", utils::Logger::WARN_LEVEL::INFO);
        }
        
        return new_item;
    }

    T* Create() {
        utils::Logger::log("Creating item. first_free_index: " + std::to_string(first_free_index) +
         ", first_unallocated_index: " + std::to_string(first_unallocated_index) +
          ", active_items.size(): " + std::to_string(active_items.size()),
           utils::Logger::WARN_LEVEL::INFO);
        
        T* new_item = nullptr;
        
        // Check if we have free slots before the unallocated area
        if (first_free_index < first_unallocated_index) {
            // Reuse a previously freed slot
            new_item = &memory_pool[first_free_index];
            utils::Logger::log("Reusing item", utils::Logger::WARN_LEVEL::INFO);
            
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
            
            utils::Logger::log("Reused item at index " + std::to_string(first_free_index - 1),
             utils::Logger::WARN_LEVEL::INFO);
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
            
            utils::Logger::log("Created new item at index " + std::to_string(first_unallocated_index - 1),
             utils::Logger::WARN_LEVEL::INFO);
        } 
        else {
            utils::Logger::log("Pool is full", utils::Logger::WARN_LEVEL::INFO);
        }
        
        return new_item;
    }

    void Destroy(T* item) {
        if (!item) return;
        
        // Find the index of the item in the pool
        size_t index = static_cast<size_t>(item - memory_pool);
        
        // Verify the item is within our pool
        if (index >= pool_size) {
            utils::Logger::log("Warning: Trying to destroy an item not in the pool",
             utils::Logger::WARN_LEVEL::ERROR);
            return;
        }
        
        // Check if the item is active
        if (active_items.find(index) == active_items.end()) {
            utils::Logger::log("Warning: Trying to destroy an inactive item",
             utils::Logger::WARN_LEVEL::ERROR);
            return;
        }
        
        utils::Logger::log("Destroying item at index " + std::to_string(index) +
         ", active_items.size() before: " + std::to_string(active_items.size()),
          utils::Logger::WARN_LEVEL::INFO);
        
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
        
        utils::Logger::log("After destroy: first_free_index: " + std::to_string(first_free_index) +
         ", first_unallocated_index: " + std::to_string(first_unallocated_index) +
          ", active_items.size(): " + std::to_string(active_items.size()),
           utils::Logger::WARN_LEVEL::INFO);
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
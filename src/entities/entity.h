#pragma once
#include <string>

struct Entity {
    Entity(std::string uuid): m_uuid(uuid) {}
    ~Entity() {}        
    std::string m_uuid;
};


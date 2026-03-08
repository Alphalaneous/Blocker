#pragma once

#include <Geode/Geode.hpp>
#include "Enums.hpp"

using namespace geode::prelude;

class BlockManager {
public:
    static BlockManager* get();

    void removeUser(BlockType type, unsigned int accountID);
    void addUser(BlockType type, unsigned int accountID, ZStringView username);

    bool isBlocked(BlockType type, unsigned int accountID);
    
    void loadUsers();

    void saveUsers();

    std::unordered_map<unsigned int, std::string>& getUsers(BlockType type);

protected:

    std::unordered_map<unsigned int, std::unordered_map<BlockType, std::unordered_map<unsigned int, std::string>>> m_blockedUsers;
};
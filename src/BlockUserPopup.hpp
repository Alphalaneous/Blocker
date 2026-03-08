#pragma once

#include <Geode/Geode.hpp>
#include "Enums.hpp"

using namespace geode::prelude;

namespace alpha::blocker {

class BlockUserPopup : public geode::Popup {
public:

    static BlockUserPopup* create(unsigned int accountID, ZStringView username);
protected:

    bool init(unsigned int accountID, ZStringView username);

    void addToggle(ZStringView text, BlockType type, ZStringView info);

    unsigned int m_accountID;
    std::string m_username;
    CCNode* m_togglesContainer;

    std::unordered_map<BlockType, bool> m_toggles;
    std::unordered_map<BlockType, bool> m_togglesChanged;
};

}
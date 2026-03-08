#pragma once

#include <Geode/Geode.hpp>
#include "Enums.hpp"

using namespace geode::prelude;

namespace alpha::blocker {

class BlockPopup;

class UserCell : public CCLayerColor {
public:
    static UserCell* create(BlockPopup* popup, unsigned int accountID, ZStringView name, BlockType page);
protected:

    bool init(BlockPopup* popup, unsigned int accountID, ZStringView name, BlockType page);
    void draw() override;

    unsigned int m_accountID;
    std::string m_name;
    BlockType m_page;
    BlockPopup* m_popup;
};

}
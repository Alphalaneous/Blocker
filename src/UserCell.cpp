#include <Geode/ui/Button.hpp>
#include "UserCell.hpp"
#include "BlockPopup.hpp"

using namespace alpha::blocker;

UserCell* UserCell::create(BlockPopup* popup, unsigned int accountID, ZStringView name, BlockType page) {
    auto ret = new UserCell();
    if (ret->init(popup, accountID, std::move(name), page)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool UserCell::init(BlockPopup* popup, unsigned int accountID, ZStringView name, BlockType page) {
    if (!CCLayerColor::init()) return false;

    setOpacity(255);
    m_popup = popup;
    m_accountID = accountID;
    m_name = std::move(name);
    m_page = page;

    setContentSize({346, 30});
    setAnchorPoint({0.5f, 0.5f});
    ignoreAnchorPointForPosition(false);

    auto label = CCLabelBMFont::create(m_name.c_str(), "bigFont.fnt");
    label->limitLabelWidth(250, 0.5f, 0.1f);
    label->setID("username-label");

    auto labelBtn = Button::createWithNode(label, [accountID] (auto sender) {
        ProfilePage::create(accountID, false)->show();
    });
    labelBtn->setID("username-button");

    labelBtn->setPosition({10 + labelBtn->getContentWidth()/2, getContentHeight() / 2});

    auto unlockBtnSprite = ButtonSprite::create("Unblock", "bigFont.fnt", "GJ_button_05.png", 0.6f);
    unlockBtnSprite->setScale(0.5f);

    unlockBtnSprite->setID("unlock-button-sprite");

    auto unblockBtn = Button::createWithNode(unlockBtnSprite, [this] (auto sender) {
        m_popup->removeUser(m_accountID);
    });

    unblockBtn->setID("unlock-button");

    unblockBtn->setAnchorPoint({0.5f, 0.5f});
    unblockBtn->setPosition({getContentWidth() - unblockBtn->getContentWidth() / 2 - 10, getContentHeight() / 2});

    addChild(labelBtn);
    addChild(unblockBtn);

    return true;
}

void UserCell::draw() {
    CCLayerColor::draw();
    cocos2d::ccDrawColor4B(0, 0, 0, 75);
    glLineWidth(2.0f);
    cocos2d::ccDrawLine({ 1.0f, 0.0f }, { getContentWidth() - 1.0f, 0.0f });
    cocos2d::ccDrawLine({ 1.0f, getContentHeight() }, { getContentWidth() - 1.0f, getContentHeight() });
}
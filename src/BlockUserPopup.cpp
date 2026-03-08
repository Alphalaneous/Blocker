#include <Geode/ui/Button.hpp>
#include "BlockUserPopup.hpp"
#include "BlockManager.hpp"
#include "Enums.hpp"

using namespace alpha::blocker;

BlockUserPopup* BlockUserPopup::create(unsigned int accountID, ZStringView username) {
    auto ret = new BlockUserPopup();
    if (ret->init(accountID, std::move(username))) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool BlockUserPopup::init(unsigned int accountID, ZStringView username) {
    if (!Popup::init({300, 250}, "square01_001.png")) return false;

    m_closeBtn->setVisible(false);
    m_closeBtn->setID("close-button");

    m_accountID = accountID;
    m_username = std::move(username);

    setTitle(fmt::format("Confirm Block", m_username));

    auto textArea = TextArea::create(fmt::format("Are you sure you want to block <cg>{}</c>?", m_username), "chatFont.fnt", 1.f, m_size.width - 40, {0.49999f, 0.49999f}, 16, false);
    textArea->setPosition({m_size.width / 2, m_size.height - 60});
    textArea->setID("info-text-area");

    m_mainLayer->addChild(textArea);

    m_togglesContainer = CCNode::create();
    m_togglesContainer->setAnchorPoint({0.5f, 0.5f});
    m_togglesContainer->setLayout(ScrollLayer::createDefaultListLayout(5));
    m_togglesContainer->setPosition(m_size / 2 - CCPoint{0, 15});
    m_togglesContainer->setContentSize({m_size.width - 50, 30});
    m_togglesContainer->setZOrder(2);
    m_togglesContainer->setID("toggles-container");

    m_mainLayer->addChild(m_togglesContainer);

    addToggle("Block User", BlockType::Profiles, fmt::format("<cg>{}</c> will no longer be able to:\n- <cy>View your profile</c>\n- <cb>Send messages</c>\n- <cp>Send friend requests</c>\n- <cr>Messages from this user will be removed</c>", m_username));
    addToggle("Block Levels", BlockType::Levels, fmt::format("You will no longer be able to view any levels by <cg>{}</c>", m_username));
    addToggle("Block Comments", BlockType::Comments, fmt::format("You will no longer be able to view any comments by <cg>{}</c>", m_username));

    m_togglesContainer->updateLayout();

    auto bg = NineSlice::create("square02_001.png");
    bg->setContentSize(m_togglesContainer->getContentSize() + CCSize{13, 10});
    bg->setPosition(m_togglesContainer->getPosition());
    bg->setOpacity(127);
    bg->setZOrder(1);
    bg->setID("toggles-background");

    m_mainLayer->addChild(bg);

    auto bottomContainer = CCNode::create();
    bottomContainer->setAnchorPoint({0.5f, 0.f});
    bottomContainer->setContentSize({300, 30});
    bottomContainer->setPosition({m_size.width / 2, 15});
    bottomContainer->setID("bottom-container");

    auto bottomLayout = RowLayout::create();
    bottomLayout->setGap(10);

    bottomContainer->setLayout(bottomLayout);

    auto backSpr = ButtonSprite::create("Back");
    backSpr->setID("back-sprite");

    auto backBtn = Button::createWithNode(backSpr, [this] (auto sender) {
        onClose(sender);
    });
    backBtn->setID("back-button");

    bottomContainer->addChild(backBtn);

    auto blockSpr = ButtonSprite::create("Block", "goldFont.fnt", "GJ_button_06.png");
    blockSpr->setID("block-sprite");

    auto blockBtn = Button::createWithNode(blockSpr, [this] (auto sender) {
        onClose(sender);

        if (m_togglesChanged[BlockType::Profiles]) {
            if (m_toggles[BlockType::Profiles]) {
                BlockManager::get()->addUser(BlockType::Profiles, m_accountID, m_username);
                GameLevelManager::get()->blockUser(m_accountID);
            }
            else {
                BlockManager::get()->removeUser(BlockType::Profiles, m_accountID);
                GameLevelManager::get()->unblockUser(m_accountID);
            }
        }

        if (m_togglesChanged[BlockType::Levels]) {
            if (m_toggles[BlockType::Levels]) {
                BlockManager::get()->addUser(BlockType::Levels, m_accountID, m_username);
            }
            else {
                BlockManager::get()->removeUser(BlockType::Levels, m_accountID);
            }
        }

        if (m_togglesChanged[BlockType::Comments]) {
            if (m_toggles[BlockType::Comments]) {
                BlockManager::get()->addUser(BlockType::Comments, m_accountID, m_username);
            }
            else {
                BlockManager::get()->removeUser(BlockType::Comments, m_accountID);
            }
            if (auto profile = typeinfo_cast<ProfilePage*>(GameLevelManager::get()->m_userInfoDelegate)) {
                profile->onUpdate(nullptr);
            }
        }
    });
    blockBtn->setID("block-button");

    bottomContainer->addChild(blockBtn);

    bottomContainer->updateLayout();

    m_mainLayer->addChild(bottomContainer);

    return true;
}

void BlockUserPopup::addToggle(ZStringView text, BlockType type, ZStringView info) {
    auto node = CCNode::create();
    node->setAnchorPoint({0.5f, 0.5f});
    node->setContentSize({m_togglesContainer->getContentWidth(), 25});
    node->setID(fmt::format("toggle-container-{}", static_cast<unsigned int>(type)));

    auto label = CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
    label->setAnchorPoint({0.f, 0.5f});
    label->setPosition({2, node->getContentHeight() / 2});
    label->setScale(0.5f);
    label->setID("toggle-label");

    node->addChild(label);

    auto menu = CCMenu::create();
    menu->setAnchorPoint({0.5f, 0.5f});
    menu->setContentSize(node->getContentSize());
    menu->setPosition(node->getContentSize() / 2);
    menu->ignoreAnchorPointForPosition(false);
    menu->setID("toggle-menu");

    node->addChild(menu);

    auto toggler = CCMenuItemExt::createTogglerWithStandardSprites(0.6f, [this, type] (CCMenuItemToggler* self) {
        m_toggles[type] = !self->m_toggled;
        m_togglesChanged[type] = true;
    });

    menu->setID("block-toggle");

    m_toggles[type] = BlockManager::get()->isBlocked(type, m_accountID);
    toggler->toggle(BlockManager::get()->isBlocked(type, m_accountID));

    toggler->setPosition({node->getContentWidth() - 11.3f, node->getContentHeight() / 2});

    menu->addChild(toggler);

    auto infoBtn = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_infoIcon_001.png", 0.5f, [text = std::string(text), info = std::string(info)] (auto sender) {
        createQuickPopup(text.c_str(), info, "OK", nullptr, nullptr);
    });
    infoBtn->setID("info-button");

    infoBtn->setPosition({label->getPositionX() + label->getScaledContentWidth() + 15, node->getContentHeight() / 2 - 1});

    menu->addChild(infoBtn);

    m_togglesContainer->addChild(node);
}
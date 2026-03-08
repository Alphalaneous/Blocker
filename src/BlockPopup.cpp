#include "BlockPopup.hpp"
#include "BlockManager.hpp"
#include "Enums.hpp"
#include "UserCell.hpp"

using namespace alpha::blocker;

PageButton* PageButton::create(BlockPopup* popup, ZStringView label, BlockType page) {
    auto ret = new PageButton();
    if (ret->init(popup, std::move(label), page)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool PageButton::init(BlockPopup* popup, ZStringView label, BlockType page) {

    m_popup = popup;
    m_page = page;

    m_selected = ButtonSprite::create(label.c_str(), "goldFont.fnt", "GJ_button_03.png");
    m_selected->setScale(0.7f);
    m_selected->setVisible(false);
    m_selected->setPosition(m_selected->getScaledContentSize() / 2);
    m_selected->setID("selected-sprite");

    m_deselected = ButtonSprite::create(label.c_str());
    m_deselected->setScale(0.7f);
    m_deselected->setPosition(m_deselected->getScaledContentSize() / 2);
    m_deselected->setID("deselected-sprite");
    
    auto container = CCNode::create();
    container->setAnchorPoint({0.5f, 0.5f});
    container->setContentSize(m_selected->getScaledContentSize());
    container->setID("sprite-containerc");

    container->addChild(m_selected);
    container->addChild(m_deselected);

    Button::initWithNode(container, [this] (auto self) {
        m_popup->selectButton(this);
    });

    return true;
}

void PageButton::select() {
    m_selected->setVisible(true);
    m_deselected->setVisible(false);
}

void PageButton::deselect() {
    m_selected->setVisible(false);
    m_deselected->setVisible(true);
}

BlockType PageButton::getPage() {
    return m_page;
}

BlockPopup::~BlockPopup() {
    GameLevelManager::get()->m_userListDelegate = nullptr;
}

BlockPopup* BlockPopup::create() {
    auto ret = new BlockPopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool BlockPopup::init() {
    if (!Popup::init({440, 290})) return false;

    GameLevelManager::get()->m_userListDelegate = this;

    m_noElasticity = true;
    setOpacity(150);
    
    auto label = CCLabelBMFont::create("Blocked", "bigFont.fnt");

    label->setAnchorPoint({0.5f, 1.f});
    label->setPosition({m_size.width / 2, m_size.height - 8});
    label->setScale(0.8f);
    label->setID("title-label");

    m_closeBtn->setSprite(CCSprite::createWithSpriteFrameName("GJ_backBtn_001.png"));
    m_closeBtn->setPosition({10, m_size.height - 12});
    m_closeBtn->setID("close-label");

    m_mainLayer->addChild(label);

    CCSize size = {346, 196};

    m_scrollLayer = ScrollLayer::create(size);
    m_scrollLayer->setContentSize(size);
    m_scrollLayer->ignoreAnchorPointForPosition(false);
    m_scrollLayer->m_contentLayer->setLayout(ScrollLayer::createDefaultListLayout(0));
    m_scrollLayer->setID("users-scroll-layer");

    auto border = Border::create(m_scrollLayer, {191, 114, 62, 255}, size);
    border->setPosition(m_size / 2 - CCPoint{0, 18});
    border->setContentSize(size);
    border->setSize(size);
    border->ignoreAnchorPointForPosition(false);
    border->setID("scroll-layer-border");

    auto bg = border->getChildByType<NineSlice>(0);
    bg->setScaleMultiplier(2.3f);

    m_mainLayer->addChild(border);

    auto buttonNode = CCNode::create();

    buttonNode->setContentSize({size.width - 20, 30});
    buttonNode->setAnchorPoint({0.5f, 1.f});
    buttonNode->setPosition({m_size.width / 2, m_size.height - 40});
    buttonNode->setID("tabs-node");

    auto profileButton = PageButton::create(this, "Profiles", BlockType::Profiles);
    profileButton->setID("profiles-button");

    m_buttons.push_back(profileButton);
    buttonNode->addChild(profileButton);

    auto levelButton = PageButton::create(this, "Levels", BlockType::Levels);
    levelButton->setID("levels-button");

    m_buttons.push_back(levelButton);
    buttonNode->addChild(levelButton);

    auto commentsButton = PageButton::create(this, "Comments", BlockType::Comments);
    commentsButton->setID("comments-button");

    m_buttons.push_back(commentsButton);
    buttonNode->addChild(commentsButton);

    auto buttonLayout = RowLayout::create();
    buttonLayout->setAxisAlignment(AxisAlignment::Between);
    buttonLayout->setGrowCrossAxis(true);
    
    buttonNode->setLayout(buttonLayout);
    buttonNode->updateLayout();

    m_mainLayer->addChild(buttonNode);

    m_refreshButton = Button::createWithSpriteFrameName("GJ_updateBtn_001.png", [this] (auto sender) {
        if (m_page == BlockType::Profiles) {
            refreshProfilePage();
        }
        else {
            auto& page = m_pages[m_page];
            page.clear();

            loadPageUsers(m_page);
            refreshPage();
        }
    });
    m_refreshButton->setPosition({m_size.width - 10, 12});
    m_refreshButton->setID("refresh-button");

    m_mainLayer->addChild(m_refreshButton);

    m_errorLabel = CCLabelBMFont::create("Something went wrong...", "goldFont.fnt");
    m_errorLabel->setScale(0.6f);
    m_errorLabel->setPosition(border->getContentSize() / 2);
    m_errorLabel->setZOrder(10);
    m_errorLabel->setID("error-label");

    border->addChild(m_errorLabel);

    m_loadingSpinner = LoadingSpinner::create(50.f);
    m_loadingSpinner->setPosition(border->getContentSize() / 2);
    m_loadingSpinner->setID("loading-spinner");

    border->addChild(m_loadingSpinner);

    auto stored = GameLevelManager::get()->getStoredUserList(UserListType::Blocked);

    if (!stored) {
        refreshProfilePage();
    }
    else {
        loadProfilePageUsers(stored);
    }

    loadPageUsers(BlockType::Comments);
    loadPageUsers(BlockType::Levels);
    
    selectButton(profileButton);

    return true;
}

void BlockPopup::getUserListFinished(cocos2d::CCArray* scores, UserListType type) {
    m_hadError = false;
    m_loading = false;
    loadProfilePageUsers(scores);
    if (m_page == BlockType::Profiles) {
        refreshPage();
        m_loadingSpinner->setVisible(false);
    }
}

void BlockPopup::getUserListFailed(UserListType type, GJErrorCode errorType) {
    m_hadError = true;
    m_loading = false;
    if (m_page == BlockType::Profiles) {
        m_errorLabel->setVisible(true);
        m_loadingSpinner->setVisible(false);
    }
}

void BlockPopup::userListChanged(cocos2d::CCArray* scores, UserListType type) {
    auto& page = m_pages[BlockType::Profiles];
    page.clear();

    loadProfilePageUsers(scores);
    refreshPage();
}

void BlockPopup::forceReloadList(UserListType type) {}

void BlockPopup::refreshProfilePage() {
    auto timeLeft = GameLevelManager::get()->getTimeLeft("upd_friends", 5.0);
    if (timeLeft - 1U < 5) return;

    auto& page = m_pages[BlockType::Profiles];
    page.clear();

    m_scrollLayer->m_contentLayer->removeAllChildren();

    m_loading = true;
    m_loadingSpinner->setVisible(true);

    GameLevelManager::get()->makeTimeStamp("upd_friends");
    GameLevelManager::get()->invalidateUserList(UserListType::Blocked, true);
    GameLevelManager::get()->getUserList(UserListType::Blocked);
}

void BlockPopup::removeUser(unsigned int accountID) {
    if (m_page == BlockType::Profiles) {
        GameLevelManager::get()->unblockUser(accountID);
    }

    auto& page = m_pages[m_page];
    page.erase(accountID);

    refreshPage();
    
    BlockManager::get()->removeUser(m_page, accountID);
}

void BlockPopup::loadProfilePageUsers(cocos2d::CCArray* scores) {
    auto& page = m_pages[BlockType::Profiles];
    auto& users = BlockManager::get()->getUsers(BlockType::Profiles);
    users.clear();

    for (const auto& v : scores->asExt<GJUserScore>()) {
        auto cell = UserCell::create(this, v->m_accountID, v->m_userName, BlockType::Profiles);
        page[v->m_accountID] = cell;
        users[v->m_accountID] = v->m_userName;
    }
}

void BlockPopup::loadPageUsers(BlockType type) {
    auto& page = m_pages[type];
    for (const auto& [k, v] : BlockManager::get()->getUsers(type)) {
        auto cell = UserCell::create(this, k, v, type);
        page[k] = cell;
    }
}

void BlockPopup::refreshPage() {
    m_errorLabel->setVisible(false);
    m_loadingSpinner->setVisible(false);

    if (m_page == BlockType::Profiles) {
        if (m_hadError) m_errorLabel->setVisible(true);
        if (m_loading) m_loadingSpinner->setVisible(true);
    }
    
    m_scrollLayer->m_contentLayer->removeAllChildren();
    auto& page = m_pages[m_page];

    bool odd = true;
    for (const auto& [id, cell] : page) {
        m_scrollLayer->m_contentLayer->addChild(cell);
        cell->setColor(odd ? ODD_COLOR : EVEN_COLOR);
        odd = !odd;
    }

    m_scrollLayer->m_contentLayer->updateLayout();
    m_scrollLayer->scrollToTop();
}

void BlockPopup::selectButton(PageButton* selected) {
    for (auto button : m_buttons) {
        button->deselect();
    }
    selected->select();
    m_page = selected->getPage();

    refreshPage();
}

void BlockPopup::onClose(CCObject* sender) {
    Popup::onClose(sender);
    auto profile = ProfilePage::create(GJAccountManager::get()->m_accountID, true);
    profile->m_noElasticity = true;
    profile->show();
}
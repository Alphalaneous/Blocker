#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Button.hpp>
#include "Enums.hpp"

using namespace geode::prelude;

namespace alpha::blocker {

class BlockPopup;
class UserCell;

class PageButton : public geode::Button {
public: 
    static PageButton* create(BlockPopup* popup, ZStringView label, BlockType page);

    void select();
    void deselect();
    BlockType getPage();

protected:
    bool init(BlockPopup* popup, ZStringView label, BlockType page);

    BlockType m_page;
    BlockPopup* m_popup;

    ButtonSprite* m_selected;
    ButtonSprite* m_deselected;
};

class BlockPopup : public geode::Popup, public UserListDelegate {
public:
    ~BlockPopup();
    static BlockPopup* create();

    void onClose(CCObject*) override;
    void selectButton(PageButton* selected);
    void refreshPage();

    void refreshProfilePage();
    void removeUser(unsigned int accountID);

protected:

    bool init() override;

    void loadProfilePageUsers(cocos2d::CCArray* scores);
    void loadPageUsers(BlockType type);

    void getUserListFinished(cocos2d::CCArray* scores, UserListType type) override;
    void getUserListFailed(UserListType type, GJErrorCode errorType) override;
    void userListChanged(cocos2d::CCArray* scores, UserListType type) override;
    void forceReloadList(UserListType type) override;

    ScrollLayer* m_scrollLayer;
    BlockType m_page;

    std::unordered_map<BlockType, std::unordered_map<unsigned int, Ref<UserCell>>> m_pages;

    std::vector<PageButton*> m_buttons;
    std::vector<unsigned int> m_unblockedProfiles;

    Ref<Button> m_refreshButton;

    CCLabelBMFont* m_errorLabel;
    LoadingSpinner* m_loadingSpinner;
    bool m_hadError;
    bool m_loading;

    static constexpr ccColor3B ODD_COLOR = {161, 88, 44};
    static constexpr ccColor3B EVEN_COLOR = {194, 114, 62};

};

}
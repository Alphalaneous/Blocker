#include <Geode/Geode.hpp>
#include <Geode/modify/FriendsProfilePage.hpp>
#include <Geode/modify/ProfilePage.hpp>
#include <Geode/modify/GameLevelManager.hpp>
#include <Geode/modify/BoomListView.hpp>
#include "BlockManager.hpp"
#include "BlockPopup.hpp"
#include "BlockUserPopup.hpp"
#include "Enums.hpp"

using namespace geode::prelude;
using namespace alpha::blocker;

class $modify(MyBoomListView, BoomListView) {

    bool init(cocos2d::CCArray* entries, TableViewCellDelegate* delegate, float height, float width, int page, BoomListType type, float y) {
		CCArray* entriesToRemove = CCArray::create();
			
		for (auto entry : entries->asExt()) {
			if (auto level = typeinfo_cast<GJGameLevel*>(entry)) {
				if (BlockManager::get()->isBlocked(BlockType::Levels, level->m_accountID)) {
					entriesToRemove->addObject(entry);
				}
			}
			if (auto level = typeinfo_cast<GJLevelList*>(entry)) {
				if (BlockManager::get()->isBlocked(BlockType::Levels, level->m_accountID)) {
					entriesToRemove->addObject(entry);
				}
			}
			if (auto comment = typeinfo_cast<GJComment*>(entry)) {
				if (BlockManager::get()->isBlocked(BlockType::Comments, comment->m_accountID)) {
					entriesToRemove->addObject(entry);
				}
			}
		}

		entries->removeObjectsInArray(entriesToRemove);

		return BoomListView::init(entries, type, width, height);
	}
};

class $modify(MyGameLevelManager, GameLevelManager) {
    void onUnblockUserCompleted(gd::string response, gd::string tag) {
		if (auto profile = typeinfo_cast<ProfilePage*>(m_userInfoDelegate)) {
			profile->onUpdate(nullptr);
		}
	}
};

class $modify(MyFriendsProfilePage, FriendsProfilePage) {

    bool init(UserListType type) {
		if (!FriendsProfilePage::init(type)) return false;

		auto topRightMenu = m_mainLayer->getChildByID("top-right-menu");
		auto blockedBtn = topRightMenu->getChildByID("blocked-button");
		blockedBtn->setVisible(false);

		return true;
	}
};

class $modify(MyProfilePage, ProfilePage) {

	struct Fields {
		Button* m_blockButton = nullptr;
	};

	void setupBlockButton() {
		auto fields = m_fields.self();
		if (fields->m_blockButton) {
			fields->m_blockButton->removeFromParent();
		}
		fields->m_blockButton = Button::createWithSpriteFrameName("accountBtn_blocked_001.png", [this] (auto sender) {
			onBlockUser(nullptr);
		});
		fields->m_blockButton->setScale(0.75f);
		fields->m_blockButton->setZOrder(2);
		fields->m_blockButton->setPosition(getContentSize() / 2 - CCPoint{0, 145 - fields->m_blockButton->getScaledContentHeight() / 2 - 6.5f});
		fields->m_blockButton->setID("block-button"_spr);

		m_mainLayer->addChild(fields->m_blockButton);
	}

    void getUserInfoFailed(int id) {
		ProfilePage::getUserInfoFailed(id);
		setupBlockButton();
	}
	
    void userInfoChanged(GJUserScore* score) {
		ProfilePage::userInfoChanged(score);
		if (score->m_friendReqStatus == 2) {
			setupBlockButton();
		}
	}

    void loadPageFromUserInfo(GJUserScore* score) {
		ProfilePage::loadPageFromUserInfo(score);
		auto fields = m_fields.self();
		if (fields->m_blockButton) {
			fields->m_blockButton->removeFromParent();
		}

		if (!m_ownProfile) return;

		auto bottomMenu = m_mainLayer->getChildByID("bottom-menu");

		auto btn = CCMenuItemExt::createSpriteExtraWithFrameName("accountBtn_blocked_001.png", 1, [this] (auto sender) {
			onClose(nullptr);
			BlockPopup::create()->show();
		});
		btn->setID("blocked-button"_spr);

		bottomMenu->insertBefore(btn, bottomMenu->getChildByID("settings-button"));

		bottomMenu->updateLayout();
	};

    void onBlockUser(cocos2d::CCObject* sender) {
		BlockUserPopup::create(m_accountID, m_usernameLabel->getString())->show();
	}
};
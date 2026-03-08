#include "BlockManager.hpp"
#include "Enums.hpp"

BlockManager* BlockManager::get() {
    static BlockManager instance;
    return &instance;
}

void BlockManager::removeUser(BlockType type, unsigned int accountID) {
    getUsers(type).erase(accountID);
}

void BlockManager::addUser(BlockType type, unsigned int accountID, ZStringView username) {
    getUsers(type)[accountID] = username;
}

bool BlockManager::isBlocked(BlockType type, unsigned int accountID) {
    return getUsers(type).find(accountID) != getUsers(type).end();
}

void BlockManager::loadUsers() {
    auto& container = Mod::get()->getSaveContainer();
    for (const auto& [k, v] : container) {
        auto accountIDRes = numFromString<unsigned int>(k);
        if (!accountIDRes) continue;
        for (const auto& [k1, v1] : v) {
            auto typeIDRes = numFromString<unsigned int>(k1);
            if (!typeIDRes) continue;
            for (const auto& [k2, v2] : v1) {
                auto blockedIDRes = numFromString<unsigned int>(k2);
                if (!blockedIDRes) continue;
                m_blockedUsers[accountIDRes.unwrap()][static_cast<BlockType>(typeIDRes.unwrap())][blockedIDRes.unwrap()] = v2.asString().unwrapOrDefault();
            }
        }
    }
}

void BlockManager::saveUsers() {
    auto& container = Mod::get()->getSaveContainer();
    for (const auto& [k, v] : m_blockedUsers) {
        for (const auto& [k1, v1] : v) {
            if (k1 == BlockType::Profiles) continue;
            for (const auto& [k2, v2] : v1) {
                container[numToString(k)][numToString(static_cast<unsigned int>(k1))][numToString(k2)] = v2;
            }
        }
    }
}

std::unordered_map<unsigned int, std::string>& BlockManager::getUsers(BlockType type) {
    return m_blockedUsers[GJAccountManager::get()->m_accountID][type];
}

$on_mod(DataLoaded) {
    BlockManager::get()->loadUsers();
}

$on_mod(DataSaved) {
    BlockManager::get()->saveUsers();
}

class BlockGrabber : public UserListDelegate {
public:

    static BlockGrabber* s_instance;

    BlockGrabber() {
        GameLevelManager::get()->m_userListDelegate = this;
        GameLevelManager::get()->invalidateUserList(UserListType::Blocked, true);
        GameLevelManager::get()->getUserList(UserListType::Blocked);
    }

    static BlockGrabber* start() {
        if (!s_instance) {
            s_instance = new BlockGrabber();
        }
        return s_instance;
    }

    void finish() {
        GameLevelManager::get()->m_userListDelegate = nullptr;
        delete s_instance;
        s_instance = nullptr;
    }

    void getUserListFinished(cocos2d::CCArray* scores, UserListType type) {
        auto& users = BlockManager::get()->getUsers(BlockType::Profiles);
        for (auto& user : scores->asExt<GJUserScore>()) {
            users[user->m_accountID] = user->m_userName;
        }
        finish();
    }

    void getUserListFailed(UserListType type, GJErrorCode errorType) {
        finish();
    }

    void userListChanged(cocos2d::CCArray* scores, UserListType type) {}
    void forceReloadList(UserListType type) {}

};

BlockGrabber* BlockGrabber::s_instance = nullptr;

$on_game(Loaded) {
    BlockGrabber::start();
}
// connection.h
#ifndef CONNECTION_H
#define CONNECTION_H
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "../public/role.hpp"

namespace logger {
class ServerRole : public Role {
    friend class Connection;

   private:
    std::vector<Message *> msgs;

   public:
    ServerRole(){};
    virtual ~ServerRole() {
        for (auto m : msgs) {
            delete m;
        }
    };
};

class Connection {
    friend class Server;

   public:
    const std::string ClientName;

   private:
    ServerRole *__root;
    std::vector<std::string> __role_labels;
    std::vector<std::string> __msg_labels;
    std::vector<ServerRole *> __roles;
    std::vector<Message *> __msgs;
    std::unordered_map<size_t, std::vector<ServerRole *>> __label_roles;
    std::unordered_map<size_t, std::vector<Message *>> __label_msgs;

   protected:
    size_t registerRole(ServerRole *role) {
        role->__id = __roles.size();
        if (role->__parent) role->__parent->__children[role->__name] = role;
        __roles.push_back(role);
        for (auto labelId : role->__labelIds) {
            __label_roles[labelId].push_back(role);
        }
        return role->__id;
    }
    size_t appendMsg(const Message &msg) {
        Message *m = new Message(msg);
        __msgs.push_back(m);
        for (auto labelId : msg.labelIds) {
            __label_msgs[labelId].push_back(m);
        }
        if (__roles.size() > msg.role_id)
            if (__roles[msg.role_id]) __roles[msg.role_id]->msgs.push_back(m);
        return __msgs.size() - 1;
    }
    size_t registerRoleLabel(const std::string &label_name) {
        __label_roles[__role_labels.size()] = std::vector<ServerRole *>();
        __role_labels.push_back(label_name);
        return __role_labels.size() - 1;
    }
    size_t registerMsgLabel(const std::string &label_name) {
        __label_msgs[__msg_labels.size()] = std::vector<Message *>();
        __msg_labels.push_back(label_name);
        return __msg_labels.size() - 1;
    }

   public:
    ServerRole *root() const { return __root; }
    const std::vector<std::string> &magLabels() const { return __msg_labels; }
    const std::vector<std::string> &roleLabels() const { return __role_labels; }
    const std::vector<ServerRole *> &roles() const { return __roles; }
    const std::vector<Message *> &messages() const { return __msgs; }

   public:
    Connection() = delete;
    Connection(const Connection &) = delete;

    explicit Connection(const std::string &client_name)
        : ClientName(client_name) {
        __roles.push_back(nullptr);
    };
    ~Connection() {
        std::vector<ServerRole *> tmp;
        for (auto r : __roles) {
            if (r)
                if (r->__parent == nullptr) {
                    tmp.push_back(r);
                }
            for (auto r : tmp) {
                delete r;
            }
        }
    };
};

}  // namespace logger

#endif  // CONNECTION_H
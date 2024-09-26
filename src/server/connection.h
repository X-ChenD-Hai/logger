// connection.h
#ifndef CONNECTION_H
#define CONNECTION_H
#include "../public/role.hpp"
#include <unordered_map>
#include <string>

namespace logger
{
    class ServerRole : public Role
    {
        friend class Connection;

    private:
        std::vector<Message *> msgs;

    public:
        ServerRole() {};
        virtual ~ServerRole()
        {
            for (auto m : msgs)
            {
                delete m;
            }
        };
    };

    class Connection
    {
    private:
        ServerRole *__root;
        std::string __client_name;
        size_t __client_id;
        std::vector<std::string> __role_labels;
        std::vector<std::string> __msg_labels;
        std::vector<ServerRole *> __roles;
        std::vector<Message *> __msgs;
        std::unordered_map<size_t, std::vector<ServerRole *>> __label_roles;
        std::unordered_map<size_t, std::vector<Message *>> __label_msgs;

    protected:
        size_t registerRole(const std::string &role_name, ServerRole *parent, const std::vector<size_t> &labelIds, size_t timestamp)
        {
            ServerRole *role = new ServerRole();
            role->__name = role_name;
            role->__parent = parent;
            role->__timestamp = timestamp;
            role->__labelIds = labelIds;
            role->__id = __roles.size();
            if (parent)
                parent->__children[role_name] = role;
            __roles.push_back(role);
            for (auto labelId : labelIds)
            {
                __label_roles[labelId].push_back(role);
            }
            return role->__id;
        }
        size_t newMsg(const Message &msg)
        {
            Message *m = new Message(msg);
            __msgs.push_back(m);
            for (auto labelId : msg.labelIds)
            {
                __label_msgs[labelId].push_back(m);
            }
            __roles[msg.role]->msgs.push_back(m);
            return __msgs.size() - 1;
        }
        size_t registerRoleLabel(const std::string &label_name)
        {
            __label_roles[__role_labels.size()] = std::vector<ServerRole *>();
            __role_labels.push_back(label_name);
            return __msg_labels.size() - 1;
        }
        size_t registerMsgLabel(const std::string &label_name)
        {
            __label_msgs[__msg_labels.size()] = std::vector<Message *>();
            __msg_labels.push_back(label_name);
            return __msg_labels.size() - 1;
        }

    public:
        Connection(const std::string &server_id);
        ~Connection()
        {
            for (auto r : __roles)
            {
                if (r->__parent == nullptr)
                    delete r;
            }
        };
    };

} // namespace logger

#endif // CONNECTION_H
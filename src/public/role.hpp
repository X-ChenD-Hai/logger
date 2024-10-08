// role.hpp
#ifndef ROLE_H
#define ROLE_H
#include <string_view>
#include <unordered_map>

#include "./message.hpp"

namespace logger {

class Role {
    friend class Client;
    friend class Connection;
    friend class ClientRole;
    friend class ServerRole;
    friend class Buffer;

   private:
    std::string __name{};
    size_t __timestamp{};
    RoleId __id = 0;
    Role* __parent = nullptr;
    std::unordered_map<std::string, Role*> __children{};
    std::vector<size_t> __labelIds{};
    Role(const Role&) = delete;
    Role() {}
    Role(std::string_view name, Role* parent)
        : __name(name), __parent(parent) {}

   public:
    const std::string& name() const { return __name; }
    const RoleId& id() const { return __id; }
    const Role* parent() const { return __parent; }
    const std::unordered_map<std::string, Role*>& children() const {
        return __children;
    }
    const std::vector<size_t>& labelIds() const { return __labelIds; }
    const size_t& timestamp() const { return __timestamp; }

   public:
    virtual ~Role() {
        for (auto it = __children.begin(); it != __children.end();
             it = __children.erase(it)) {
            delete it->second;
        }
    }
};

}  // namespace logger

#endif  // ROLE_H
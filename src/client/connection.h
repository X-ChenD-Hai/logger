// connection.h
#ifndef CONNECTION_H
#define CONNECTION_H
#include <cstddef>
#include <string>
#include <vector>

#include "../public/role.hpp"

namespace logger {
class Client;

class Connection {
   private:
    std::string server_id;

   protected:
    size_t client_id;

   public:
    std::size_t timeout_ms = 500;
    virtual bool send(Message msg, const std::vector<std::string>& labels) = 0;
    virtual void registerClient(Client* client) = 0;
    virtual void registerRole(Role* role,
                              const std::vector<std::string>& labels) = 0;
    virtual std::vector<size_t> getRoleLabelIds(
        std::vector<std::string> labels) = 0;
    virtual std::vector<size_t> getMsgLabelIds(
        std::vector<std::string> labels) = 0;

   protected:
    inline std::vector<size_t>& roleLabelIds(Role* role) {
        return role->__labelIds;
    }
    inline void setRoleId(Role* role, RoleId id) { role->__id = id; }

   public:
    Connection(std::string_view server_id) : server_id(server_id){};
    virtual ~Connection(){};
};

}  // namespace logger

#endif  // CONNECTION_H
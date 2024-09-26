// client.h
#ifndef CLIENT_H
#define CLIENT_H
#include <chrono>

#include "../public/role.hpp"
#include "./connection.h"

namespace logger {
class Client;

class ClientRole : public Role {
    friend class Client;
    ClientRole(const Client&) = delete;

   private:
    ClientRole(std::string_view name, std::vector<std::string> labels,
               Role* parent);

   public:
    virtual ~ClientRole() override {}
    size_t id();
    ClientRole* child(const std::string& name,
                      std::vector<std::string> labels = {});
    virtual void log(LogLevel level, std::string_view msg,
                     const std::vector<std::string>& labels = {},
                     size_t timestemp =
                         std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::system_clock::now() -
                             std::chrono::system_clock::time_point())
                             .count());
};

Client* getClient(const std::string& name,
                  const std::vector<Connection*>& conns);
class Client {
    friend class ClientRole;
    friend class Buffer;
    friend Client* getClient(const std::string& name,
                             const std::vector<Connection*>& conns);

   private:
    std::string __name;
    ClientRole* __root;
    static std::vector<Connection*> __conns;

   public:
    inline ClientRole* root() { return __root; };
    std::string name() { return __name; };

   public:
    ~Client();

   private:
    Client(const std::string& name, const std::vector<Connection*>& conns);
};
inline std::vector<Connection*> Client::__conns = {};
inline Client* getClient(const std::string& name,
                         const std::vector<Connection*>& conns) {
    static Client instance(name, conns);
    return &instance;
}

inline Client::Client(const std::string& name,
                      const std::vector<Connection*>& conns)
    : __name(name) {
    __conns = conns;
    for (auto conn : conns) conn->registerClient(this);
    __root = new ClientRole("root", {}, nullptr);
}
inline Client::~Client() {
    for (auto conn : __conns) {
        delete conn;
    }

    if (__root) delete __root;
}

inline ClientRole::ClientRole(std::string_view name,
                              std::vector<std::string> labels, Role* parent)
    : Role(name, parent) {
    for (auto conn : Client::__conns) {
        conn->registerRole(this, labels);
    }
}

inline size_t ClientRole::id() { return __id; }

inline ClientRole* ClientRole::child(const std::string& name,
                                     std::vector<std::string> labels) {
    ClientRole* ch;
    if (__children.find(name) == __children.end()) {
        ch = new ClientRole(name, labels, this);
        __children.insert({name, ch});
    } else
        ch = (ClientRole*)(__children[name]);
    return ch;
};

inline void ClientRole::log(LogLevel level, std::string_view msg,
                            const std::vector<std::string>& labels,
                            size_t timestemp) {
    for (auto conn : Client::__conns) {
        conn->send(Message{std::string(msg), timestemp, level, id()}, labels);
    }
}
};  // namespace logger

#endif  // CLIENT_H
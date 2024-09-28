// server.h
#ifndef SERVER_H
#define SERVER_H
#include <cstddef>
#include <iostream>
#include <string>
#include <unordered_map>

#include "../public/buffer.hpp"
#include "./connection.h"

namespace logger {
class Server {
   public:
    const std::string ServerId;

   protected:
    std::unordered_map<std::string, Connection*> __conns;

   protected:
    Connection* submitBuffer(Buffer&& buf, Connection* conn) {
        Connection* res_conn = nullptr;
        switch (buf.type()) {
            case logger::BufferType::ROLE: {
                std::cout << "recv role" << std::endl;
                auto role =
                    buf.role<ServerRole>([conn](size_t id) -> ServerRole* {
                        if (id >= conn->__roles.size())
                            return nullptr;
                        else
                            return conn->__roles.at(id);
                    });
                conn->registerRole(role);
                std::cout << role->name() << std::endl;
                std::cout << "send RoleId" << std::endl;
                send(Buffer(role->id()), conn);
                res_conn = conn;
                break;
            }
            case logger::BufferType::MSG: {
                std::cout << "recv msg" << std::endl;
                auto m = buf.msg();
                if (std::get<1>(m)) {
                    conn->appendMsg(std::get<0>(m));
                    std::cout << std::get<0>(m).msg << std::endl;
                }
                res_conn = conn;
                break;
            }
            case logger::BufferType::ROLE_LABEL:
            case logger::BufferType::MSG_LABEL: {
                std::cout << "recv label" << std::endl;
                auto l = buf.labels();
                std::vector<size_t> ids;
                for (auto s : l) {
                    size_t id;
                    if (buf.type() == logger::BufferType::ROLE_LABEL)
                        ids.push_back(id = conn->registerRoleLabel(s));
                    else
                        ids.push_back(id = conn->registerMsgLabel(s));
                    std::cout << s << "\t" << id << std::endl;
                }

                send(Buffer(ids, buf.type() == logger::BufferType::ROLE_LABEL),
                     conn);
                res_conn = conn;
                break;
            }
            case logger::BufferType::CLIENT_NAME: {
                std::cout << "recv client name" << std::endl;
                auto c = buf.clientName();
                if (std::get<1>(c)) {
                    if (__conns.find(std::get<0>(c)) != __conns.end()) {
                        delete __conns[std::get<0>(c)];
                    }
                    __conns[std::get<0>(c)] = new Connection(std::get<0>(c));
                    std::cout << std::get<0>(c) << std::endl;
                    res_conn = __conns[std::get<0>(c)];
                }
                break;
            }
            default: {
                std::cout << "Unknown buffer type received." << std::endl;
                res_conn = nullptr;
                break;
            }
        }
        return res_conn;
    }
    inline Connection* getConnectionByName(const std::string& name) {
        if (__conns.find(name) != __conns.end())
            return __conns[name];
        else
            return nullptr;
    }
    virtual void send(Buffer&& buf1, const Connection* conn) = 0;

   public:
    virtual void listening() = 0;

   public:
    Server() = delete;
    Server(const Server&) = delete;
    explicit Server(const std::string& id) : ServerId(id){};
    ~Server() {
        for (auto it = __conns.begin(); it != __conns.end();
             it = __conns.erase(it)) {
            delete it->second;
        }
    }
};

}  // namespace logger

#endif  // SERVER_H
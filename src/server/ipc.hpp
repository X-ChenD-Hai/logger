// ipc.hpp
#ifndef IPC_H
#define IPC_H

#include <cstddef>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "./server.h"
#include "libipc/ipc.h"
constexpr size_t DefaultTimeOot = 1;
namespace logger {
class IpcServer : public Server {
   private:
    std::unordered_map<std::string, ipc::channel*> client_cc_list;
    size_t time_out = DefaultTimeOot;
    std::vector<std::function<bool(void)>> tasks;

    ipc::channel* cc = nullptr;
    virtual void send(Buffer&& buf, const Connection* conn) override {
        if (conn)
            if (client_cc_list.find(conn->ClientName) != client_cc_list.end()) {
                if (client_cc_list[conn->ClientName]->wait_for_recv(2,
                                                                    time_out)) {
                    client_cc_list[conn->ClientName]->send(buf.data(),
                                                           buf.size());
                }
            } else {
            }
        else {
            if (cc->wait_for_recv(2, time_out)) {
                cc->send(buf.data(), buf.size());
            }
        }
    }

   public:
    virtual void listening() override {
        if (cc->wait_for_recv(2, time_out)) {
            auto buf = cc->recv();
            auto conn = submitBuffer(Buffer(buf.data(), buf.size()), nullptr);
            if (client_cc_list.find(conn->ClientName) == client_cc_list.end()) {
                client_cc_list[conn->ClientName] =
                    new ipc::channel((ServerId + "-" + conn->ClientName).data(),
                                     ipc::receiver | ipc::sender);
            }
        }
        for (auto client : client_cc_list) {
            if (client.second->wait_for_recv(2, time_out)) {
                auto buf = client.second->recv();
                auto conn = getConnectionByName(client.first);
                if (conn) submitBuffer(Buffer(buf.data(), buf.size()), conn);
            }
        }
    }
    explicit IpcServer(const std::string& id) : Server(id) {
        cc = new ipc::channel(ServerId.data(), ipc::receiver | ipc::sender);
    }
    virtual ~IpcServer() {
        if (cc) {
            delete cc;
            cc = nullptr;
        }
        for (auto it = client_cc_list.begin(); it != client_cc_list.end();
             it = client_cc_list.erase(it)) {
            delete it->second;
        }
    }
};

};  // namespace logger

#endif  // IPC_H
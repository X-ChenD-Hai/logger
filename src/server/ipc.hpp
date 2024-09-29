// ipc.hpp
#ifndef IPC_H
#define IPC_H

#include <cstddef>
#include <functional>
#include <queue>
#include <string>
#include <unordered_map>

#include "./server.h"
#include "libipc/ipc.h"
namespace logger {
constexpr size_t DefaultTimeOut = 1;
constexpr size_t OnesReSendBatchSize = 10;
class IpcServer : public Server {
   private:
    std::unordered_map<std::string, ipc::channel*> client_cc_list;
    size_t time_out = DefaultTimeOut;
    std::queue<std::function<bool(void)>> tasks;

    ipc::channel* cc = nullptr;

   public:
    virtual bool send(const Buffer& buf, const Connection* conn,
                      size_t resend_times) override {
        if (resend_times == 0) return true;
        if (!conn) {
            if (cc->wait_for_recv(2, time_out)) {
                cc->send(buf.data(), buf.size());
                return true;
            } else {
                tasks.push([this, buf = std::move(buf), conn,
                            resend_times]() mutable {
                    return this->send(std::move(buf), conn, resend_times - 1);
                });
            }
        }
        if (client_cc_list.find(conn->ClientName) != client_cc_list.end()) {
            if (client_cc_list[conn->ClientName]->wait_for_recv(2, time_out)) {
                client_cc_list[conn->ClientName]->send(buf.data(), buf.size());
                return true;
            } else {
                tasks.push([this, buf = std::move(buf), conn,
                            resend_times]() mutable {
                    return this->send(std::move(buf), conn, resend_times - 1);
                });
            }
        }
        return false;
    }

   public:
    virtual void listening() override {
        if (cc->wait_for_recv(2, time_out)) {
            auto buf = cc->recv(time_out);
            if (buf.data()) {
                auto conn =
                    submitBuffer(Buffer(buf.data(), buf.size()), nullptr);
                if (client_cc_list.find(conn->ClientName) ==
                    client_cc_list.end()) {
                    client_cc_list[conn->ClientName] = new ipc::channel(
                        (ServerId + "-" + conn->ClientName).data(),
                        ipc::receiver | ipc::sender);
                }
            }
        }
        for (auto client : client_cc_list) {
            if (client.second->wait_for_recv(2, time_out)) {
                auto buf = client.second->recv(time_out);
                if (buf.data()) {
                    auto conn = getConnectionByName(client.first);
                    if (conn)
                        submitBuffer(Buffer(buf.data(), buf.size()), conn);
                }
            }
        }
        size_t batch = OnesReSendBatchSize;
        while (!--batch && !tasks.empty()) {
            tasks.front()();
            tasks.pop();
        }
    }
    explicit IpcServer(const std::string& id) : Server(id) {
        cc = new ipc::channel(ServerId.data(), ipc::receiver | ipc::sender);
    }
    virtual ~IpcServer() override {
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
// ipc.hpp
#ifndef IPC_H
#define IPC_H
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include "../public/buffer.hpp"
#include "client.hpp"
#include "connection.h"
#include "libipc/ipc.h"

namespace logger {
class Client;
class IpcConnention : public Connection {
   private:
    static std::unordered_map<std::string, size_t> role_label_map;
    static std::unordered_map<std::string, size_t> msg_label_map;
    std::queue<std::function<void(void)>> tasks;

    ipc::channel* cc;

    std::thread t;
    bool stop = false;
    std::mutex mtx;
    std::condition_variable cv;

   public:
    IpcConnention(std::string_view server_id) : Connection(server_id) {
        cc = new ipc::channel(server_id.data(), ipc::receiver | ipc::sender);

        t = std::thread([this]() {
            while (!stop) {
                std::cout << "wait for new task" << std::endl;
                std::unique_lock<std::mutex> lk(mtx);
                cv.wait(lk, [this]() { return !tasks.empty() || stop; });
                if (tasks.empty()) continue;
                auto task = tasks.front();
                tasks.pop();
                lk.unlock();
                task();
            }
        });
    };
    ~IpcConnention() {
        stop = true;
        cv.notify_all();
        t.join();
        if (cc) delete cc;
    };
    virtual void registerClient(Client* client) override {
        std::lock_guard<std::mutex> lk(mtx);
        tasks.push([this, client]() {
            if (!cc->wait_for_recv(2, timeout_ms)) {
                std::cout << "client recv timeout" << std::endl;
                return;
            }
            auto buff = Buffer(client->ClientName);
            if (!cc->send(buff.data(), buff.size(), timeout_ms)) {
                std::cout << "client send timeout" << std::endl;
                return;
            }

            delete cc;
            cc =
                new ipc::channel((ServerName + "-" + client->ClientName).data(),
                                 ipc::receiver | ipc::sender);
        });
        cv.notify_one();
    }

    bool send(Message msg, ClientRole* role,
              const std::vector<std::string>& labels = {}) override {
        std::lock_guard<std::mutex> lk(mtx);
        tasks.push([this, role, msg, labels]() mutable {
            msg.labelIds = getMsgLabelIds(labels);
            msg.role_id = role->id();
            auto buf = Buffer(msg);
            std::cout << "send msg with roleid " << msg.role_id << std::endl;
            if (!cc->wait_for_recv(2, timeout_ms)) {
                std::cout << "client recv timeout" << std::endl;
                return;
            }
            if (!cc->send(buf.data(), buf.size())) {
                std::cout << "client send timeout" << std::endl;
                return;
            }
        });
        cv.notify_one();
        return true;
    };

    void registerRole(Role* role,
                      const std::vector<std::string>& labels = {}) override {
        std::lock_guard<std::mutex> lk(mtx);
        tasks.push([this, role, labels]() mutable {
            roleLabelIds(role) = getRoleLabelIds(labels);
            auto buf = Buffer(*role);
            if (!cc->wait_for_recv(2, timeout_ms)) {
                std::cout << "client recv timeout" << std::endl;
                return;
            }
            if (!cc->send(buf.data(), buf.size(), timeout_ms)) {
                std::cout << "client send timeout" << std::endl;
                return;
            }
            if (!cc->wait_for_recv(2, timeout_ms)) {
                std::cout << "client wait for recv RoleId timeout" << std::endl;
                return;
            }
            auto rbuf = cc->recv(timeout_ms);
            if (rbuf.size() == 0) {
                std::cout << "client recv timeout" << std::endl;
                return;
            }
            auto buf2 = Buffer(rbuf.data(), rbuf.size());
            auto idx = buf2.roleId();
            if (std::get<1>(idx)) setRoleId(role, std::get<0>(idx));

            std::cout << "id: " << role->id() << std::endl;
            std::cout << "label id :" << std::endl;
            for (auto id : roleLabelIds(role)) {
                std::cout << id << std::endl;
            }
        });

        cv.notify_one();
    };
    std::vector<size_t> getRoleLabelIds(
        std::vector<std::string> labels) override {
        std::vector<size_t> res;
        std::vector<std::string> send_tmp;
        for (auto label : labels)
            if (role_label_map.find(label) == role_label_map.end())
                send_tmp.push_back(label);
            else
                res.push_back(role_label_map[label]);
        if (send_tmp.size() != 0) {
            if (!cc->wait_for_recv(2, timeout_ms)) {
                std::cout << "client recv timeout" << std::endl;
                return res;
            }
            auto buf1 = Buffer(send_tmp, true);
            if (!cc->send(buf1.data(), buf1.size())) {
                std::cout << "client send timeout" << std::endl;
                return res;
            }
            if (!cc->wait_for_recv(2)) {
                std::cout << "client recv timeout" << std::endl;
                return res;
            }
            auto rbuf = cc->recv();
            if (rbuf.data() == nullptr) {
                std::cout << "client recv timeout" << std::endl;
                return res;
            }

            Buffer buf2(rbuf.data(), rbuf.size());
            if (buf2.type() == BufferType::ROLE_LABEL_IDS) {
                auto recv_tmp = buf2.labelIds();
                auto c = recv_tmp.size() < send_tmp.size() ? recv_tmp.size()
                                                           : send_tmp.size();
                for (size_t i = 0; i < c; i++) {
                    role_label_map[send_tmp[i]] = recv_tmp[i];
                    res.push_back(recv_tmp[i]);
                    std::cout << "register label " << send_tmp[i] << "\t"
                              << recv_tmp[i] << std::endl;
                }
            }
        }
        return res;
    };
    virtual std::vector<size_t> getMsgLabelIds(
        std::vector<std::string> labels) override {
        std::vector<size_t> res;
        std::vector<std::string> tmp;
        for (auto label : labels)
            if (msg_label_map.find(label) == role_label_map.end())
                tmp.push_back(label);
            else
                res.push_back(msg_label_map[label]);
        if (tmp.size() != 0) {
            auto buf1 = Buffer(tmp, false);
            if (!cc->wait_for_recv(2, timeout_ms)) {
                std::cout << "client recv timeout" << std::endl;
                return res;
            }
            if (!cc->send(buf1.data(), buf1.size(), timeout_ms)) {
                std::cout << "client send timeout" << std::endl;
                return res;
            }
            if (!cc->wait_for_recv(2, timeout_ms)) {
                std::cout << "client recv timeout" << std::endl;
                return res;
            }
            auto rbuf = cc->recv(timeout_ms);
            if (rbuf.data() == nullptr) {
                std::cout << "client recv timeout" << std::endl;
                return res;
            }
            auto buf2 = Buffer(rbuf.data(), rbuf.size());
            if (buf2.type() == BufferType::MSG_LABEL_IDS) {
                auto res_tmp = buf2.labelIds();
                auto c =
                    res_tmp.size() < tmp.size() ? res_tmp.size() : tmp.size();
                for (size_t i = 0; i < c; i++) {
                    msg_label_map[tmp[i]] = res_tmp[i];
                    res.push_back(res_tmp[i]);
                }
            }
        }
        return res;
    };
};

inline std::unordered_map<std::string, size_t> IpcConnention::role_label_map{};
inline std::unordered_map<std::string, size_t> IpcConnention::msg_label_map{};
}  // namespace logger

#endif  // IPC_H
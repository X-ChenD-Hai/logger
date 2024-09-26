#include <Python.h>

#include <iostream>

#include "../../../public/buffer.hpp"
#include "../../connection.h"
#include "libipc/ipc.h"

logger::ServerRole *getRole(size_t id) { return nullptr; }

void test_ipc_() {
    ipc::channel cc("test", ipc::receiver | ipc::sender);
    size_t roleid = 1;
    size_t msglabelid = 1;
    size_t rolelabelid = 1;
    size_t clientid = 1;
    while (1) {
        std::cout << "---------------------------------------------"
                  << std::endl;
        cc.wait_for_recv(2);
        std::cout << "wait for recv" << std::endl;
        auto rbuf = cc.recv();
        auto buf = logger::Buffer(rbuf.data(), rbuf.size());
        std::cout << "recv " << buf.size()
                  << " bytes type: " << (size_t)buf.type() << std::endl;
        switch (buf.type()) {
            case logger::BufferType::ROLE: {
                std::cout << "recv role" << std::endl;
                auto r = buf.role<logger::ServerRole,
                                  [](size_t) -> logger::ServerRole * {
                                      return nullptr;
                                  }>(roleid++);
                std::cout << r->name() << std::endl;
                cc.wait_for_recv(2);
                auto buf = logger::Buffer(r->id());
                cc.send(buf.data(), buf.size());
                break;
            }
            case logger::BufferType::MSG: {
                std::cout << "recv msg" << std::endl;
                auto m = buf.msg();
                std::cout << std::get<0>(m).msg << std::endl;
                break;
            }
            case logger::BufferType::ROLE_LABEL:
            case logger::BufferType::MSG_LABEL: {
                std::cout << "recv label" << std::endl;
                auto l = buf.labels();
                std::vector<size_t> ids;
                for (auto s : l) {
                    if (buf.type() == logger::BufferType::ROLE_LABEL)
                        ids.push_back(rolelabelid++);
                    else
                        ids.push_back(msglabelid++);
                    std::cout << s << std::endl;
                }
                auto buf2 = logger::Buffer(
                    ids, buf.type() == logger::BufferType::ROLE_LABEL);
                cc.send(buf2.data(), buf2.size());
                break;
            }
            case logger::BufferType::CLIENT_NAME: {
                std::cout << "recv client name" << std::endl;
                auto c = buf.clientName();
                std::cout << std::get<0>(c) << std::endl;
                auto buf2 = logger::Buffer(clientid++);
                cc.send(buf2.data(), buf2.size());
                break;
            }
            default: {
                std::cout << "Unknown buffer type received." << std::endl;
                break;
            }
        }
    }
}

int main(int argc, char const *argv[]) {
    test_ipc_();
    return 0;
}

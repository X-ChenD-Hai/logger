// server.h
#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H
#include <vector>

#include "./server.h"

namespace logger {

class ServerManager {
   protected:
    bool __stop = false;
    std::vector<Server *> __server_list;

   public:
    ServerManager(const std::vector<Server *> &server)
        : __server_list(server){};
    ~ServerManager(){};
};

};  // namespace logger

#endif  // SERVER_MANAGER_H
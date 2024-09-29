#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <pybind11/stl.h>

#include <string>
#include <unordered_map>

#include "../../ipc.hpp"

class PyServer : public logger::Server {
   public:
    using logger::Server::Server;

   public:
    virtual bool send(const logger::Buffer& buf1,
                      const logger::Connection* conn,
                      size_t resend_times) override {
        PYBIND11_OVERRIDE_PURE(bool, logger::Server, send, buf1, conn,
                               resend_times);
    }

   public:
    void listening() override {
        PYBIND11_OVERRIDE_PURE(void, logger::Server, listening);
    }
};

namespace py = pybind11;

PYBIND11_MODULE(interface, m) {
    py::class_<logger::Message>(m, "Message")
        .def(py::init<>())
        .def_readonly("msg", &logger::Message::msg);

    py::class_<logger::ServerRole>(m, "ServerRole")
        .def(py::init<>())
        .def_property_readonly("messages", &logger::ServerRole::getMsgs)
        .def_property_readonly("name", &logger::ServerRole::name)
        .def_property_readonly("id", &logger::ServerRole::id)
        .def_property_readonly(
            "parent",
            [](py::object self) {
                return static_cast<const logger::ServerRole*>(
                    self.cast<logger::ServerRole*>()->parent());
            })
        .def_property_readonly(
            "children",
            [](py::object self) {
                return reinterpret_cast<const std::unordered_map<
                    std::string, const logger::ServerRole*>&>(
                    self.cast<logger::ServerRole*>()->children());
            })
        .def_property_readonly("labels", &logger::ServerRole::labelIds)
        .def_property_readonly("timestamp", &logger::ServerRole::timestamp);

    py::class_<logger::Connection>(m, "Connection")
        .def(py::init<const std::string&>())
        .def("roles", &logger::Connection::roles)
        .def("root", &logger::Connection::root);

    py::class_<logger::Server, PyServer>(m, "Server")
        .def(py::init<const std::string&>())
        .def("send", &logger::Server::send)
        .def("listening", &logger::Server::listening)
        .def("connections", &logger::Server::connections)
        .def("connection", &logger::Server::connection);

    py::class_<logger::IpcServer, logger::Server>(m, "IpcServer")
        .def(py::init<const std::string&>());
}
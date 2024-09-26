// buffer.hpp
#ifndef BUFFER_H
#define BUFFER_H
#include "./message.hpp"
#include "role.hpp"
#if defined(__WIN32__)
#include <corecrt.h>
#elif defined(__linux__)
#include <cstdint>
#include <cstring>
#endif

namespace logger {
typedef size_t RoleId;
class Role;

constexpr size_t BufferVersion = 0x000001ull;

enum class BufferType : uint8_t {
    NULL_ = 0,
    CLIENT_NAME,
    CLIENT_ID,
    ROLE,
    ROLE_ID,
    MSG,
    ROLE_LABEL,
    MSG_LABEL,
    ROLE_LABEL_IDS,
    MSG_LABEL_IDS,
};

struct BuffHeader {
    const size_t version = BufferVersion;
    BufferType type;
};
struct ClientIdBufferHeader : public BuffHeader {
    size_t client_id;
};
struct ClientNameBufferHeader : public BuffHeader {
    size_t client_name_size;
};
struct RoleBufferHeader : public BuffHeader {
    const size_t timestamp, parent_id, name_size, labels_num;
};
struct RoleIdBufferHeader : public BuffHeader {
    const size_t role_id;
};
struct LabelsBufferHeader : public BuffHeader {
    const size_t labels_num;
};
struct LabelIdsBufferHeader : public BuffHeader {
    const size_t labels_num;
};
struct MsgBufferHeader : public BuffHeader {
    const size_t timestamp, level, role_id, msg_size, labels_num;
};

class Buffer {
   private:
    char* __data = nullptr;
    RoleId __size = 0;

   public:
    BufferType type() { return ((BuffHeader*)__data)->type; };
    template <class T, T* (*getParentById)(size_t)>
        requires std::is_base_of<Role, T>::value
    T* role(RoleId id) {
        if (type() != BufferType::ROLE) return nullptr;
        RoleBufferHeader* header = (RoleBufferHeader*)__data;
        Role* r = new T();
        r->__name =
            std::string((char*)header + sizeof(logger::RoleBufferHeader),
                        header->name_size);
        r->__labelIds = std::vector<size_t>(
            (size_t*)((char*)header + sizeof(logger::RoleBufferHeader) +
                      header->name_size),
            (size_t*)((char*)header + sizeof(logger::RoleBufferHeader) +
                      header->name_size + header->labels_num));
        r->__parent = getParentById(header->parent_id);
        r->__id = id;
        if (r->__parent) r->__parent->__children[r->name()] = r;
        r->__timestamp = header->timestamp;
        return (T*)r;
    };
    std::tuple<Message, bool> msg() {
        if (type() != BufferType::MSG) return {Message{}, false};
        Message msg;
        MsgBufferHeader* header = (logger::MsgBufferHeader*)__data;
        msg.role = header->role_id;
        msg.msg = std::string((char*)header + sizeof(logger::MsgBufferHeader),
                              header->msg_size);
        msg.level = header->level;
        ;
        msg.labelIds = std::vector<size_t>(
            (size_t*)((char*)header + sizeof(logger::MsgBufferHeader) +
                      header->msg_size),
            (size_t*)((char*)header + sizeof(logger::MsgBufferHeader) +
                      header->msg_size + header->labels_num));
        return {std::move(msg), true};
    };
    std::vector<std::string> labels() {
        if (type() != BufferType::MSG_LABEL && type() != BufferType::ROLE_LABEL)
            return {};
        logger::LabelsBufferHeader* header =
            (logger::LabelsBufferHeader*)__data;
        std::vector<std::string> labels;
        char* offset = (char*)(header + 1);
        std::string tmp;
        for (size_t i = 0; i < header->labels_num; i++) {
            tmp = std::string(offset + sizeof(size_t), *(size_t*)(offset));
            offset += tmp.size() + sizeof(size_t);
            labels.push_back(tmp);
        }
        return labels;
    };
    std::vector<size_t> labelIds() {
        if (type() != BufferType::MSG_LABEL_IDS &&
            type() != BufferType::ROLE_LABEL_IDS)
            return {};
        logger::LabelIdsBufferHeader* header =
            (logger::LabelIdsBufferHeader*)__data;
        return std::vector<size_t>((size_t*)(header + 1),
                                   (size_t*)(header + 1) + header->labels_num);
    }
    std::tuple<RoleId, bool> roleId() {
        if (type() != BufferType::ROLE_ID) return {0, false};
        RoleIdBufferHeader* header = (RoleIdBufferHeader*)__data;
        return {header->role_id, true};
    }
    std::tuple<size_t, bool> clientId() {
        if (type() != BufferType::CLIENT_ID) return {0, false};
        ClientIdBufferHeader* header = (ClientIdBufferHeader*)__data;
        return {header->client_id, true};
    }

    std::tuple<std::string, bool> clientName() {
        if (type() != BufferType::CLIENT_NAME) return {"", false};
        ClientNameBufferHeader* header = (ClientNameBufferHeader*)__data;
        return {std::string((char*)header + sizeof(ClientNameBufferHeader),
                            header->client_name_size),
                true};
    }

   public:
    size_t size() const { return __size; };
    char* data() const { return __data; };

    Buffer(const std::string& client_name) {
        ClientNameBufferHeader header = {
            {BufferVersion, BufferType::CLIENT_NAME},

            client_name.size(),
        };
        __size = sizeof(ClientNameBufferHeader) + header.client_name_size;
        __data = new char[__size];
        memcpy(__data, &header, sizeof(ClientNameBufferHeader));
        memcpy(__data + sizeof(ClientNameBufferHeader), client_name.data(),
               header.client_name_size);
    }
    Buffer(const Role& role) {
        RoleBufferHeader header = {
            {BufferVersion, BufferType::ROLE},       role.timestamp(),
            role.parent() ? role.parent()->id() : 0, role.name().size(),
            role.labelIds().size() * sizeof(size_t),
        };
        __size =
            sizeof(RoleBufferHeader) + header.name_size + header.labels_num;

        __data = new char[__size];
        memcpy(__data, &header, sizeof(RoleBufferHeader));
        memcpy(__data + sizeof(RoleBufferHeader), role.name().data(),
               header.name_size);
        memcpy(__data + sizeof(RoleBufferHeader) + header.name_size,
               role.labelIds().data(), header.labels_num);
    };
    Buffer(const Message& msg) {
        MsgBufferHeader header = {
            {BufferVersion, BufferType::MSG},
            msg.timestamp,
            msg.level,
            msg.role,
            msg.msg.size(),
            msg.labelIds.size() * sizeof(size_t),
        };
        __size = sizeof(MsgBufferHeader) + header.msg_size + header.labels_num;

        __data = new char[__size];
        memcpy(__data, &header, sizeof(MsgBufferHeader));
        memcpy(__data + sizeof(MsgBufferHeader), msg.msg.data(),
               header.msg_size);
        memcpy(__data + sizeof(MsgBufferHeader) + header.msg_size,
               msg.labelIds.data(), header.labels_num);
    }
    Buffer(const std::vector<std::string>& labels, bool isRoleLabel = false) {
        LabelsBufferHeader header = {
            {BufferVersion,
             isRoleLabel ? BufferType::ROLE_LABEL : BufferType::MSG_LABEL},
            labels.size()};
        __size = sizeof(header);
        for (size_t i = 0; i < labels.size(); i++)
            __size += labels[i].size() + sizeof(size_t);
        __data = new char[__size];
        memcpy(__data, &header, sizeof(header));
        char* offset = __data + sizeof(header);
        size_t length = 0;
        for (auto s : labels) {
            length = s.size();
            memcpy(offset, &length, sizeof(size_t));
            offset += sizeof(size_t);
            memcpy(offset, s.data(), s.size());
            offset += length;
        }
    }
    Buffer(const std::vector<size_t>& labelIds, bool isRoleLabel = false) {
        LabelIdsBufferHeader header = {
            {BufferVersion, isRoleLabel ? BufferType::ROLE_LABEL_IDS
                                        : BufferType::MSG_LABEL_IDS},
            labelIds.size()};
        __size = sizeof(header);
        __size += labelIds.size() * sizeof(size_t);
        __data = new char[__size];
        memcpy(__data, &header, sizeof(header));
        memcpy(__data + sizeof(header), labelIds.data(),
               labelIds.size() * sizeof(size_t));
    }
    Buffer(const RoleId& id) {
        RoleIdBufferHeader header = {{BufferVersion, BufferType::ROLE_ID}, id};
        __size = sizeof(header);
        __data = new char[__size];
        memcpy(__data, &header, sizeof(header));
    }

    Buffer(void* data, size_t size) : __data(new char[size]), __size(size) {
        memcpy(__data, data, size);
    };
    Buffer(Buffer& buf) = delete;
    Buffer(Buffer&& buf) : __data((char*)buf.__data), __size(buf.__size){};
    ~Buffer() {
        if (__data) {
            delete[] __data;
            __data = nullptr;
        }
    }
};

}  // namespace logger

#endif  // BUFFER_H
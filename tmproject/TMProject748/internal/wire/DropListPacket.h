#pragma once
#include "MessageHeader.h"
#include <cstdint>
#include <cstring>

namespace drop_list {
constexpr unsigned short QueryOpcode=0x7F41, ResultOpcode=0x7F42;
constexpr unsigned int ChanceScale=1000000000u, PageSize=10;
struct Query {
    MSG_STANDARD Header;
    std::uint16_t Version; std::uint8_t Mode, Reserved;
    std::uint32_t ID, Offset;
    char Text[64]; unsigned char Items[816];
};
struct Row {
    std::uint16_t Item, Flags;
    std::uint32_t Reserved, Base, Current, Reserved2;
    char Monster[48];
};
struct Result {
    MSG_STANDARD Header;
    std::uint16_t Version; std::uint8_t Mode, Status;
    std::uint32_t ID, Offset, Total;
    std::int32_t Bonus;
    std::uint16_t Count, Reserved;
    Row Rows[10];
};
static_assert(sizeof(Query)==904 && offsetof(Query,Items)==88,"drop query v1");
static_assert(sizeof(Row)==68 && offsetof(Row,Monster)==20,"drop row v1");
static_assert(sizeof(Result)==716 && offsetof(Result,Rows)==36,"drop result v1");
inline bool Parse(const void* bytes,std::size_t size,Result& out) {
    if(!bytes || size!=sizeof(Result)) return false;
    Result p{}; std::memcpy(&p,bytes,sizeof(p));
    if(p.Header.Size!=sizeof(p) || p.Header.Type!=ResultOpcode || p.Header.ID!=0x7530 ||
        p.Version!=1 || p.Mode>1 || p.Status>1 || !p.ID || p.Reserved || p.Count>PageSize ||
        p.Offset%PageSize || p.Offset>p.Total || p.Count>p.Total-p.Offset || (p.Status && p.Count)) return false;
    for(unsigned int i=0;i<PageSize;++i) {
        const Row& r=p.Rows[i];
        if(i>=p.Count) { Row empty{}; if(std::memcmp(&r,&empty,sizeof(r))) return false; continue; }
        if(!r.Item || r.Item>=6500 || !r.Flags || (r.Flags&~15u) || r.Reserved || r.Reserved2 ||
            r.Base>ChanceScale || r.Current>ChanceScale || !std::memchr(r.Monster,0,sizeof(r.Monster))) return false;
    }
    out=p; return true;
}

class PendingQuery {
public:
    unsigned int Start(unsigned char mode) {
        if (++sequence_ == 0) ++sequence_;
        Begin(sequence_, mode); return sequence_;
    }
    void Begin(unsigned int id, unsigned char mode) { id_=id;mode_=mode; }
    void Reset() { id_=0; }
    bool IsPending() const { return id_!=0; }
    bool Consume(const Result& result) {
        if(!id_ || result.ID!=id_ || result.Mode!=mode_) return false;
        Reset();return true;
    }
private:
    unsigned int id_{};
    unsigned int sequence_{};
    unsigned char mode_{};
};
}

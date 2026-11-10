#pragma once
#include <string>

namespace drop_search {
// Item captions use Windows-1252. Fold the Latin letters used by EN/PT
// without changing catalog bytes or the displayed caption.
inline std::string Key(const char* text) {
    std::string result;
    if (!text) return result;
    for (const unsigned char* p=reinterpret_cast<const unsigned char*>(text); *p; ++p) {
        unsigned char c=*p;
        if (c>='A' && c<='Z') c+='a'-'A';
        else if (c>=0xC0 && c<=0xDE && c!=0xD7) c+=0x20;
        if (c=='_') c=' ';
        if (c>=0xE0 && c<=0xE5) c='a';
        else if (c==0xE7) c='c';
        else if (c>=0xE8 && c<=0xEB) c='e';
        else if (c>=0xEC && c<=0xEF) c='i';
        else if (c==0xF1) c='n';
        else if ((c>=0xF2 && c<=0xF6) || c==0xF8) c='o';
        else if (c>=0xF9 && c<=0xFC) c='u';
        else if (c==0xFD || c==0xFF || c==0x9F) c='y';
        if (c==' ' || c=='\t' || c=='\r' || c=='\n') {
            if (!result.empty() && result.back()!=' ') result.push_back(' ');
        } else result.push_back(static_cast<char>(c));
    }
    if (!result.empty() && result.back()==' ') result.pop_back();
    return result;
}
}

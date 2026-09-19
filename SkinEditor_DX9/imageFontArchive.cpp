#include "imageFontArchive.h"
#include "../lib/DxLib/DxLib.h"
#include <Windows.h>
#include <algorithm>
#include <functional>
#include <filesystem>
#include <set>

namespace {
constexpr size_t MaxArchive = 128 * 1024 * 1024;
constexpr size_t MaxIndex = 4 * 1024 * 1024;
// DXArchive::KeyCreate(NULL): the documented default key, not password recovery.
constexpr unsigned char DefaultKey[] = {0x55,0xaa,0x20,0x55,0x55,0x06,0x55,0xaa,0x55,0xd5,0x7c,0x66};
uint32_t U32(const std::string& s, size_t p) {
    return (unsigned char)s[p] | ((uint32_t)(unsigned char)s[p+1] << 8) |
        ((uint32_t)(unsigned char)s[p+2] << 16) | ((uint32_t)(unsigned char)s[p+3] << 24);
}
void Put32(std::string& s, size_t p, uint32_t n) {
    for (size_t i = 0; i < 4; ++i) s[p+i] = (char)(n >> (i*8));
}
void Crypt(std::string& s, uint64_t offset, bool keyed) {
    if (keyed) for (size_t i = 0; i < s.size(); ++i) s[i] ^= DefaultKey[(offset+i)%12];
}
bool Range(uint64_t p, uint64_t n, uint64_t end) { return p <= end && n <= end-p; }
bool Fail(std::string& error, const char* message) { error = message; return false; }
struct File {
    HANDLE h = INVALID_HANDLE_VALUE;
    uint64_t size = 0, time = 0;
    explicit File(const std::string& path) {
        h = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
        BY_HANDLE_FILE_INFORMATION info{};
        if (h != INVALID_HANDLE_VALUE && GetFileInformationByHandle(h, &info)) {
            size = ((uint64_t)info.nFileSizeHigh << 32) | info.nFileSizeLow;
            time = ((uint64_t)info.ftLastWriteTime.dwHighDateTime << 32) | info.ftLastWriteTime.dwLowDateTime;
        }
    }
    ~File() { if (h != INVALID_HANDLE_VALUE) CloseHandle(h); }
    bool Read(uint64_t p, size_t n, std::string& bytes) {
        if (h == INVALID_HANDLE_VALUE || !Range(p,n,size) || n > MaxArchive) return false;
        LARGE_INTEGER offset{}; offset.QuadPart = p; DWORD count = 0;
        if (!SetFilePointerEx(h,offset,nullptr,FILE_BEGIN)) return false;
        std::string result(n, '\0');
        if (!ReadFile(h,result.data(),(DWORD)n,&count,nullptr) || count != n) return false;
        bytes = std::move(result); return true;
    }
};
bool IsFile(const std::string& path) {
    DWORD attr = GetFileAttributesA(path.c_str());
    return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
}
// Bound every token and back-reference before writing; the upstream decoder is
// intentionally not called on untrusted data because it has no buffer lengths.
bool Expand(const std::string& packed, uint32_t size, std::string& bytes, std::string& error) {
    if (packed.size() < 9 || U32(packed,0) != size || U32(packed,4) != packed.size())
        return Fail(error,"Invalid DXA compression header.");
    std::string result; result.reserve(size);
    const unsigned char escape = (unsigned char)packed[8];
    size_t p = 9;
    while (p < packed.size()) {
        unsigned char c = (unsigned char)packed[p++];
        if (c != escape) {
            if (result.size() >= size) return Fail(error,"DXA output exceeds declared size.");
            result += (char)c; continue;
        }
        if (p == packed.size()) return Fail(error,"Truncated DXA escape.");
        c = (unsigned char)packed[p++];
        if (c == escape) {
            if (result.size() >= size) return Fail(error,"DXA output exceeds declared size.");
            result += (char)c; continue;
        }
        if (c > escape) --c;
        size_t count = c >> 3;
        if (c & 4) {
            if (p == packed.size()) return Fail(error,"Truncated DXA length.");
            count |= (size_t)(unsigned char)packed[p++] << 5;
        }
        count += 4;
        size_t n = (c & 3) + 1, distance = 0;
        if (n == 4 || !Range(p,n,packed.size())) return Fail(error,"Invalid DXA back-reference.");
        for (size_t i=0; i<n; ++i) distance |= (size_t)(unsigned char)packed[p++] << (8*i);
        ++distance;
        if (distance > result.size() || count > size-result.size()) return Fail(error,"DXA back-reference is out of bounds.");
        for (size_t i=0; i<count; ++i) result += result[result.size()-distance];
    }
    if (result.size() != size) return Fail(error,"DXA output size mismatch.");
    bytes = std::move(result); return true;
}
}

std::string SEFontPathKey(const std::string& path) {
    std::string key = path;
    // Do not interpret 0x5c inside a CP932 multibyte character as a separator.
    for (size_t i=0; i<key.size(); ++i) {
        if (IsDBCSLeadByteEx(932,(BYTE)key[i])) { ++i; continue; }
        if (key[i] == '/') key[i] = '\\';
        else if (key[i] >= 'A' && key[i] <= 'Z') key[i] += 'a'-'A';
    }
    return key;
}
bool SEImageFontArchive::ParseHeader(std::string& error) {
    if (header.size() < 24) return Fail(error,"Truncated DXA header.");
    keyed = true;
    Crypt(header,0,keyed);
    version = (unsigned char)header[2] | ((unsigned char)header[3]<<8);
    if (header[0] != 'D' || header[1] != 'X' || version < 1 || version > 4)
        return Fail(error,"Unsupported DXA: font editing supports legacy versions 1-4 with the default key. Custom passwords and versions 5+ are not modified.");
    const size_t headSize = version >= 4 ? 28 : 24;
    if (header.size() < headSize) return Fail(error,"Truncated DXA header.");
    header.resize(headSize);
    dataStart=U32(header,8); indexStart=U32(header,12); fileTable=U32(header,16); dirTable=U32(header,20);
    const size_t indexSize=U32(header,4);
    if (fileSize > MaxArchive || dataStart < headSize || indexStart < dataStart ||
        indexSize > MaxIndex || !Range(indexStart,indexSize,fileSize) || indexStart+indexSize != fileSize ||
        fileTable >= dirTable || !Range(dirTable,16,indexSize))
        return Fail(error,"Invalid or oversized DXA tables (128 MiB archive / 4 MiB index limit).");
    return true;
}
bool SEImageFontArchive::ParseIndex(std::string& error) {
    Crypt(index,indexStart,keyed); entries.clear();
    const uint32_t stride = version >= 2 ? 44 : 40;
    std::set<uint32_t> visitedDirs, visitedRecords;
    auto nameAt = [&](uint32_t p, std::string& name) {
        if (!Range(p,4,fileTable)) return false;
        const size_t padded = ((unsigned char)index[p] | ((unsigned char)index[p+1]<<8))*4;
        const size_t start = p+4+padded;
        if (!padded || !Range(start,padded,fileTable)) return false;
        const size_t end = index.find('\0',start);
        if (end == std::string::npos || end >= start+padded) return false;
        name=index.substr(start,end-start);
        const std::string key=SEFontPathKey(name);
        if (key.empty() || key == "." || key == ".." || key.size() > 255) return false;
        for (size_t i=0; i<key.size(); ++i) {
            if (IsDBCSLeadByteEx(932,(BYTE)key[i])) { if (++i == key.size()) return false; continue; }
            if ((unsigned char)key[i] < 32 || key[i] == '\\' || key[i] == ':') return false;
        }
        return true;
    };
    std::function<bool(uint32_t,const std::string&,int)> walk;
    walk = [&](uint32_t offset, const std::string& prefix, int depth) {
        if (depth > 32 || !Range(dirTable,(uint64_t)offset+16,index.size()) || !visitedDirs.insert(offset).second) return false;
        const size_t d=dirTable+offset;
        const uint32_t count=U32(index,d+8), first=U32(index,d+12);
        if (count > 65536 || !Range(fileTable,(uint64_t)first+(uint64_t)count*stride,dirTable)) return false;
        for (uint32_t i=0; i<count; ++i) {
            const uint32_t row=fileTable+first+i*stride;
            if (visitedRecords.size() >= 65536 || !visitedRecords.insert(row).second) return false;
            std::string name;
            if (!nameAt(U32(index,row),name)) return false;
            name=prefix+name;
            if (name.size() > 2048) return false;
            const uint32_t offsetData=U32(index,row+32);
            if (U32(index,row+4)&FILE_ATTRIBUTE_DIRECTORY) {
                if (!walk(offsetData,name+"\\",depth+1)) return false;
            } else {
                Entry entry{name,row,offsetData,U32(index,row+36),version >= 2 ? U32(index,row+40) : 0xffffffff};
                const uint32_t stored=entry.packed == 0xffffffff ? entry.size : entry.packed;
                if (!Range(dataStart,(uint64_t)entry.offset+stored,indexStart) ||
                    !entries.emplace(SEFontPathKey(name),entry).second) return false;
            }
        }
        return true;
    };
    if (!walk(0,"",0)) return Fail(error,"Invalid DXA directory, filename or file extent.");
    return true;
}
bool SEImageFontArchive::Open(const std::string& file, bool keepSnapshot, std::string& error) {
    File input(file); SEImageFontArchive next;
    // Workspaces may change the process working directory after opening a font.
    // Preserve the exact save target independently of that later directory.
    next.path=std::filesystem::absolute(file).lexically_normal().string();
    next.fileSize=input.size; next.fileTime=input.time;
    if (!input.Read(0,(size_t)(std::min)(input.size,uint64_t(28)),next.header) || !next.ParseHeader(error)) {
        if (error.empty()) error="Cannot read DXA archive."; return false;
    }
    if (keepSnapshot) {
        auto bytes=std::make_shared<std::string>();
        if (!input.Read(0,(size_t)input.size,*bytes)) return Fail(error,"Cannot snapshot DXA archive.");
        next.snapshot=bytes;
        next.index=bytes->substr(next.indexStart,U32(next.header,4));
    } else if (!input.Read(next.indexStart,U32(next.header,4),next.index)) return Fail(error,"Cannot read DXA index.");
    if (!next.ParseIndex(error)) return false;
    *this=std::move(next); error.clear(); return true;
}
bool SEImageFontArchive::Parse(std::shared_ptr<const std::string> bytes, std::string& error) {
    if (!bytes) return Fail(error,"Missing DXA bytes.");
    SEImageFontArchive next; next.snapshot=bytes; next.fileSize=bytes->size(); next.header=bytes->substr(0,28);
    if (!next.ParseHeader(error)) return false;
    next.index=bytes->substr(next.indexStart,U32(next.header,4));
    if (!next.ParseIndex(error)) return false;
    *this=std::move(next); error.clear(); return true;
}
bool SEImageFontArchive::Read(const std::string& member, size_t limit, std::string& bytes, std::string& error) const {
    auto found=entries.find(SEFontPathKey(member));
    if (found == entries.end()) return Fail(error,"File not found inside DXA archive.");
    const Entry& e=found->second;
    const size_t stored=e.packed == 0xffffffff ? e.size : e.packed;
    if (e.size > limit || stored > MaxArchive) return Fail(error,"DXA member exceeds the font/image size limit.");
    std::string content;
    const uint64_t offset=(uint64_t)dataStart+e.offset;
    if (snapshot) content=snapshot->substr((size_t)offset,stored);
    else {
        File input(path);
        if (input.size != fileSize || input.time != fileTime || !input.Read(offset,stored,content))
            return Fail(error,"DXA changed or could not be read; reload it.");
    }
    Crypt(content,offset,keyed);
    if (e.packed != 0xffffffff) return Expand(content,e.size,bytes,error);
    bytes=std::move(content); return true;
}
bool SEImageFontArchive::Replace(const std::string& member, const std::string& bytes, std::string& result, std::string& error) const {
    auto found=entries.find(SEFontPathKey(member));
    if (!snapshot || found == entries.end() || bytes.size() > 8*1024*1024) return Fail(error,"DXA font replacement requires an unchanged archive snapshot.");
    const Entry& e=found->second;
    std::string newHeader=header, newIndex=index;
    // Keep every unrelated payload at exactly its original offset. Reuse the
    // selected slot when possible; otherwise append before the index. No re-pack
    // or version conversion, and no extracted files left beside the skin.
    size_t target=(size_t)dataStart+e.offset;
    const size_t stored=e.packed == 0xffffffff ? e.size : e.packed;
    result=snapshot->substr(0,indexStart);
    bool overlaps=false;
    for (const auto& pair : entries) if (pair.first != found->first) {
        const auto& other=pair.second;
        const uint64_t start=(uint64_t)dataStart+other.offset;
        const uint64_t count=other.packed == 0xffffffff ? other.size : other.packed;
        if (target < start+count && start < target+stored) overlaps=true;
    }
    if (bytes.size() > stored || overlaps) {
        target=(result.size()+3)&~size_t(3); result.resize(target+bytes.size(),'\0');
        result.resize((result.size()+3)&~size_t(3),'\0');
    }
    if (result.size()+newIndex.size() > MaxArchive) return Fail(error,"Edited DXA would exceed 128 MiB.");
    std::string payload=bytes; Crypt(payload,target,keyed); result.replace(target,payload.size(),payload);
    Put32(newIndex,e.record+32,(uint32_t)(target-dataStart)); Put32(newIndex,e.record+36,(uint32_t)bytes.size());
    if (version >= 2) Put32(newIndex,e.record+40,0xffffffff);
    const uint32_t newIndexStart=(uint32_t)result.size(); Put32(newHeader,12,newIndexStart);
    Crypt(newIndex,newIndexStart,keyed); result += newIndex;
    Crypt(newHeader,0,keyed); result.replace(0,newHeader.size(),newHeader);
    SEImageFontArchive verify; std::string actual;
    if (!verify.Parse(std::make_shared<const std::string>(result),error) ||
        !verify.Read(member,8*1024*1024,actual,error) || actual != bytes)
        return Fail(error,"DXA replacement verification failed; nothing was saved.");
    return true;
}
std::vector<std::string> SEImageFontArchive::Fonts() const {
    std::vector<std::string> result;
    for (const auto& pair : entries) if (pair.first.size() >= 8 && pair.first.compare(pair.first.size()-8,8,".lr2font") == 0) result.push_back(pair.second.name);
    return result;
}
bool SEFindImageFontArchive(const std::string& virtualPath, std::string& archive, std::string& member) {
    // Walk separators (CP932-aware) from the leaf toward the root.
    const std::string key=SEFontPathKey(virtualPath);
    std::vector<size_t> separators;
    for (size_t i=0; i<key.size(); ++i) {
        if (IsDBCSLeadByteEx(932,(BYTE)key[i])) { ++i; continue; }
        if (key[i] == '\\') separators.push_back(i);
    }
    for (auto i=separators.rbegin(); i!=separators.rend(); ++i) {
        std::string candidate=virtualPath.substr(0,*i);
        const std::string prefix=key.substr(0,*i);
        if (prefix.size() < 4 || prefix.compare(prefix.size()-4,4,".dxa") != 0) candidate += ".dxa";
        if (IsFile(candidate)) { archive=candidate; member=virtualPath.substr(*i+1); return true; }
    }
    archive.clear(); member.clear(); return false;
}
std::string SEImageFontArchiveVirtualPath(const std::string& archive, const std::string& member) {
    const std::string key=SEFontPathKey(archive);
    return (key.size() >= 4 && key.compare(key.size()-4,4,".dxa") == 0 ? archive.substr(0,archive.size()-4) : archive)+"\\"+member;
}
bool SEReadFontResource(const std::string& path, size_t limit, std::string& bytes, std::string& error) {
    std::string archive, member;
    if (SEFindImageFontArchive(path,archive,member)) {
        // FileRead_open may decompress immediately. Validate extents, sizes and
        // compressed tokens before passing editor-selected resources to DxLib.
        SEImageFontArchive checkedArchive;
        std::string checked;
        if (!checkedArchive.Open(archive,false,error) ||
            !checkedArchive.Read(member,limit,checked,error)) return false;
        const std::string virtualPath=SEImageFontArchiveVirtualPath(archive,member);
        const int handle=DxLib::FileRead_open(virtualPath.c_str());
        if (handle <= 0) return Fail(error,"DxLib could not open the DXA member.");
        const auto size=DxLib::FileRead_size_handle(handle);
        bool ok=size >= 0 && (uint64_t)size <= limit && size <= INT_MAX;
        std::string result;
        if (ok) {
            result.resize((size_t)size);
            ok=DxLib::FileRead_read(result.data(),(int)size,handle) == size;
        }
        DxLib::FileRead_close(handle);
        if (!ok) return Fail(error,"Cannot read DXA member or font/image size limit exceeded.");
        if (result != checked) return Fail(error,"DXA changed or library cache is stale; reload the font/images.");
        bytes=std::move(result); return true;
    }
    File input(path);
    if (input.size > limit || !input.Read(0,(size_t)input.size,bytes)) return Fail(error,"Cannot read font/image file or DXA member, or size limit exceeded.");
    return true;
}

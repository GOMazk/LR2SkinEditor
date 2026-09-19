#include "imageFontDocument.h"
#include "winWorkspace.h"
#include "../LR2/En_fileutil.h"
#include "../LR2/LR2_skinmanage.h"
#include "../lib/DxLib/DxLib.h"
#include <Windows.h>
#include <shellapi.h>
#include <filesystem>
#include <fstream>
#include <cstdio>

namespace {
void Put(std::string& s, size_t p, uint32_t value) {
    for (size_t i=0; i<4; ++i) s[p+i]=(char)(value>>(i*8));
}
void Crypt(std::string& s) {
    const unsigned char key[]={0x55,0xaa,0x20,0x55,0x55,6,0x55,0xaa,0x55,0xd5,0x7c,0x66};
    for (size_t i=0; i<s.size(); ++i) s[i]^=key[i%12];
}
size_t Name(std::string& s, const std::string& name) {
    const size_t start=s.size(), pad=(name.size()+4)&~size_t(3);
    s.resize(start+4+pad*2,'\0'); s[start]=(char)(pad/4);
    unsigned sum=0;
    for (size_t i=0; i<name.size(); ++i) {
        const char c=name[i]>='a' && name[i]<='z' ? name[i]-32 : name[i];
        s[start+4+i]=c; sum+=(unsigned char)c; s[start+4+pad+i]=name[i];
    }
    s[start+2]=(char)sum; s[start+3]=(char)(sum>>8); return start;
}
const std::string Font="#S,18\r\n#M,1\r\n#T,0,page.tga\r\n#R,65,0,0,0,2,1\r\n// keep\r\n";
std::string Image() {
    std::string tga(18,'\0'); tga[2]=2; tga[12]=2; tga[14]=1; tga[16]=32; tga[17]=0x28;
    tga.append("\0\0\xff\xff\0\xff\0\xff",8); return tga;
}
// Independent small fixture: nested folder, two font definitions, raw TGA,
// compressed literal font stream, and compressed overlapping LZ back-reference.
std::string Fixture(int version=3) {
    std::string compressed(9,'\0'); Put(compressed,0,(uint32_t)Font.size());
    Put(compressed,4,(uint32_t)(Font.size()+9)); compressed += Font;
    std::string repeated(9,'\0'); Put(repeated,0,10); Put(repeated,4,13);
    repeated += 'A'; repeated.append("\0\x29\0",3); // escaped opcode 0x28: distance 1/count 9
    std::vector<std::string> bodies={version==1 ? Font : compressed, Image(), Font, version==1 ? std::string(10,'A') : repeated};
    const size_t head=version>=4 ? 28 : 24, stride=version>=2 ? 44 : 40;
    std::string names; Name(names,"");
    size_t nFolder=Name(names,"nested"), nFont=Name(names,"font.lr2font"), nPage=Name(names,"page.tga"), nOther=Name(names,"other.lr2font"), nRepeat=Name(names,"repeat.bin");
    std::string data, files(stride*6,'\0'), dirs(32,'\0');
    auto record=[&](size_t r,size_t name,uint32_t attr,uint32_t offset,uint32_t size,uint32_t packed) {
        size_t p=r*stride; Put(files,p,(uint32_t)name); Put(files,p+4,attr); Put(files,p+32,offset); Put(files,p+36,size);
        if(version>=2) Put(files,p+40,packed);
    };
    record(0,0,16,0,0,0xffffffff); record(1,nFolder,16,16,0,0xffffffff);
    const size_t ids[]={nFont,nPage,nOther,nRepeat};
    for(size_t i=0;i<4;++i) {
        record(i+2,ids[i],32,(uint32_t)data.size(),(uint32_t)(i==0 ? Font.size() : i==3 ? 10 : bodies[i].size()),
            version>=2 && (i==0 || i==3) ? (uint32_t)bodies[i].size() : 0xffffffff);
        data+=bodies[i]; data.resize((data.size()+3)&~size_t(3),'\0');
    }
    Put(dirs,4,0xffffffff); Put(dirs,8,1); Put(dirs,12,(uint32_t)stride);
    Put(dirs,16,(uint32_t)stride); Put(dirs,24,4); Put(dirs,28,(uint32_t)(stride*2));
    std::string result(head,'\0'); result[0]='D';result[1]='X';result[2]=(char)version;
    Put(result,4,(uint32_t)(names.size()+files.size()+dirs.size())); Put(result,8,(uint32_t)head);
    Put(result,12,(uint32_t)(head+data.size())); Put(result,16,(uint32_t)names.size()); Put(result,20,(uint32_t)(names.size()+files.size()));
    if(version>=4) Put(result,24,932);
    result+=data+names+files+dirs; Crypt(result); return result;
}
bool Write(const std::string& file, const std::string& bytes) { std::ofstream out(file,std::ios::binary); out.write(bytes.data(),bytes.size()); out.close(); return !!out; }
std::string Read(const std::string& file) { std::ifstream in(file,std::ios::binary); return std::string(std::istreambuf_iterator<char>(in),{}); }
}

int RunImageFontArchiveSelfTest() {
    std::string error, bytes;
    for (int version=1; version<=4; ++version) {
        SEImageFontArchive archive;
        if (!archive.Parse(std::make_shared<const std::string>(Fixture(version)),error) || archive.Fonts().size()!=2 || archive.version!=version) return 1;
        if (!archive.Read("NESTED/FONT.LR2FONT",8192,bytes,error) || bytes!=Font ||
            !archive.Read("nested/repeat.bin",100,bytes,error) || bytes!=std::string(10,'A')) return 2;
        if (archive.Read("nested/font.lr2font",1,bytes,error)) return 3;
        std::string edited=Font+std::string(900,' '), output;
        if (!archive.Replace("nested/font.lr2font",edited,output,error)) return 4;
        SEImageFontArchive verify;
        if (!verify.Parse(std::make_shared<const std::string>(output),error) || verify.version!=version ||
            !verify.Read("nested/font.lr2font",8192,bytes,error) || bytes!=edited ||
            !verify.Read("nested/page.tga",8192,bytes,error) || bytes!=Image() ||
            !verify.Read("nested/other.lr2font",8192,bytes,error) || bytes!=Font) return 5;
        if (!verify.Replace("nested/font.lr2font",Font,bytes,error) || bytes.size()!=output.size()) return 6;
    }
    const std::string fixture=Fixture();
    SEImageFontArchive corrupt;
    for(size_t size : {size_t(0),size_t(23),fixture.size()-1})
        if(corrupt.Parse(std::make_shared<const std::string>(fixture.substr(0,size)),error)) return 7;
    std::string bad=fixture; Crypt(bad); Put(bad,12,0xfffffff0); Crypt(bad);
    if(corrupt.Parse(std::make_shared<const std::string>(bad),error)) return 8;
    bad=fixture; Crypt(bad); bad[2]=8; Crypt(bad);
    if(corrupt.Parse(std::make_shared<const std::string>(bad),error)) return 9;
    char temp[MAX_PATH]={}, root[MAX_PATH]={};
    if(!GetTempPathA(MAX_PATH,temp) || !GetTempFileNameA(temp,"dxf",0,root) || !DeleteFileA(root) || !CreateDirectoryA(root,nullptr)) return 10;
    struct Cleanup { std::string root,path; ~Cleanup(){DxLib::DXArchiveRelease(path.c_str()); std::error_code ec; std::filesystem::remove_all(root,ec);} } cleanup{root,std::string(root)+"\\archive.dxa"};
    const std::string path=SEImageFontArchiveVirtualPath(cleanup.path,"nested\\font.lr2font");
    for(int version=1;version<=4;++version) {
        DxLib::DXArchiveRelease(cleanup.path.c_str());
        if(!Write(cleanup.path,Fixture(version))) return 11;
        if(!SEReadFontResource(path,8192,bytes,error) || bytes!=Font) return 12;
    }
    DxLib::DXArchiveRelease(cleanup.path.c_str());
    if(!Write(cleanup.path,fixture)) return 11;
    // The old Win32 search turns valid archive members into the literal ERROR.
    if(!GetRandomFileNoError(CSTR(path.c_str()),CSTR("")).isSame("ERROR")) return 26;
    const std::string resolved=SEFindPreviewImageFontPath(path.c_str(),"","");
    if(SEFontPathKey(resolved)!=SEFontPathKey(path)) return 27;
    const auto wildcard=SEFindPreviewImageFontPath(
        SEImageFontArchiveVirtualPath(cleanup.path,"nested\\*.lr2font").c_str(),"","");
    if(wildcard.empty() || !SEReadFontResource(wildcard,8192,bytes,error) || bytes!=Font) return 28;
    const std::string owner=cleanup.root+"\\part.csv";
    const auto relative=SEFindPreviewImageFontPath("archive\\nested\\font.lr2font",owner.c_str(),owner.c_str());
    if(SEFontPathKey(relative)!=SEFontPathKey(path) ||
        !SEFindPreviewImageFontPath("archive\\nested\\missing.lr2font",owner.c_str(),owner.c_str()).empty()) return 29;
    if(!SEReadFontResource(path,8192,bytes,error) || bytes!=Font) return 12;
    if(!SEReadFontResource(SEImageFontArchiveVirtualPath(cleanup.path,"nested\\page.tga"),8192,bytes,error) || bytes!=Image()) return 13;
    D3DXIMAGE_INFO info{};
    if(FAILED(D3DXGetImageInfoFromFileInMemory(bytes.data(),(UINT)bytes.size(),&info)) || info.Width!=2 || info.Height!=1) return 14;
    SEImageFontDocument doc;
    {
        struct RestoreDirectory { std::filesystem::path old=std::filesystem::current_path();
            ~RestoreDirectory(){std::error_code ec;std::filesystem::current_path(old,ec);} } restore;
        std::filesystem::current_path(cleanup.root);
        if(!doc.Open("archive\\nested\\font.lr2font",error) || !doc.archive) return 15;
    }
    if(!std::filesystem::path(doc.archive->path).is_absolute() || !doc.SetMetrics(24,2,error) || !doc.Save(error)) return 15;
    if(!SEReadFontResource(path,8192,bytes,error) || bytes!=doc.bytes || doc.Dirty()) return 16;
    SEImageFontArchive after;
    if(!after.Open(cleanup.path,true,error) || !after.Read("nested/page.tga",8192,bytes,error) || bytes!=Image()) return 17;
    const std::string saved=Read(cleanup.path);
    if(!doc.SetMetrics(30,3,error) || !Write(cleanup.path+".skineditor.bak","retain") || doc.Save(error) ||
        Read(cleanup.path)!=saved || Read(cleanup.path+".skineditor.bak")!="retain") return 18;
    DeleteFileA((cleanup.path+".skineditor.bak").c_str());
    if(!Write(cleanup.path,fixture) || doc.Save(error) || Read(cleanup.path)!=fixture) return 19;
    if(!doc.Open(path,error) || !doc.SetMetrics(31,1,error)) return 20;
    DxLib::DXArchiveRelease(cleanup.path.c_str());
    HANDLE locked=CreateFileA(cleanup.path.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr);
    if(locked==INVALID_HANDLE_VALUE) return 21;
    const bool savedLocked=doc.Save(error); CloseHandle(locked);
    if(savedLocked || !doc.Dirty() || Read(cleanup.path)!=fixture) return 22;
    if(!doc.Save(error)) return 23;
    // Native LR2 reader + editor save must use the newly built library, not the
    // editor's index reader. This fails when accidentally linking the old DxLib.
    auto workspace=std::make_unique<WORKSPACE>();
    std::vector<FontChar> chars(0x3bce); std::vector<FontImage> pages(1000);
    for(auto& c:chars)c.grHandle=-1; for(auto& p:pages)p.grHandle=-1;
    auto& font=workspace->g.skstruct.ImageFonts[0]; font.chars=chars.data();font.images=pages.data();
    workspace->imageFontRuntimePaths[0]=path;
    if(!workspace->OpenImageFont(path) || font.size!=31 || chars[65].width!=2 || std::string(pages[0].filename)!="page.tga") return 24;
    auto& e=workspace->imageFontEditor; e.height=42;e.fieldsDirty=e.metricsDirty=true;
    if(!workspace->SaveImageFont() || font.size!=42 || workspace->documentRevision!=0 || workspace->arr_history.count!=0) return 25;
    font.chars=nullptr; font.images=nullptr;
    return 0;
}

// Optional read-only check for a real local archive. No skin/archive is written.
int CheckImageFontArchiveCommand() {
    int count=0; auto args=CommandLineToArgvW(GetCommandLineW(),&count);
    std::string path;
    for(int i=1;args && i+1<count;++i) if(wcscmp(args[i],L"--check-image-font-dxa")==0) {
        char utf8[32768]={}; WideCharToMultiByte(CP_UTF8,0,args[i+1],-1,utf8,sizeof(utf8),nullptr,nullptr); SEImageFontEncode(utf8,path);
    }
    if(args)LocalFree(args);
    SEImageFontArchive archive;std::string error,bytes;size_t glyphs=0,pages=0;
    if(path.empty() || !archive.Open(path,true,error)) { printf("DXA: %s\n",error.c_str()); return 1; }
    for(const auto& member:archive.Fonts()) {
        SEImageFontDocument doc;
        if(!doc.Open(SEImageFontArchiveVirtualPath(path,member),error)) { printf("Font: %s\n",error.c_str());return 2; }
        glyphs+=doc.glyphs.size();
        for(const auto& page:doc.pages) {
            const auto imagePath=SEImageFontPagePath(doc,page.first);
            if(!SEReadFontResource(imagePath,64*1024*1024,bytes,error)) {printf("Page %s: %s\n",imagePath.c_str(),error.c_str());return 3;}
            D3DXIMAGE_INFO info{};
            if(FAILED(D3DXGetImageInfoFromFileInMemory(bytes.data(),(UINT)bytes.size(),&info))) return 4;
            ++pages;
        }
    }
    DxLib::DXArchiveRelease(path.c_str());
    printf("DXA v%d: %zu fonts, %zu glyphs, %zu image pages read successfully (read-only).\n",archive.version,archive.Fonts().size(),glyphs,pages);
    return archive.Fonts().empty() ? 5 : 0;
}

// Read-only integration probe: follow the same LoadSkin/ReadSkinSE provenance
// and OpenImageFont action used by the "Fonts in current Preview" combo.
int CheckPreviewImageFontsCommand() {
    int count=0; auto args=CommandLineToArgvW(GetCommandLineW(),&count);
    std::string path;
    for(int i=1;args && i+1<count;++i) if(wcscmp(args[i],L"--check-preview-image-fonts")==0) {
        char utf8[32768]={}; WideCharToMultiByte(CP_UTF8,0,args[i+1],-1,utf8,sizeof(utf8),nullptr,nullptr);
        SEImageFontEncode(utf8,path);
    }
    if(args)LocalFree(args);
    if(path.empty() || path.size()>=MAX_PATH || !DxLib::DxLib_IsInit()) return 1;
    auto workspace=std::make_unique<WORKSPACE>();
    if(!InitSkinData(&workspace->g.skinData)) return 2;
    workspace->skinBrowserDataInitialized=true;
    ParseLR2SkinCustom(&workspace->g.skinData,CSTR(path.c_str()));
    if(workspace->g.skinData.Count<=0) return 3;
    workspace->meta=workspace->g.skinData.Data[0];
    strcpy_s(workspace->mainpath,path.c_str());
    if(workspace->LoadSkin(workspace->mainpath)!=0) return 4;
    int opened=0;
    for(size_t slot=0;slot<workspace->imageFontRuntimePaths.size();++slot) {
        const std::string font=workspace->imageFontRuntimePaths[slot];
        if(font.empty()) continue;
        if(font=="ERROR" || !workspace->OpenImageFont(font)) {
            printf("Slot %zu: %s (%s)\n",slot,font.c_str(),workspace->imageFontEditor.status.c_str()); return 5;
        }
        if(workspace->g.skstruct.ImageFonts[slot].size<=0) return 6;
        printf("Slot %zu: %s (%zu glyphs)\n",slot,font.c_str(),workspace->imageFontEditor.document.glyphs.size());
        ++opened;
    }
    printf("%d Preview image fonts opened successfully (read-only).\n",opened);
    return opened ? 0 : 7;
}

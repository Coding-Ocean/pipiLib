#pragma once
#include"pch.h"
#include"WARNING.h"
class SHADER_RESOURCE
{
public:
    SHADER_RESOURCE(LPCWSTR resName)
        :Succeeded(false)
    {
        HINSTANCE hInst = GetModuleHandle(0);
        HRSRC hRes = FindResource(hInst, resName, L"SHADER");
        WARNING(hRes == 0, L"リソースがみつからない", resName);
        //リソースのサイズを取得する 
        DWORD sizeOfRes = SizeofResource(hInst, hRes);
        WARNING(sizeOfRes == 0, L"リソースのサイズがゼロ", L"");
        //取得したリソースをメモリにロードする
        HGLOBAL hMem = LoadResource(hInst, hRes);
        WARNING(hMem == 0, L"リソースがロードできない", L"");
        //ロードしたリソースデータのアドレスを取得する
        const char* mem = (const char*)LockResource(hMem);
        if (mem == 0) {
            FreeResource(hMem);
            WARNING(true, L"リソースのアドレスが取得できない", L"");
            return;
        }
        //Bufferにコピー
        Succeeded = true;
        Buffer.assign(mem, sizeOfRes);
        FreeResource(hMem);
    }
    bool succeeded() const
    {
        return Succeeded;
    }
    unsigned char* code() const
    {
        char* p = const_cast<char*>(Buffer.data());
        return reinterpret_cast<unsigned char*>(p);
    }
    size_t size() const
    {
        return Buffer.size();
    }
private:
    std::string Buffer;
    bool Succeeded;
};


#pragma once
#include"pch.h"
class SHADER_RESOURCE
{
public:
    SHADER_RESOURCE(LPCWSTR resName) :Succeeded(false)
    {
        HINSTANCE hInst = GetModuleHandle(0);
        HRSRC hRes = FindResource(hInst, resName, L"SHADER");
        //WARNING(hRes == 0, "リソースがみつからない", "");
        // リソースのサイズを取得する 
        DWORD sizeOfRes = SizeofResource(hInst, hRes);
        //WARNING(sizeOfRes == 0, "リソースのサイズがゼロ", "");
        // 取得したリソースをメモリにロードする
        HGLOBAL hMem = LoadResource(hInst, hRes);
        //WARNING(HMem == 0, "リソースがロードできない", "");
        // ロードしたリソースデータのアドレスを取得する
        const char* mem = (const char*)LockResource(hMem);
        if (mem == 0) {
            FreeResource(hMem);
            //WARNING(true, "リソースのアドレスが取得できない", "");
            return;
        }
        //Bufferにコピー
        Succeeded = true;
        Buffer.assign(mem,sizeOfRes);
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


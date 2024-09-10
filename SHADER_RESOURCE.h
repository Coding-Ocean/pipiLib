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
        WARNING(hRes == 0, L"���\�[�X���݂���Ȃ�", resName);
        //���\�[�X�̃T�C�Y���擾���� 
        DWORD sizeOfRes = SizeofResource(hInst, hRes);
        WARNING(sizeOfRes == 0, L"���\�[�X�̃T�C�Y���[��", L"");
        //�擾�������\�[�X���������Ƀ��[�h����
        HGLOBAL hMem = LoadResource(hInst, hRes);
        WARNING(hMem == 0, L"���\�[�X�����[�h�ł��Ȃ�", L"");
        //���[�h�������\�[�X�f�[�^�̃A�h���X���擾����
        const char* mem = (const char*)LockResource(hMem);
        if (mem == 0) {
            FreeResource(hMem);
            WARNING(true, L"���\�[�X�̃A�h���X���擾�ł��Ȃ�", L"");
            return;
        }
        //Buffer�ɃR�s�[
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


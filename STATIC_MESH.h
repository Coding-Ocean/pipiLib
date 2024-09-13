#pragma once
using namespace Microsoft::WRL;

class STATIC_MESH
{
public:
    STATIC_MESH();
    ~STATIC_MESH();
    void createFromText(const wchar_t* filename);
    void update(const MATRIX& m);
    void draw();
private:
    //���_�o�b�t�@�ƃC���f�b�N�X�o�b�t�@
    ComPtr<ID3D12Resource> VertexBuf;
    D3D12_VERTEX_BUFFER_VIEW Vbv{};
    ComPtr<ID3D12Resource> IndexBuf;
    D3D12_INDEX_BUFFER_VIEW Ibv{};
    //�R���X�^���g�o�b�t�@�A�e�N�X�`���o�b�t�@�A�f�B�X�N���v�^�q�[�v
    ComPtr<ID3D12Resource> ConstBuf1;
    struct GRAPHIC::CONST_BUF1* ConstBuf1Map{};
    ComPtr<ID3D12Resource> ConstBuf2;
    struct GRAPHIC::CONST_BUF2* ConstBuf2Map{};
    ComPtr<ID3D12Resource> TextureBuf;
    ComPtr<ID3D12DescriptorHeap> CbvTbvHeap;
};


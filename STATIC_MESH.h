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
    //頂点バッファとインデックスバッファ
    ComPtr<ID3D12Resource> VertexBuf;
    D3D12_VERTEX_BUFFER_VIEW Vbv{};
    ComPtr<ID3D12Resource> IndexBuf;
    D3D12_INDEX_BUFFER_VIEW Ibv{};
    //コンスタントバッファ、テクスチャバッファ、ディスクリプタヒープ
    ComPtr<ID3D12Resource> ConstBuf1;
    struct GRAPHIC::CONST_BUF1* ConstBuf1Map{};
    ComPtr<ID3D12Resource> ConstBuf2;
    struct GRAPHIC::CONST_BUF2* ConstBuf2Map{};
    ComPtr<ID3D12Resource> TextureBuf;
    ComPtr<ID3D12DescriptorHeap> CbvTbvHeap;
};


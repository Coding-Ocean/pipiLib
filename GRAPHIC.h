#pragma once

using namespace Microsoft::WRL;

class GRAPHIC
{
public:
    static GRAPHIC& instance();
    void create();
    void destroy();
    void clear(float r=0.0f, float g=0.0f, float b=0.0f);
    void present();

    //void setPipeline();

    //void createVtxPosition(
    //    std::vector<float>positions,
    //    ID3D12Resource** positionBuf,
    //    D3D12_VERTEX_BUFFER_VIEW* positionBufView
    //);
    //void createVtxPosition(
    //    float* positions, size_t size,
    //    ID3D12Resource** positionBuf,
    //    D3D12_VERTEX_BUFFER_VIEW* positionBufView
    //);
    //void createVtxTexcoord(
    //    float* texcoords, size_t size,
    //    ID3D12Resource** texcoordBuf,
    //    D3D12_VERTEX_BUFFER_VIEW* texcoordBufView
    //);
    //void createIndex(
    //    unsigned short* indices, size_t size, 
    //    ID3D12Resource** indexBuf,
    //    D3D12_INDEX_BUFFER_VIEW* indexBufView
    //);
    //struct CONST_BUF1 {
    //    XMMATRIX world;
    //    XMFLOAT4 diffuse;
    //};
    //void createConstBuf1(
    //    size_t size,
    //    ID3D12Resource** constBuf1,
    //    CONST_BUF1** mapConstBuf1
    //);
    //void createTextureBuf(
    //    const char* filename,
    //    ID3D12Resource** textureBuf
    //);
    //void createCbvTbvHeap(
    //    ID3D12Resource* constBuf1,
    //    ID3D12Resource* textureBuf,
    //    ID3D12DescriptorHeap** CbvTbvHeap
    //);
    //void draw(
    //    D3D12_VERTEX_BUFFER_VIEW& positionBufView,
    //    D3D12_VERTEX_BUFFER_VIEW& texcoordBufView,
    //    D3D12_INDEX_BUFFER_VIEW& indexBufView,
    //    UINT numIndices,
    //    ID3D12DescriptorHeap* CbvTbvHeap
    //);
private:
    void WaitDrawDone();
    //デバッグ
    HRESULT Hr;
    //デバイス
    ComPtr<ID3D12Device> Device;
    //コマンド
    ComPtr<ID3D12CommandAllocator> CommandAllocator;
    ComPtr<ID3D12GraphicsCommandList> CommandList;
    ComPtr<ID3D12CommandQueue> CommandQueue;
    //フェンス
    ComPtr<ID3D12Fence> Fence;
    HANDLE FenceEvent;
    UINT64 FenceValue;
    //スワップチェイン、バックバッファ
    ComPtr<IDXGISwapChain4> SwapChain;
    ComPtr<ID3D12Resource> BackBufs[2];//Bb
    ComPtr<ID3D12DescriptorHeap> BackBufViewHeap;//BbvHeap
    UINT BackBufIdx;
    UINT BackBufViewHeapSize;
    //デプスバッファ
    ComPtr<ID3D12Resource> DepthStencilBuf;
    ComPtr<ID3D12DescriptorHeap> DepthStencilBufViewHeap;

    //パイプライン
    ComPtr<ID3D12RootSignature> RootSignature;
    ComPtr<ID3D12PipelineState> PipelineState;
    D3D12_VIEWPORT Viewport{};
    D3D12_RECT ScissorRect{};
    //コンスタントバッファ０構造体
    struct CONST_BUF0 {
        XMMATRIX proj;
        XMMATRIX view;
    };
    //コンスタントバッファ０
    ComPtr<ID3D12Resource> ConstBuf0;
    CONST_BUF0* MapConstBuf0{};
    ComPtr<ID3D12DescriptorHeap> CbvHeap;
    //シングルトン
    GRAPHIC() {};
    GRAPHIC(const GRAPHIC&) = delete;
    void operator=(const GRAPHIC&) = delete;
};


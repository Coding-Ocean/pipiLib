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

    void setPipeline();

    void createVertexBuf(
        float* vertices, size_t size, size_t stride,
        ID3D12Resource** vertexBuf,
        D3D12_VERTEX_BUFFER_VIEW* vertexBufView
    );
    void createIndexBuf(
        unsigned short* indices, size_t size, 
        ID3D12Resource** indexBuf,
        D3D12_INDEX_BUFFER_VIEW* indexBufView
    );

    //�R���X�^���g�o�b�t�@�\����
    struct CONST_BUF0 {
        class MATRIX proj;
        class MATRIX view;
        class FLOAT3 lightPos;
    };
    struct CONST_BUF1 {
        class MATRIX world;
    };
    struct CONST_BUF2 {
        struct FLOAT4 diffuse;
        struct FLOAT4 ambient;
        struct FLOAT4 specular;
    };
    void createConstBuf(
        size_t size,
        ID3D12Resource** constBuf,
        void** constBufMap
    );
    
    void createTextureBuf(
        LPCWSTR filename,
        ID3D12Resource** textureBuf
    );

    void createTextureBuf(
        LPCSTR filename,
        ID3D12Resource** textureBuf
    );

    //���[�g�e�[�u���ɑΉ�����f�B�X�N���v�^�q�[�v������
    void createCbvTbvHeap(
        ID3D12Resource* constBuf1,
        ID3D12Resource* constBuf2,
        ID3D12Resource* textureBuf,
        ID3D12DescriptorHeap** CbvTbvHeap
    );

    void draw(
        D3D12_VERTEX_BUFFER_VIEW& vertexBufView,
        D3D12_INDEX_BUFFER_VIEW& indexBufView,
        ID3D12DescriptorHeap* CbvTbvHeap
    );

    void setView(FLOAT3& eye, FLOAT3& focus, FLOAT3& up);

private:
    void WaitDrawDone();

    //�f�o�b�O
    HRESULT Hr;

    //�f�o�C�X
    ComPtr<ID3D12Device> Device;
    //�R�}���h
    ComPtr<ID3D12CommandAllocator> CommandAllocator;
    ComPtr<ID3D12GraphicsCommandList> CommandList;
    ComPtr<ID3D12CommandQueue> CommandQueue;
    //�t�F���X
    ComPtr<ID3D12Fence> Fence;
    HANDLE FenceEvent;
    UINT64 FenceValue;

    //�X���b�v�`�F�C���A�o�b�N�o�b�t�@
    ComPtr<IDXGISwapChain4> SwapChain;
    ComPtr<ID3D12Resource> BackBufs[2];
    UINT BackBufIdx;
    ComPtr<ID3D12DescriptorHeap> BbvHeap;//BackBufViewHeap
    UINT BbvHeapSize;
    //�f�v�X�o�b�t�@
    ComPtr<ID3D12Resource> DepthStencilBuf;
    ComPtr<ID3D12DescriptorHeap> DsvHeap;//DepthStencilBufViewHeap

    //�R���X�^���g�o�b�t�@�O
    ComPtr<ID3D12Resource> ConstBuf0;
    CONST_BUF0* ConstBuf0Map{};
    ComPtr<ID3D12DescriptorHeap> Cb0vHeap;

    //�p�C�v���C��
    ComPtr<ID3D12RootSignature> RootSignature;
    ComPtr<ID3D12PipelineState> PipelineState;
    D3D12_VIEWPORT Viewport{};
    D3D12_RECT ScissorRect{};

    //�V���O���g��
    GRAPHIC() {};
    GRAPHIC(const GRAPHIC&) = delete;
    void operator=(const GRAPHIC&) = delete;
};


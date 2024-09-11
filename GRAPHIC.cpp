#include"pch.h"
#include"WARNING.h"
#include"SHADER_RESOURCE.h"
#include"WINDOW.h"
#include"GRAPHIC.h"
#include<memory>

#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3d12.lib")

GRAPHIC& GRAPHIC::instance()
{
    static std::unique_ptr<GRAPHIC> instance(new GRAPHIC);
    return *instance;
}

void GRAPHIC::create()
{
	auto& w = WINDOW::instance();

	//�f�o�C�X�A�R�}���h�A�t�F���X
	{
#ifdef _DEBUG
		//�f�o�b�O�o�[�W�����ł̓f�o�b�O���C���[��L��������
		{
			ComPtr<ID3D12Debug> debug;
			Hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
			assert(SUCCEEDED(Hr));
			debug->EnableDebugLayer();
		}
#endif
		//�f�o�C�X������(�ȈՃo�[�W����)
		{
			Hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0,
				IID_PPV_ARGS(&Device));
			assert(SUCCEEDED(Hr));
		}
		//�R�}���h
		{
			//�R�}���h�A���P�[�^������
			Hr = Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(&CommandAllocator));
			assert(SUCCEEDED(Hr));

			//�R�}���h���X�g������
			Hr = Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
				CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&CommandList));
			assert(SUCCEEDED(Hr));

			//�R�}���h�L���[������
			D3D12_COMMAND_QUEUE_DESC desc = {};
			desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;		//GPU�^�C���A�E�g���L��
			desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;		//���ڃR�}���h�L���[
			Hr = Device->CreateCommandQueue(&desc, IID_PPV_ARGS(&CommandQueue));
			assert(SUCCEEDED(Hr));
		}
		//�t�F���X
		{
			//GPU�̏����������`�F�b�N����t�F���X������
			Hr = Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence));
			assert(SUCCEEDED(Hr));
			FenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
			assert(FenceEvent != nullptr);
			FenceValue = 1;
		}
	}
	//�����_�[�^�[�Q�b�g
	{
		//�X���b�v�`�F�C��������(�����Ƀo�b�N�o�b�t�@���܂܂�Ă���)
		{
			//DXGI�t�@�N�g��������
			ComPtr<IDXGIFactory6> dxgiFactory;
			Hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory));
			assert(SUCCEEDED(Hr));

			//�X���b�v�`�F�C��������
			DXGI_SWAP_CHAIN_DESC1 desc = {};
			desc.BufferCount = 2; //�o�b�N�o�b�t�@2��
			desc.Width = w.clientWidth();
			desc.Height = w.clientHeight();
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			desc.SampleDesc.Count = 1;
			ComPtr<IDXGISwapChain1> swapChain;
			Hr = dxgiFactory->CreateSwapChainForHwnd(
				CommandQueue.Get(), w.hWnd(), &desc, nullptr, nullptr, &swapChain);
			assert(SUCCEEDED(Hr));

			//IDXGISwapChain4�C���^�[�t�F�C�X���T�|�[�g���Ă��邩�q�˂�
			Hr = swapChain.As(&SwapChain);
			assert(SUCCEEDED(Hr));
		}
		//�o�b�N�o�b�t�@�u�r���[�v�̓��ꕨ�ł���u�f�B�X�N���v�^�q�[�v�v������
		{
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			desc.NumDescriptors = 2;//�o�b�N�o�b�t�@�r���[�Q��
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//RenderTargetView
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;//�V�F�[�_����A�N�Z�X���Ȃ��̂�NONE��OK
			Hr = Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&BbvHeap));
			assert(SUCCEEDED(Hr));
		}
		//�o�b�N�o�b�t�@�u�r���[�v���u�f�B�X�N���v�^�q�[�v�v�ɂ���
		{
			D3D12_CPU_DESCRIPTOR_HANDLE hBbvHeap
				= BbvHeap->GetCPUDescriptorHandleForHeapStart();

			BbvHeapSize
				= Device->GetDescriptorHandleIncrementSize(
					D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

			for (UINT idx = 0; idx < 2; idx++) {
				//�o�b�N�o�b�t�@�����o��
				Hr = SwapChain->GetBuffer(idx, IID_PPV_ARGS(&BackBufs[idx]));
				assert(SUCCEEDED(Hr));
				//�o�b�N�o�b�t�@�̃r���[���q�[�v�ɂ���
				hBbvHeap.ptr += idx * BbvHeapSize;
				Device->CreateRenderTargetView(BackBufs[idx].Get(), nullptr, hBbvHeap);
			}
		}

		//�f�v�X�X�e���V���o�b�t�@������
		{
			D3D12_HEAP_PROPERTIES prop = {};
			prop.Type = D3D12_HEAP_TYPE_DEFAULT;//DEFAULT��������UNKNOWN�ł悵
			prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			D3D12_RESOURCE_DESC desc = {};
			desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//2�����̃e�N�X�`���f�[�^�Ƃ���
			desc.Width = w.clientWidth();//���̓o�b�N�o�b�t�@�Ɠ���
			desc.Height = w.clientHeight();//�����̓o�b�N�o�b�t�@�Ɠ���
			desc.DepthOrArraySize = 1;//�e�N�X�`���z��ł��Ȃ���3D�e�N�X�`���ł��Ȃ�
			desc.Format = DXGI_FORMAT_D32_FLOAT;//�[�x�l�������ݗp�t�H�[�}�b�g
			desc.SampleDesc.Count = 1;//�T���v����1�s�N�Z��������1��
			desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;//���̃o�b�t�@�͐[�x�X�e���V���Ƃ��Ďg�p���܂�
			desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			desc.MipLevels = 1;
			//�f�v�X�X�e���V���o�b�t�@���N���A����l
			D3D12_CLEAR_VALUE depthClearValue = {};
			depthClearValue.DepthStencil.Depth = 1.0f;//�[���P(�ő�l)�ŃN���A
			depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;//32bit�[�x�l�Ƃ��ăN���A
			//�f�v�X�X�e���V���o�b�t�@�����
			Hr = Device->CreateCommittedResource(
				&prop,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_DEPTH_WRITE, //�f�v�X�������݂Ɏg�p
				&depthClearValue,
				IID_PPV_ARGS(&DepthStencilBuf));
			assert(SUCCEEDED(Hr));
		}
		//�f�v�X�X�e���V���o�b�t�@�u�r���[�v�̓��ꕨ�ł���u�f�X�N���v�^�q�[�v�v������
		{
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};//�[�x�Ɏg����Ƃ��������킩��΂���
			desc.NumDescriptors = 1;//�[�x�r���[1�̂�
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;//�f�v�X�X�e���V���r���[�Ƃ��Ďg��
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			Hr = Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&DsvHeap));
			assert(SUCCEEDED(Hr));
		}
		//�f�v�X�X�e���V���o�b�t�@�u�r���[�v���u�f�B�X�N���v�^�q�[�v�v�ɂ���
		{
			D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
			desc.Format = DXGI_FORMAT_D32_FLOAT;//�f�v�X�l��32bit�g�p
			desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2D�e�N�X�`��
			desc.Flags = D3D12_DSV_FLAG_NONE;//�t���O�͓��ɂȂ�
			D3D12_CPU_DESCRIPTOR_HANDLE hDsvHeap
				= DsvHeap->GetCPUDescriptorHandleForHeapStart();
			Device->CreateDepthStencilView(DepthStencilBuf.Get(), &desc, hDsvHeap);
		}
	}{}
	//�R���X�^���g�o�b�t�@�O������A�f�t�H���g�l������Ă���
	{
		createConstBuf(256, &ConstBuf0, (void**)&ConstBuf0Map);
		//�R���X�^���g�o�b�t�@�O�́u�r���[�v�̓��ꕨ�ł���u�f�B�X�N���v�^�q�[�v�v������
		{
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			desc.NumDescriptors = 1;
			desc.NodeMask = 0;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;// D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			Hr = Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&Cb0vHeap));
			assert(SUCCEEDED(Hr));
		}
		//�R���X�^���g�o�b�t�@�O�́u�r���[�v���u�f�B�X�N���v�^�q�[�v�v�ɂ���
		{
			D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
			desc.BufferLocation = ConstBuf0->GetGPUVirtualAddress();
			desc.SizeInBytes = static_cast<UINT>(ConstBuf0->GetDesc().Width);
			auto hCbvHeap = Cb0vHeap->GetCPUDescriptorHandleForHeapStart();
			Device->CreateConstantBufferView(&desc, hCbvHeap);
		}
		//�R���X�^���g�o�b�t�@�O�f�t�H���g�ݒ�
		//�r���[�}�g���b�N�X
		FLOAT3 eye = { 0, 0, 2 }, focus = { 0, 0, 0 }, up = { 0, 1, 0 };
		ConstBuf0Map->view.lookat(eye, focus, up);
		//�v���W�F�N�V�����}�g���b�N�X
		float aspect = static_cast<float>(w.clientWidth()) / w.clientHeight();
		ConstBuf0Map->proj.pers(3.141592f / 4, aspect, 1.0f, 10.0f);
		//���C�g�̈ʒu
		ConstBuf0Map->lightPos = FLOAT3(0, 0, 1);
	}

	//�p�C�v���C��
	{
		//���[�g�V�O�l�`��
		{
			//�f�B�X�N���v�^�����W�B�f�B�X�N���v�^�q�[�v�ƃV�F�[�_��R�Â�����́B
			D3D12_DESCRIPTOR_RANGE range[2] = {};
			UINT b0 = 0;
			range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			range[0].BaseShaderRegister = b0;
			range[0].NumDescriptors = 3;
			range[0].RegisterSpace = 0;
			range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
			UINT t0 = 0;
			range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			range[1].BaseShaderRegister = t0;
			range[1].NumDescriptors = 1;
			range[1].RegisterSpace = 0;
			range[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			//���[�g�p�����^���f�B�X�N���v�^�e�[�u���Ƃ��Ďg�p
			D3D12_ROOT_PARAMETER rootParam[1] = {};
			rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParam[0].DescriptorTable.pDescriptorRanges = range;
			rootParam[0].DescriptorTable.NumDescriptorRanges = _countof(range);
			rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

			//�T���v���̋L�q�B���̃T���v�����V�F�[�_�[�� s0 �ɃZ�b�g�����
			D3D12_STATIC_SAMPLER_DESC samplerDesc[1] = {};
			samplerDesc[0].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;//��Ԃ��Ȃ�(�j�A���X�g�l�C�o�[)
			samplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//���J��Ԃ�
			samplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//�c�J��Ԃ�
			samplerDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//���s�J��Ԃ�
			samplerDesc[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;//�{�[�_�[�̎��͍�
			samplerDesc[0].MaxLOD = D3D12_FLOAT32_MAX;//�~�b�v�}�b�v�ő�l
			samplerDesc[0].MinLOD = 0.0f;//�~�b�v�}�b�v�ŏ��l
			samplerDesc[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//�I�[�o�[�T���v�����O�̍ۃ��T���v�����O���Ȃ��H
			samplerDesc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//�s�N�Z���V�F�[�_����̂݉�

			//���[�g�V�O�j�`���̋L�q�B���rootParam��samplerDesc���P�ɂ܂Ƃ߂�B
			D3D12_ROOT_SIGNATURE_DESC desc = {};
			desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
			desc.pParameters = rootParam;//���[�g�p�����[�^�̐擪�A�h���X
			desc.NumParameters = _countof(rootParam);//���[�g�p�����[�^��
			desc.pStaticSamplers = samplerDesc;//�T���v���[�̐擪�A�h���X
			desc.NumStaticSamplers = _countof(samplerDesc);//�T���v���[��

			//���[�g�V�O�l�`�����V���A���C�Y��blob(��)������B
			ID3DBlob* blob;
			Hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, nullptr);
			assert(SUCCEEDED(Hr));

			//���[�g�V�O�l�`��������
			Hr = Device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(),
				IID_PPV_ARGS(&RootSignature));
			assert(SUCCEEDED(Hr));
			blob->Release();
		}

		//�V�F�[�_�����\�[�X����ǂݍ���
		SHADER_RESOURCE vs(L"LAMBERT_VS");
		assert(vs.succeeded());
		SHADER_RESOURCE ps(L"LAMBERT_PS");
		assert(ps.succeeded());

		
		//�ȉ��A�e��L�q

		UINT slot0 = 0;
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, slot0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, slot0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    slot0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		D3D12_RASTERIZER_DESC rasterDesc = {};
		rasterDesc.FrontCounterClockwise = true;
		rasterDesc.CullMode = D3D12_CULL_MODE_NONE;
		rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
		rasterDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		rasterDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		rasterDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		rasterDesc.DepthClipEnable = true;
		rasterDesc.MultisampleEnable = false;
		rasterDesc.AntialiasedLineEnable = false;
		rasterDesc.ForcedSampleCount = 0;
		rasterDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		D3D12_BLEND_DESC blendDesc = {};
		blendDesc.AlphaToCoverageEnable = true;
		blendDesc.IndependentBlendEnable = false;
		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].LogicOpEnable = false;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
		depthStencilDesc.DepthEnable = true;
		depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;//�S�ď�������
		depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;//�����������̗p
		depthStencilDesc.StencilEnable = false;//�X�e���V���o�b�t�@�͎g��Ȃ�


		//�����܂ł̋L�q���܂Ƃ߂ăp�C�v���C���X�e�[�g�I�u�W�F�N�g������
		D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc = {};
		pipelineDesc.pRootSignature = RootSignature.Get();
		pipelineDesc.VS = { vs.code(), vs.size() };
		pipelineDesc.PS = { ps.code(), ps.size() };
		pipelineDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		pipelineDesc.RasterizerState = rasterDesc;
		pipelineDesc.BlendState = blendDesc;
		pipelineDesc.DepthStencilState = depthStencilDesc;
		pipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		pipelineDesc.SampleMask = UINT_MAX;
		pipelineDesc.SampleDesc.Count = 1;
		pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		pipelineDesc.NumRenderTargets = 1;
		pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		Hr = Device->CreateGraphicsPipelineState(
			&pipelineDesc,
			IID_PPV_ARGS(&PipelineState)
		);
		assert(SUCCEEDED(Hr));

		//�o�͗̈��ݒ�
		Viewport.TopLeftX = 0.0f;
		Viewport.TopLeftY = 0.0f;
		Viewport.Width = static_cast<float>(w.clientWidth());
		Viewport.Height = static_cast<float>(w.clientHeight());
		Viewport.MinDepth = 0.0f;
		Viewport.MaxDepth = 1.0f;

		//�؂����`��ݒ�
		ScissorRect.left = 0;
		ScissorRect.top = 0;
		ScissorRect.right = w.clientWidth();
		ScissorRect.bottom = w.clientHeight();
	}
}

void GRAPHIC::destroy()
{
	WaitDrawDone();
	ConstBuf0->Unmap(0, nullptr);
}

void GRAPHIC::clear(float r, float g, float b)
{
	//���݂̃o�b�N�o�b�t�@�̃C���f�b�N�X���擾�B���̃v���O�����̏ꍇ0 or 1�ɂȂ�B
	BackBufIdx = SwapChain->GetCurrentBackBufferIndex();

	//�o�b�N�o�b�t�@�̑J��
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		BackBufs[BackBufIdx].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandList->ResourceBarrier(1, &barrier);

	//�o�b�N�o�b�t�@�̃f�B�X�N���v�^�n���h����p�ӂ���
	auto hBbvHeap = BbvHeap->GetCPUDescriptorHandleForHeapStart();
	hBbvHeap.ptr += BackBufIdx * BbvHeapSize;
	//�f�v�X�X�e���V���o�b�t�@�̃f�B�X�N���v�^�n���h����p�ӂ���
	auto hDsvHeap = DsvHeap->GetCPUDescriptorHandleForHeapStart();
	//�o�b�N�o�b�t�@�ƃf�v�X�X�e���V���o�b�t�@��`��^�[�Q�b�g�Ƃ��Đݒ肷��
	CommandList->OMSetRenderTargets(1, &hBbvHeap, false, &hDsvHeap);

	//�`��^�[�Q�b�g���N���A����
	const float clearColor[] = { r, g, b, 1.0f };
	CommandList->ClearRenderTargetView(hBbvHeap, clearColor, 0, nullptr);
	//�f�v�X�X�e���V���o�b�t�@���N���A����
	CommandList->ClearDepthStencilView(hDsvHeap, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//�r���[�|�[�g�ƃV�U�[��`���Z�b�g
	CommandList->RSSetViewports(1, &Viewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);
}

void GRAPHIC::present()
{
	//�o�b�N�o�b�t�@�̑J��
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		BackBufs[BackBufIdx].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);
	CommandList->ResourceBarrier(1, &barrier);

	//�R�}���h���X�g���N���[�Y����
	CommandList->Close();
	//�R�}���h���X�g�����s����
	ID3D12CommandList* commandLists[] = { CommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
	//�`�抮����҂�
	WaitDrawDone();

	//�o�b�t�@��\��
	SwapChain->Present(1, 0);

	//�R�}���h�A���P�[�^�����Z�b�g
	Hr = CommandAllocator->Reset();
	assert(SUCCEEDED(Hr));
	//�R�}���h���X�g�����Z�b�g
	Hr = CommandList->Reset(CommandAllocator.Get(), nullptr);
	assert(SUCCEEDED(Hr));
}

void GRAPHIC::setPipeline()
{
	//�p�C�v���C���X�e�[�g���Z�b�g
	CommandList->SetPipelineState(PipelineState.Get());
	//���[�g�V�O�j�`��
	CommandList->SetGraphicsRootSignature(RootSignature.Get());
}

void GRAPHIC::createVertexBuf(float* vertices, size_t size, UINT stride, ID3D12Resource** vertexBuf, D3D12_VERTEX_BUFFER_VIEW* vertexBufView)
{
	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto desc = CD3DX12_RESOURCE_DESC::Buffer(size);
	Hr = Device->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(vertexBuf));
	assert(SUCCEEDED(Hr));

	//�ʒu�o�b�t�@�ɐ��f�[�^���R�s�[
	UINT8* mappedBuf;
	Hr = (*vertexBuf)->Map(0, nullptr, reinterpret_cast<void**>(&mappedBuf));
	assert(SUCCEEDED(Hr));
	memcpy(mappedBuf, vertices, size);
	(*vertexBuf)->Unmap(0, nullptr);

	//�ʒu�o�b�t�@�̃r���[�����������Ă����B�i�f�B�X�N���v�^�q�[�v�ɍ��Ȃ��Ă悢�j
	vertexBufView->BufferLocation = (*vertexBuf)->GetGPUVirtualAddress();
	vertexBufView->SizeInBytes = static_cast<UINT>(size);
	vertexBufView->StrideInBytes = stride;
}

void GRAPHIC::createIndexBuf(unsigned short* indices, size_t size, ID3D12Resource** indexBuf, D3D12_INDEX_BUFFER_VIEW* indexBufView)
{
	//�C���f�b�N�X�o�b�t�@������
	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto desc = CD3DX12_RESOURCE_DESC::Buffer(size);
	Hr = Device->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(indexBuf));
	assert(SUCCEEDED(Hr));

	//������o�b�t�@�Ƀf�[�^���R�s�[
	UINT8* mappedBuf = nullptr;
	Hr = (*indexBuf)->Map(0, nullptr, (void**)&mappedBuf);
	assert(SUCCEEDED(Hr));
	memcpy(mappedBuf, indices, size);
	(*indexBuf)->Unmap(0, nullptr);

	//�C���f�b�N�X�o�b�t�@�r���[������
	indexBufView->BufferLocation = (*indexBuf)->GetGPUVirtualAddress();
	indexBufView->Format = DXGI_FORMAT_R16_UINT;
	indexBufView->SizeInBytes = static_cast<UINT>(size);;
}

void GRAPHIC::createConstBuf(size_t size, ID3D12Resource** constBuf, void** constBufMap)
{
	//�R���X�^���g�o�b�t�@�P������
	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto desc = CD3DX12_RESOURCE_DESC::Buffer((size + 0xff) & ~0xff);//256�̔{���łȂ���΂Ȃ�Ȃ�
	Hr = Device->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(constBuf)
	);
	assert(SUCCEEDED(Hr));

	//�}�b�v���Ă���
	Hr = (*constBuf)->Map(0, nullptr, constBufMap);
	assert(SUCCEEDED(Hr));
}

#include <cstdlib> // wcstombs�̂��߂ɕK�v
void GRAPHIC::createTextureBuf(LPCWSTR wcFilename, ID3D12Resource** textureBuf)
{
	char mbFilename[100]; // �ϊ���̃}���`�o�C�g������̃o�b�t�@
	size_t count = 100; // �ϊ�����ő�o�C�g��
	//���C�h��������}���`�o�C�g������ɕϊ�
	size_t result;	wcstombs_s(&result, mbFilename, count, wcFilename, count);
	assert(result > 0);

	//�t�@�C����ǂݍ��݁A���f�[�^�����o��
	unsigned char* pixels;
	int width, height, bytePerPixel;
	pixels = stbi_load(mbFilename, &width, &height, &bytePerPixel, 4);
	WARNING(pixels == nullptr, L"�e�N�X�`�����ǂݍ��߂܂���", wcFilename);
	
	//�P�s�̃s�b�`��256�̔{���ɂ��Ă���(�o�b�t�@�T�C�Y��256�̔{���łȂ���΂����Ȃ�)
	const UINT64 alignedRowPitch = (width * bytePerPixel + 0xff) & ~0xff;

	//�A�b�v���[�h�p���ԃo�b�t�@������A���f�[�^���R�s�[���Ă���
	ComPtr<ID3D12Resource> uploadBuf;
	{
		//�e�N�X�`���ł͂Ȃ��t�c�[�̃o�b�t�@�Ƃ��Ă���
		auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto desc = CD3DX12_RESOURCE_DESC::Buffer(alignedRowPitch * height);
		Hr = Device->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&uploadBuf));
		assert(SUCCEEDED(Hr));

		//���f�[�^��uploadbuff�Ɉ�U�R�s�[���܂�
		uint8_t* mapBuf = nullptr;
		Hr = uploadBuf->Map(0, nullptr, (void**)&mapBuf);//�}�b�v
		auto srcAddress = pixels;
		auto originalRowPitch = width * bytePerPixel;
		for (int y = 0; y < height; ++y) {
			memcpy(mapBuf, srcAddress, originalRowPitch);
			//1�s���Ƃ̒�������킹�Ă��
			srcAddress += originalRowPitch;
			mapBuf += alignedRowPitch;
		}
		uploadBuf->Unmap(0, nullptr);//�A���}�b�v
	}

	//�����āA�ŏI�R�s�[��ł���e�N�X�`���o�b�t�@�����
	{
		auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, width, height);
		Hr = Device->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(textureBuf));
		assert(SUCCEEDED(Hr));
	}

	//uploadBuf����textureBuf�փR�s�[���钷�����̂肪�n�܂�܂�

	//�܂��R�s�[�����P�[�V�����̏����E�t�b�g�v�����g�w��
	D3D12_TEXTURE_COPY_LOCATION src = {};
	src.pResource = uploadBuf.Get();
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src.PlacedFootprint.Footprint.Width = static_cast<UINT>(width);
	src.PlacedFootprint.Footprint.Height = static_cast<UINT>(height);
	src.PlacedFootprint.Footprint.Depth = static_cast<UINT>(1);
	src.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(alignedRowPitch);
	src.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//�R�s�[�惍�P�[�V�����̏����E�T�u���\�[�X�C���f�b�N�X�w��
	D3D12_TEXTURE_COPY_LOCATION dst = {};
	dst.pResource = *textureBuf;
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0;

	//�R�}���h���X�g�ŃR�s�[��\�񂵂܂���I�I�I
	CommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
	//���Ă��Ƃ̓o���A������̂ł�
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		*textureBuf,
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	CommandList->ResourceBarrier(1, &barrier);
	//uploadBuf�A�����[�h
	CommandList->DiscardResource(uploadBuf.Get(), nullptr);
	//�R�}���h���X�g�����
	CommandList->Close();
	//���s
	ID3D12CommandList* commandLists[] = { CommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
	//���\�[�X��GPU�ɓ]�������܂őҋ@����
	WaitDrawDone();

	//�R�}���h�A���P�[�^�����Z�b�g
	HRESULT Hr = CommandAllocator->Reset();
	assert(SUCCEEDED(Hr));
	//�R�}���h���X�g�����Z�b�g
	Hr = CommandList->Reset(CommandAllocator.Get(), nullptr);
	assert(SUCCEEDED(Hr));

	//�J��
	stbi_image_free(pixels);
}

void GRAPHIC::createCbvTbvHeap(ID3D12Resource* constBuf1, ID3D12Resource* constBuf2, ID3D12Resource* textureBuf, ID3D12DescriptorHeap** CbvTbvHeap)
{
	//�u�r���[�v�̓��ꕨ�ł���u�f�B�X�N���v�^�q�[�v�v������
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = 4;//�R���X�^���g�o�b�t�@3�ƃe�N�X�`���o�b�t�@�P��
		desc.NodeMask = 0;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		Hr = Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(CbvTbvHeap));
		assert(SUCCEEDED(Hr));
	}

	auto hCbvTbvHeap = (*CbvTbvHeap)->GetCPUDescriptorHandleForHeapStart();
	auto cbvTbvHeapSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//�R���X�^���g�o�b�t�@�O�́u�f�B�X�N���v�^�q�[�v�v�����́u�f�B�X�N���v�^�q�[�v�v�ɃR�s�[
	{
		auto hCb0vHeap = Cb0vHeap->GetCPUDescriptorHandleForHeapStart();
		Device->CopyDescriptorsSimple(1, hCbvTbvHeap, hCb0vHeap, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	//�R���X�^���g�o�b�t�@�P�́u�r���[�v���u�f�B�X�N���v�^�q�[�v�v�ɂ���
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
		desc.BufferLocation = constBuf1->GetGPUVirtualAddress();
		desc.SizeInBytes = static_cast<UINT>(constBuf1->GetDesc().Width);
		hCbvTbvHeap.ptr += cbvTbvHeapSize;
		Device->CreateConstantBufferView(&desc, hCbvTbvHeap);
	}
	//�R���X�^���g�o�b�t�@�Q�́u�r���[�v���u�f�B�X�N���v�^�q�[�v�v�ɂ���
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
		desc.BufferLocation = constBuf2->GetGPUVirtualAddress();
		desc.SizeInBytes = static_cast<UINT>(constBuf2->GetDesc().Width);
		hCbvTbvHeap.ptr += cbvTbvHeapSize;
		Device->CreateConstantBufferView(&desc, hCbvTbvHeap);
	}
	//�e�N�X�`���o�b�t�@�́u�r���[�v���u�f�B�X�N���v�^�q�[�v�v�ɂ���
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
		desc.Format = textureBuf->GetDesc().Format;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2D�e�N�X�`��
		desc.Texture2D.MipLevels = 1;//�~�b�v�}�b�v�͎g�p���Ȃ��̂�1
		hCbvTbvHeap.ptr += cbvTbvHeapSize;
		Device->CreateShaderResourceView(textureBuf, &desc, hCbvTbvHeap);
	}
}

void GRAPHIC::draw(
	D3D12_VERTEX_BUFFER_VIEW& vbv,
	D3D12_INDEX_BUFFER_VIEW& ibv, 
	ID3D12DescriptorHeap* cbvTbvHeap
)
{
	//���_���Z�b�g
	CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	CommandList->IASetVertexBuffers(0, 1, &vbv);
	CommandList->IASetIndexBuffer(&ibv);
	//�R�}���h���X�g�Ƀf�B�X�N���v�^�q�[�v���Z�b�g
	CommandList->SetDescriptorHeaps(1, &cbvTbvHeap);
	//�f�B�X�N���v�^�e�[�u���Ƀf�B�X�N���v�^�q�[�v���Z�b�g
	auto hCbvTbvHeap = cbvTbvHeap->GetGPUDescriptorHandleForHeapStart();
	CommandList->SetGraphicsRootDescriptorTable(0, hCbvTbvHeap);
	//�`��
	CommandList->DrawIndexedInstanced(ibv.SizeInBytes/2, 1, 0, 0, 0);
}

void GRAPHIC::WaitDrawDone()
{
	//���݂�Fence�l���R�}���h�I�����Fence�ɏ������܂��悤�ɂ���
	UINT64 fvalue = FenceValue;
	CommandQueue->Signal(Fence.Get(), fvalue);
	FenceValue++;

	//�܂��R�}���h�L���[���I�����Ă��Ȃ����Ƃ��m�F����
	if (Fence->GetCompletedValue() < fvalue)
	{
		//����Fence�ɂ����āAfvalue �̒l�ɂȂ�����C�x���g�𔭐�������
		Fence->SetEventOnCompletion(fvalue, FenceEvent);
		//�C�x���g����������܂ő҂�
		WaitForSingleObject(FenceEvent, INFINITE);
	}
}

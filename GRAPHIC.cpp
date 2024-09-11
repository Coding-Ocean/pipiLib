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

	//デバイス、コマンド、フェンス
	{
#ifdef _DEBUG
		//デバッグバージョンではデバッグレイヤーを有効化する
		{
			ComPtr<ID3D12Debug> debug;
			Hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
			assert(SUCCEEDED(Hr));
			debug->EnableDebugLayer();
		}
#endif
		//デバイスをつくる(簡易バージョン)
		{
			Hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0,
				IID_PPV_ARGS(&Device));
			assert(SUCCEEDED(Hr));
		}
		//コマンド
		{
			//コマンドアロケータをつくる
			Hr = Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(&CommandAllocator));
			assert(SUCCEEDED(Hr));

			//コマンドリストをつくる
			Hr = Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
				CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&CommandList));
			assert(SUCCEEDED(Hr));

			//コマンドキューをつくる
			D3D12_COMMAND_QUEUE_DESC desc = {};
			desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;		//GPUタイムアウトが有効
			desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;		//直接コマンドキュー
			Hr = Device->CreateCommandQueue(&desc, IID_PPV_ARGS(&CommandQueue));
			assert(SUCCEEDED(Hr));
		}
		//フェンス
		{
			//GPUの処理完了をチェックするフェンスをつくる
			Hr = Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence));
			assert(SUCCEEDED(Hr));
			FenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
			assert(FenceEvent != nullptr);
			FenceValue = 1;
		}
	}
	//レンダーターゲット
	{
		//スワップチェインをつくる(ここにバックバッファが含まれている)
		{
			//DXGIファクトリをつくる
			ComPtr<IDXGIFactory6> dxgiFactory;
			Hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory));
			assert(SUCCEEDED(Hr));

			//スワップチェインをつくる
			DXGI_SWAP_CHAIN_DESC1 desc = {};
			desc.BufferCount = 2; //バックバッファ2枚
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

			//IDXGISwapChain4インターフェイスをサポートしているか尋ねる
			Hr = swapChain.As(&SwapChain);
			assert(SUCCEEDED(Hr));
		}
		//バックバッファ「ビュー」の入れ物である「ディスクリプタヒープ」をつくる
		{
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			desc.NumDescriptors = 2;//バックバッファビュー２つ
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//RenderTargetView
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;//シェーダからアクセスしないのでNONEでOK
			Hr = Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&BbvHeap));
			assert(SUCCEEDED(Hr));
		}
		//バックバッファ「ビュー」を「ディスクリプタヒープ」につくる
		{
			D3D12_CPU_DESCRIPTOR_HANDLE hBbvHeap
				= BbvHeap->GetCPUDescriptorHandleForHeapStart();

			BbvHeapSize
				= Device->GetDescriptorHandleIncrementSize(
					D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

			for (UINT idx = 0; idx < 2; idx++) {
				//バックバッファを取り出す
				Hr = SwapChain->GetBuffer(idx, IID_PPV_ARGS(&BackBufs[idx]));
				assert(SUCCEEDED(Hr));
				//バックバッファのビューをヒープにつくる
				hBbvHeap.ptr += idx * BbvHeapSize;
				Device->CreateRenderTargetView(BackBufs[idx].Get(), nullptr, hBbvHeap);
			}
		}

		//デプスステンシルバッファをつくる
		{
			D3D12_HEAP_PROPERTIES prop = {};
			prop.Type = D3D12_HEAP_TYPE_DEFAULT;//DEFAULTだから後はUNKNOWNでよし
			prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			D3D12_RESOURCE_DESC desc = {};
			desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//2次元のテクスチャデータとして
			desc.Width = w.clientWidth();//幅はバックバッファと同じ
			desc.Height = w.clientHeight();//高さはバックバッファと同じ
			desc.DepthOrArraySize = 1;//テクスチャ配列でもないし3Dテクスチャでもない
			desc.Format = DXGI_FORMAT_D32_FLOAT;//深度値書き込み用フォーマット
			desc.SampleDesc.Count = 1;//サンプルは1ピクセル当たり1つ
			desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;//このバッファは深度ステンシルとして使用します
			desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			desc.MipLevels = 1;
			//デプスステンシルバッファをクリアする値
			D3D12_CLEAR_VALUE depthClearValue = {};
			depthClearValue.DepthStencil.Depth = 1.0f;//深さ１(最大値)でクリア
			depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;//32bit深度値としてクリア
			//デプスステンシルバッファを作る
			Hr = Device->CreateCommittedResource(
				&prop,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_DEPTH_WRITE, //デプス書き込みに使用
				&depthClearValue,
				IID_PPV_ARGS(&DepthStencilBuf));
			assert(SUCCEEDED(Hr));
		}
		//デプスステンシルバッファ「ビュー」の入れ物である「デスクリプタヒープ」をつくる
		{
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};//深度に使うよという事がわかればいい
			desc.NumDescriptors = 1;//深度ビュー1つのみ
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;//デプスステンシルビューとして使う
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			Hr = Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&DsvHeap));
			assert(SUCCEEDED(Hr));
		}
		//デプスステンシルバッファ「ビュー」を「ディスクリプタヒープ」につくる
		{
			D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
			desc.Format = DXGI_FORMAT_D32_FLOAT;//デプス値に32bit使用
			desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2Dテクスチャ
			desc.Flags = D3D12_DSV_FLAG_NONE;//フラグは特になし
			D3D12_CPU_DESCRIPTOR_HANDLE hDsvHeap
				= DsvHeap->GetCPUDescriptorHandleForHeapStart();
			Device->CreateDepthStencilView(DepthStencilBuf.Get(), &desc, hDsvHeap);
		}
	}{}
	//コンスタントバッファ０をつくり、デフォルト値をいれておく
	{
		createConstBuf(256, &ConstBuf0, (void**)&ConstBuf0Map);
		//コンスタントバッファ０の「ビュー」の入れ物である「ディスクリプタヒープ」をつくる
		{
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			desc.NumDescriptors = 1;
			desc.NodeMask = 0;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;// D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			Hr = Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&Cb0vHeap));
			assert(SUCCEEDED(Hr));
		}
		//コンスタントバッファ０の「ビュー」を「ディスクリプタヒープ」につくる
		{
			D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
			desc.BufferLocation = ConstBuf0->GetGPUVirtualAddress();
			desc.SizeInBytes = static_cast<UINT>(ConstBuf0->GetDesc().Width);
			auto hCbvHeap = Cb0vHeap->GetCPUDescriptorHandleForHeapStart();
			Device->CreateConstantBufferView(&desc, hCbvHeap);
		}
		//コンスタントバッファ０デフォルト設定
		//ビューマトリックス
		FLOAT3 eye = { 0, 0, 2 }, focus = { 0, 0, 0 }, up = { 0, 1, 0 };
		ConstBuf0Map->view.lookat(eye, focus, up);
		//プロジェクションマトリックス
		float aspect = static_cast<float>(w.clientWidth()) / w.clientHeight();
		ConstBuf0Map->proj.pers(3.141592f / 4, aspect, 1.0f, 10.0f);
		//ライトの位置
		ConstBuf0Map->lightPos = FLOAT3(0, 0, 1);
	}

	//パイプライン
	{
		//ルートシグネチャ
		{
			//ディスクリプタレンジ。ディスクリプタヒープとシェーダを紐づけるもの。
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

			//ルートパラメタをディスクリプタテーブルとして使用
			D3D12_ROOT_PARAMETER rootParam[1] = {};
			rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParam[0].DescriptorTable.pDescriptorRanges = range;
			rootParam[0].DescriptorTable.NumDescriptorRanges = _countof(range);
			rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

			//サンプラの記述。このサンプラがシェーダーの s0 にセットされる
			D3D12_STATIC_SAMPLER_DESC samplerDesc[1] = {};
			samplerDesc[0].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;//補間しない(ニアレストネイバー)
			samplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//横繰り返し
			samplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//縦繰り返し
			samplerDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//奥行繰り返し
			samplerDesc[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;//ボーダーの時は黒
			samplerDesc[0].MaxLOD = D3D12_FLOAT32_MAX;//ミップマップ最大値
			samplerDesc[0].MinLOD = 0.0f;//ミップマップ最小値
			samplerDesc[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//オーバーサンプリングの際リサンプリングしない？
			samplerDesc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//ピクセルシェーダからのみ可視

			//ルートシグニチャの記述。上のrootParamとsamplerDescを１つにまとめる。
			D3D12_ROOT_SIGNATURE_DESC desc = {};
			desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
			desc.pParameters = rootParam;//ルートパラメータの先頭アドレス
			desc.NumParameters = _countof(rootParam);//ルートパラメータ数
			desc.pStaticSamplers = samplerDesc;//サンプラーの先頭アドレス
			desc.NumStaticSamplers = _countof(samplerDesc);//サンプラー数

			//ルートシグネチャをシリアライズ⇒blob(塊)をつくる。
			ID3DBlob* blob;
			Hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, nullptr);
			assert(SUCCEEDED(Hr));

			//ルートシグネチャをつくる
			Hr = Device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(),
				IID_PPV_ARGS(&RootSignature));
			assert(SUCCEEDED(Hr));
			blob->Release();
		}

		//シェーダをリソースから読み込む
		SHADER_RESOURCE vs(L"LAMBERT_VS");
		assert(vs.succeeded());
		SHADER_RESOURCE ps(L"LAMBERT_PS");
		assert(ps.succeeded());

		
		//以下、各種記述

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
		depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;//全て書き込み
		depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;//小さい方を採用
		depthStencilDesc.StencilEnable = false;//ステンシルバッファは使わない


		//ここまでの記述をまとめてパイプラインステートオブジェクトをつくる
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

		//出力領域を設定
		Viewport.TopLeftX = 0.0f;
		Viewport.TopLeftY = 0.0f;
		Viewport.Width = static_cast<float>(w.clientWidth());
		Viewport.Height = static_cast<float>(w.clientHeight());
		Viewport.MinDepth = 0.0f;
		Viewport.MaxDepth = 1.0f;

		//切り取り矩形を設定
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
	//現在のバックバッファのインデックスを取得。このプログラムの場合0 or 1になる。
	BackBufIdx = SwapChain->GetCurrentBackBufferIndex();

	//バックバッファの遷移
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		BackBufs[BackBufIdx].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandList->ResourceBarrier(1, &barrier);

	//バックバッファのディスクリプタハンドルを用意する
	auto hBbvHeap = BbvHeap->GetCPUDescriptorHandleForHeapStart();
	hBbvHeap.ptr += BackBufIdx * BbvHeapSize;
	//デプスステンシルバッファのディスクリプタハンドルを用意する
	auto hDsvHeap = DsvHeap->GetCPUDescriptorHandleForHeapStart();
	//バックバッファとデプスステンシルバッファを描画ターゲットとして設定する
	CommandList->OMSetRenderTargets(1, &hBbvHeap, false, &hDsvHeap);

	//描画ターゲットをクリアする
	const float clearColor[] = { r, g, b, 1.0f };
	CommandList->ClearRenderTargetView(hBbvHeap, clearColor, 0, nullptr);
	//デプスステンシルバッファをクリアする
	CommandList->ClearDepthStencilView(hDsvHeap, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//ビューポートとシザー矩形をセット
	CommandList->RSSetViewports(1, &Viewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);
}

void GRAPHIC::present()
{
	//バックバッファの遷移
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		BackBufs[BackBufIdx].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);
	CommandList->ResourceBarrier(1, &barrier);

	//コマンドリストをクローズする
	CommandList->Close();
	//コマンドリストを実行する
	ID3D12CommandList* commandLists[] = { CommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
	//描画完了を待つ
	WaitDrawDone();

	//バッファを表示
	SwapChain->Present(1, 0);

	//コマンドアロケータをリセット
	Hr = CommandAllocator->Reset();
	assert(SUCCEEDED(Hr));
	//コマンドリストをリセット
	Hr = CommandList->Reset(CommandAllocator.Get(), nullptr);
	assert(SUCCEEDED(Hr));
}

void GRAPHIC::setPipeline()
{
	//パイプラインステートをセット
	CommandList->SetPipelineState(PipelineState.Get());
	//ルートシグニチャ
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

	//位置バッファに生データをコピー
	UINT8* mappedBuf;
	Hr = (*vertexBuf)->Map(0, nullptr, reinterpret_cast<void**>(&mappedBuf));
	assert(SUCCEEDED(Hr));
	memcpy(mappedBuf, vertices, size);
	(*vertexBuf)->Unmap(0, nullptr);

	//位置バッファのビューを初期化しておく。（ディスクリプタヒープに作らなくてよい）
	vertexBufView->BufferLocation = (*vertexBuf)->GetGPUVirtualAddress();
	vertexBufView->SizeInBytes = static_cast<UINT>(size);
	vertexBufView->StrideInBytes = stride;
}

void GRAPHIC::createIndexBuf(unsigned short* indices, size_t size, ID3D12Resource** indexBuf, D3D12_INDEX_BUFFER_VIEW* indexBufView)
{
	//インデックスバッファをつくる
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

	//作ったバッファにデータをコピー
	UINT8* mappedBuf = nullptr;
	Hr = (*indexBuf)->Map(0, nullptr, (void**)&mappedBuf);
	assert(SUCCEEDED(Hr));
	memcpy(mappedBuf, indices, size);
	(*indexBuf)->Unmap(0, nullptr);

	//インデックスバッファビューをつくる
	indexBufView->BufferLocation = (*indexBuf)->GetGPUVirtualAddress();
	indexBufView->Format = DXGI_FORMAT_R16_UINT;
	indexBufView->SizeInBytes = static_cast<UINT>(size);;
}

void GRAPHIC::createConstBuf(size_t size, ID3D12Resource** constBuf, void** constBufMap)
{
	//コンスタントバッファ１をつくる
	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto desc = CD3DX12_RESOURCE_DESC::Buffer((size + 0xff) & ~0xff);//256の倍数でなければならない
	Hr = Device->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(constBuf)
	);
	assert(SUCCEEDED(Hr));

	//マップしておく
	Hr = (*constBuf)->Map(0, nullptr, constBufMap);
	assert(SUCCEEDED(Hr));
}

#include <cstdlib> // wcstombsのために必要
void GRAPHIC::createTextureBuf(LPCWSTR wcFilename, ID3D12Resource** textureBuf)
{
	char mbFilename[100]; // 変換先のマルチバイト文字列のバッファ
	size_t count = 100; // 変換する最大バイト数
	//ワイド文字列をマルチバイト文字列に変換
	size_t result;	wcstombs_s(&result, mbFilename, count, wcFilename, count);
	assert(result > 0);

	//ファイルを読み込み、生データを取り出す
	unsigned char* pixels;
	int width, height, bytePerPixel;
	pixels = stbi_load(mbFilename, &width, &height, &bytePerPixel, 4);
	WARNING(pixels == nullptr, L"テクスチャが読み込めません", wcFilename);
	
	//１行のピッチを256の倍数にしておく(バッファサイズは256の倍数でなければいけない)
	const UINT64 alignedRowPitch = (width * bytePerPixel + 0xff) & ~0xff;

	//アップロード用中間バッファをつくり、生データをコピーしておく
	ComPtr<ID3D12Resource> uploadBuf;
	{
		//テクスチャではなくフツーのバッファとしてつくる
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

		//生データをuploadbuffに一旦コピーします
		uint8_t* mapBuf = nullptr;
		Hr = uploadBuf->Map(0, nullptr, (void**)&mapBuf);//マップ
		auto srcAddress = pixels;
		auto originalRowPitch = width * bytePerPixel;
		for (int y = 0; y < height; ++y) {
			memcpy(mapBuf, srcAddress, originalRowPitch);
			//1行ごとの辻褄を合わせてやる
			srcAddress += originalRowPitch;
			mapBuf += alignedRowPitch;
		}
		uploadBuf->Unmap(0, nullptr);//アンマップ
	}

	//そして、最終コピー先であるテクスチャバッファを作る
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

	//uploadBufからtextureBufへコピーする長い道のりが始まります

	//まずコピー元ロケーションの準備・フットプリント指定
	D3D12_TEXTURE_COPY_LOCATION src = {};
	src.pResource = uploadBuf.Get();
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src.PlacedFootprint.Footprint.Width = static_cast<UINT>(width);
	src.PlacedFootprint.Footprint.Height = static_cast<UINT>(height);
	src.PlacedFootprint.Footprint.Depth = static_cast<UINT>(1);
	src.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(alignedRowPitch);
	src.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//コピー先ロケーションの準備・サブリソースインデックス指定
	D3D12_TEXTURE_COPY_LOCATION dst = {};
	dst.pResource = *textureBuf;
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0;

	//コマンドリストでコピーを予約しますよ！！！
	CommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
	//ってことはバリアがいるのです
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		*textureBuf,
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	CommandList->ResourceBarrier(1, &barrier);
	//uploadBufアンロード
	CommandList->DiscardResource(uploadBuf.Get(), nullptr);
	//コマンドリストを閉じて
	CommandList->Close();
	//実行
	ID3D12CommandList* commandLists[] = { CommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
	//リソースがGPUに転送されるまで待機する
	WaitDrawDone();

	//コマンドアロケータをリセット
	HRESULT Hr = CommandAllocator->Reset();
	assert(SUCCEEDED(Hr));
	//コマンドリストをリセット
	Hr = CommandList->Reset(CommandAllocator.Get(), nullptr);
	assert(SUCCEEDED(Hr));

	//開放
	stbi_image_free(pixels);
}

void GRAPHIC::createCbvTbvHeap(ID3D12Resource* constBuf1, ID3D12Resource* constBuf2, ID3D12Resource* textureBuf, ID3D12DescriptorHeap** CbvTbvHeap)
{
	//「ビュー」の入れ物である「ディスクリプタヒープ」をつくる
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = 4;//コンスタントバッファ3つとテクスチャバッファ１つ
		desc.NodeMask = 0;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		Hr = Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(CbvTbvHeap));
		assert(SUCCEEDED(Hr));
	}

	auto hCbvTbvHeap = (*CbvTbvHeap)->GetCPUDescriptorHandleForHeapStart();
	auto cbvTbvHeapSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//コンスタントバッファ０の「ディスクリプタヒープ」をこの「ディスクリプタヒープ」にコピー
	{
		auto hCb0vHeap = Cb0vHeap->GetCPUDescriptorHandleForHeapStart();
		Device->CopyDescriptorsSimple(1, hCbvTbvHeap, hCb0vHeap, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	//コンスタントバッファ１の「ビュー」を「ディスクリプタヒープ」につくる
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
		desc.BufferLocation = constBuf1->GetGPUVirtualAddress();
		desc.SizeInBytes = static_cast<UINT>(constBuf1->GetDesc().Width);
		hCbvTbvHeap.ptr += cbvTbvHeapSize;
		Device->CreateConstantBufferView(&desc, hCbvTbvHeap);
	}
	//コンスタントバッファ２の「ビュー」を「ディスクリプタヒープ」につくる
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
		desc.BufferLocation = constBuf2->GetGPUVirtualAddress();
		desc.SizeInBytes = static_cast<UINT>(constBuf2->GetDesc().Width);
		hCbvTbvHeap.ptr += cbvTbvHeapSize;
		Device->CreateConstantBufferView(&desc, hCbvTbvHeap);
	}
	//テクスチャバッファの「ビュー」を「ディスクリプタヒープ」につくる
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
		desc.Format = textureBuf->GetDesc().Format;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
		desc.Texture2D.MipLevels = 1;//ミップマップは使用しないので1
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
	//頂点をセット
	CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	CommandList->IASetVertexBuffers(0, 1, &vbv);
	CommandList->IASetIndexBuffer(&ibv);
	//コマンドリストにディスクリプタヒープをセット
	CommandList->SetDescriptorHeaps(1, &cbvTbvHeap);
	//ディスクリプタテーブルにディスクリプタヒープをセット
	auto hCbvTbvHeap = cbvTbvHeap->GetGPUDescriptorHandleForHeapStart();
	CommandList->SetGraphicsRootDescriptorTable(0, hCbvTbvHeap);
	//描画
	CommandList->DrawIndexedInstanced(ibv.SizeInBytes/2, 1, 0, 0, 0);
}

void GRAPHIC::WaitDrawDone()
{
	//現在のFence値がコマンド終了後にFenceに書き込まれるようにする
	UINT64 fvalue = FenceValue;
	CommandQueue->Signal(Fence.Get(), fvalue);
	FenceValue++;

	//まだコマンドキューが終了していないことを確認する
	if (Fence->GetCompletedValue() < fvalue)
	{
		//このFenceにおいて、fvalue の値になったらイベントを発生させる
		Fence->SetEventOnCompletion(fvalue, FenceEvent);
		//イベントが発生するまで待つ
		WaitForSingleObject(FenceEvent, INFINITE);
	}
}

#include "pch.h"
#include "GRAPHIC.h"
#include "FILE_BUFFER.h"
#include "STATIC_MESH.h"

STATIC_MESH::STATIC_MESH()
{
}

STATIC_MESH::~STATIC_MESH()
{
	ConstBuf1->Unmap(0, nullptr);
	ConstBuf2->Unmap(0, nullptr);
}

void STATIC_MESH::create(const char* filename)
{
	auto& g = GRAPHIC::instance();

	FILE_BUFFER fb(filename);

	//頂点バッファをつくる
	{
		fb.readOnAssumption("v");
		fb.readString();//頂点名
		fb.readString();//頂点形式
		int numElements = fb.readInt() * 8;//頂点数 * １頂点の要素数
		std::vector<float> vertices(numElements);
		for (int i = 0; i < numElements; ++i) {
			vertices[i] = fb.readFloat();
		}
		size_t size = sizeof(float) * numElements;//全データのバイト数
		size_t stride = sizeof(float) * 8;//１頂点のバイト数
		g.createVertexBuf(vertices.data(), size, stride, &VertexBuf, &Vbv);
	}
	//頂点インデックスバッファをつくる
	{
		fb.readOnAssumption("i");
		fb.readString();//インデックス名
		int numElements = fb.readInt();
		std::vector<unsigned short> indices(numElements);
		for (int i = 0; i < numElements; ++i) {
			indices[i] = fb.readInt();
		}
		size_t size = sizeof(unsigned short) * numElements;
		g.createIndexBuf(indices.data(), size, &IndexBuf, &Ibv);
	}

	//コンスタントバッファ１(World matrix)をつくる
	g.createConstBuf(sizeof(GRAPHIC::CONST_BUF1), &ConstBuf1, (void**)&ConstBuf1Map);
	ConstBuf1Map->world.identity();

	//コンスタントバッファ２(Material)をつくる
	g.createConstBuf(sizeof(GRAPHIC::CONST_BUF2), &ConstBuf2, (void**)&ConstBuf2Map);
	ConstBuf2Map->diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
	ConstBuf2Map->ambient = { 0.0f, 0.0f, 0.0f, 1.0f };
	ConstBuf2Map->specular = { 0.0f, 0.0f, 0.0f, 1.0f };

	//テクスチャバッファをつくる
	fb.readOnAssumption("x");
	fb.readString();
	g.createTextureBuf(fb.readString(), &TextureBuf);

	//ディスクリプタヒープをつくる
	g.createCbvTbvHeap(ConstBuf1.Get(), ConstBuf2.Get(), TextureBuf.Get(), &CbvTbvHeap);
}

void STATIC_MESH::update(const MATRIX& world)
{
	ConstBuf1Map->world = world;
}

void STATIC_MESH::draw()
{
	auto& g = GRAPHIC::instance();
	g.draw(Vbv, Ibv, CbvTbvHeap.Get());
}

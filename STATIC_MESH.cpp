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

	//���_�o�b�t�@������
	{
		fb.readOnAssumption("v");
		fb.readString();//���_��
		fb.readString();//���_�`��
		int numElements = fb.readInt() * 8;//���_�� * �P���_�̗v�f��
		std::vector<float> vertices(numElements);
		for (int i = 0; i < numElements; ++i) {
			vertices[i] = fb.readFloat();
		}
		size_t size = sizeof(float) * numElements;//�S�f�[�^�̃o�C�g��
		size_t stride = sizeof(float) * 8;//�P���_�̃o�C�g��
		g.createVertexBuf(vertices.data(), size, stride, &VertexBuf, &Vbv);
	}
	//���_�C���f�b�N�X�o�b�t�@������
	{
		fb.readOnAssumption("i");
		fb.readString();//�C���f�b�N�X��
		int numElements = fb.readInt();
		std::vector<unsigned short> indices(numElements);
		for (int i = 0; i < numElements; ++i) {
			indices[i] = fb.readInt();
		}
		size_t size = sizeof(unsigned short) * numElements;
		g.createIndexBuf(indices.data(), size, &IndexBuf, &Ibv);
	}

	//�R���X�^���g�o�b�t�@�P(World matrix)������
	g.createConstBuf(sizeof(GRAPHIC::CONST_BUF1), &ConstBuf1, (void**)&ConstBuf1Map);
	ConstBuf1Map->world.identity();

	//�R���X�^���g�o�b�t�@�Q(Material)������
	g.createConstBuf(sizeof(GRAPHIC::CONST_BUF2), &ConstBuf2, (void**)&ConstBuf2Map);
	ConstBuf2Map->diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
	ConstBuf2Map->ambient = { 0.0f, 0.0f, 0.0f, 1.0f };
	ConstBuf2Map->specular = { 0.0f, 0.0f, 0.0f, 1.0f };

	//�e�N�X�`���o�b�t�@������
	fb.readOnAssumption("x");
	fb.readString();
	g.createTextureBuf(fb.readString(), &TextureBuf);

	//�f�B�X�N���v�^�q�[�v������
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

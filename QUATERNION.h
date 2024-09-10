#pragma once
#include"FLOAT3.h"

// (Unit) QUATERNION
class QUATERNION
{
public:
	float x;
	float y;
	float z;
	float w;

	QUATERNION()
	{
		*this = QUATERNION::Identity;
	}

	//�N�H�[�^�j�I�������𒼐ڐݒ肷��
	explicit QUATERNION(float inX, float inY, float inZ, float inW)
	{
		Set(inX, inY, inZ, inW);
	}

	//���Ɗp�x����N�H�[�^�j�I�����\�z����
	//���͂��łɐ��K������Ă�����̂Ƃ���
	explicit QUATERNION(const FLOAT3& axis, float angle)
	{
		float scalar = sinf(angle / 2.0f);
		x = axis.x * scalar;
		y = axis.y * scalar;
		z = axis.z * scalar;
		w = cosf(angle / 2.0f);
	}

	void Set(float inX, float inY, float inZ, float inW)
	{
		x = inX;
		y = inY;
		z = inZ;
		w = inW;
	}

	void Conjugate()
	{
		x *= -1.0f;
		y *= -1.0f;
		z *= -1.0f;
	}

	float LengthSq() const
	{
		return (x * x + y * y + z * z + w * w);
	}

	float Length() const
	{
		return sqrtf(LengthSq());
	}

	void Normalize()
	{
		float length = Length();
		x /= length;
		y /= length;
		z /= length;
		w /= length;
	}

	//���K��
	static QUATERNION Normalize(const QUATERNION& q)
	{
		QUATERNION retVal = q;
		retVal.Normalize();
		return retVal;
	}

	//���`�⊮
	//static QUATERNION Lerp(const QUATERNION& a, const QUATERNION& b, float f)
	//{
	//	QUATERNION retVal;
	//	retVal.x = lerp(a.x, b.x, f);
	//	retVal.y = lerp(a.y, b.y, f);
	//	retVal.z = lerp(a.z, b.z, f);
	//	retVal.w = lerp(a.w, b.w, f);
	//	retVal.Normalize();
	//	return retVal;
	//}

	static float Dot(const QUATERNION& a, const QUATERNION& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	}

	//���ʐ��`�⊮
	static QUATERNION Slerp(const QUATERNION& a, const QUATERNION& b, float f)
	{
		float rawCosm = QUATERNION::Dot(a, b);

		float cosom = -rawCosm;
		if (rawCosm >= 0.0f)
		{
			cosom = rawCosm;
		}

		float scale0, scale1;

		if (cosom < 0.9999f)
		{
			const float omega = acosf(cosom);
			const float invSin = 1.f / sinf(omega);
			scale0 = sinf((1.f - f) * omega) * invSin;
			scale1 = sinf(f * omega) * invSin;
		}
		else
		{
			//�����N�H�[�^�j�I�����������ɂ���Ȃ�A���`�⊮�ł���
			scale0 = 1.0f - f;
			scale1 = f;
		}

		if (rawCosm < 0.0f)
		{
			scale1 = -scale1;
		}

		QUATERNION retVal;
		retVal.x = scale0 * a.x + scale1 * b.x;
		retVal.y = scale0 * a.y + scale1 * b.y;
		retVal.z = scale0 * a.z + scale1 * b.z;
		retVal.w = scale0 * a.w + scale1 * b.w;
		retVal.Normalize();
		return retVal;
	}

	// ����
	// ��ɂ��A��ł��̏��ɉ�]
	static QUATERNION Concatenate(const QUATERNION& q, const QUATERNION& p)
	{
		QUATERNION retVal;

		// �x�N�^�[�����́Fps * qv + qs * pv + pv x qv
		FLOAT3 qv(q.x, q.y, q.z);
		FLOAT3 pv(p.x, p.y, p.z);
		FLOAT3 newVec = p.w * qv + q.w * pv + cross(pv, qv);
		retVal.x = newVec.x;
		retVal.y = newVec.y;
		retVal.z = newVec.z;

		// �X�J���[�����́Fps * qs - pv . qv
		retVal.w = p.w * q.w - dot(pv, qv);

		return retVal;
	}

	static const QUATERNION Identity;
};

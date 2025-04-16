#include "MathUtils.h"
#include <DirectXMath.h>

namespace Hook
{
    namespace MathUtils
    {
        using namespace DirectX;
        
        XMMATRIX GetProjectionMatrix(float fov)
        {
			auto windowWidth = RE::BSGraphics::RendererData::GetSingleton()->renderWindow->windowWidth;
			auto windowHeight = RE::BSGraphics::RendererData::GetSingleton()->renderWindow->windowHeight;

            return XMMatrixPerspectiveFovLH(
                fov,                         // Field of view angle
                windowWidth / windowHeight,    // Aspect ratio
                0.1f,                         // Near clipping plane distance
                1000.0f                       // Far clipping plane distance
            );
        }

        XMMATRIX NiMatrixToXMMatrix(const RE::NiMatrix3& niMatrix)
        {
            return XMMATRIX(
                niMatrix.entry[0][0], niMatrix.entry[0][1], niMatrix.entry[0][2], 0.0f,
                niMatrix.entry[1][0], niMatrix.entry[1][1], niMatrix.entry[1][2], 0.0f,
                niMatrix.entry[2][0], niMatrix.entry[2][1], niMatrix.entry[2][2], 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            );
        }

        RE::NiMatrix3 XMMatrixToNiMatrix(const XMMATRIX& xmMatrix)
        {
            RE::NiMatrix3 niMatrix;
            XMFLOAT4X4 float4x4;
            XMStoreFloat4x4(&float4x4, xmMatrix);
            
            niMatrix.entry[0][0] = float4x4._11;
            niMatrix.entry[0][1] = float4x4._12;
            niMatrix.entry[0][2] = float4x4._13;
            
            niMatrix.entry[1][0] = float4x4._21;
            niMatrix.entry[1][1] = float4x4._22;
            niMatrix.entry[1][2] = float4x4._23;
            
            niMatrix.entry[2][0] = float4x4._31;
            niMatrix.entry[2][1] = float4x4._32;
            niMatrix.entry[2][2] = float4x4._33;
            
            return niMatrix;
        }

        RE::NiPoint3 WorldToScreen(RE::NiAVObject* cam, RE::NiAVObject* obj, float fov)
        {
			auto windowWidth = RE::BSGraphics::RendererData::GetSingleton()->renderWindow->windowWidth;
			auto windowHeight = RE::BSGraphics::RendererData::GetSingleton()->renderWindow->windowHeight;

            auto camRot = cam->world.rotate;
            auto camTrans = cam->world.translate;
            auto objTrans = obj->world.translate;

            auto worldSpacePoint = camRot * (objTrans - camTrans);

            XMMATRIX projectionMatrix = GetProjectionMatrix(fov);
            XMVECTOR clipPosition = XMVectorSet(worldSpacePoint.x, worldSpacePoint.y, worldSpacePoint.z, 1.0f);
            clipPosition = XMVector4Transform(clipPosition, projectionMatrix);

            XMVECTOR ndcPosition = clipPosition / clipPosition.m128_f32[3];

            XMVECTOR screenPosition;
            screenPosition.m128_f32[0] = (ndcPosition.m128_f32[0] + 1.0f) / 2.0f * windowWidth;
            screenPosition.m128_f32[1] = (1.0f - ndcPosition.m128_f32[1]) / 2.0f * windowHeight;
            screenPosition.m128_f32[2] = ndcPosition.m128_f32[2];

            return RE::NiPoint3(
                screenPosition.m128_f32[0], 
                screenPosition.m128_f32[1], 
                screenPosition.m128_f32[2]
            );
        }

        RE::NiPoint3 ScreenToWorld(RE::PlayerCamera* cam, const RE::NiPoint3& screenPos, float depth)
        {
			auto windowWidth = RE::BSGraphics::RendererData::GetSingleton()->renderWindow->windowWidth;
			auto windowHeight = RE::BSGraphics::RendererData::GetSingleton()->renderWindow->windowHeight;

			XMMATRIX invProjection = XMMatrixInverse(nullptr, GetProjectionMatrix(cam->firstPersonFOV));
            
            float x = (2.0f * screenPos.x) / windowWidth - 1.0f;
            float y = 1.0f - (2.0f * screenPos.y) / windowHeight;
            float z = depth;
            
            XMVECTOR clipPos = XMVectorSet(x, y, z, 1.0f);
            XMVECTOR viewPos = XMVector4Transform(clipPos, invProjection);
            viewPos /= viewPos.m128_f32[3];
            
            return RE::NiPoint3(
                viewPos.m128_f32[0],
                viewPos.m128_f32[1],
                viewPos.m128_f32[2]
            );
        }

        float DotProduct(const RE::NiPoint3& a, const RE::NiPoint3& b)
        {
            return a.x * b.x + a.y * b.y + a.z * b.z;
        }

        RE::NiPoint3 CrossProduct(const RE::NiPoint3& a, const RE::NiPoint3& b)
        {
            return RE::NiPoint3(
                a.y * b.z - a.z * b.y,
                a.z * b.x - a.x * b.z,
                a.x * b.y - a.y * b.x
            );
        }

        RE::NiPoint3 Normalize(const RE::NiPoint3& vec)
        {
            float len = Length(vec);
            if (len > 0.0f) {
                return RE::NiPoint3(vec.x / len, vec.y / len, vec.z / len);
            }
            return RE::NiPoint3();
        }

        float Length(const RE::NiPoint3& vec)
        {
            return std::sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
        }

        float Distance(const RE::NiPoint3& a, const RE::NiPoint3& b)
        {
            float dx = a.x - b.x;
            float dy = a.y - b.y;
            float dz = a.z - b.z;
            return std::sqrt(dx * dx + dy * dy + dz * dz);
        }

        RE::NiPoint3 Lerp(const RE::NiPoint3& from, const RE::NiPoint3& to, float t)
        {
            t = Clamp(t, 0.0f, 1.0f);
            return RE::NiPoint3(
                from.x + (to.x - from.x) * t,
                from.y + (to.y - from.y) * t,
                from.z + (to.z - from.z) * t
            );
        }

        float Lerp(float from, float to, float t)
        {
            t = Clamp(t, 0.0f, 1.0f);
            return from + (to - from) * t;
        }

        float Clamp(float value, float min, float max)
        {
            return value < min ? min : (value > max ? max : value);
        }

        float ToRadians(float degrees)
        {
            return degrees * (XM_PI / 180.0f);
        }

        float ToDegrees(float radians)
        {
            return radians * (180.0f / XM_PI);
        }

        bool NearlyEqual(float a, float b, float epsilon)
        {
            return std::abs(a - b) < epsilon;
        }

        float LegacyToModern(bool legacyFlag, float legacyValue)
        {
            return legacyFlag ? legacyValue : legacyValue / 1000.0f;
        }

        float ModernToLegacy(bool legacyFlag,float modernValue)
        {
            return legacyFlag ? modernValue : modernValue * 1000.0f;
        }

        bool PointInTriangle(const RE::NiPoint3& p, const RE::NiPoint3& a, 
                            const RE::NiPoint3& b, const RE::NiPoint3& c)
        {
            // 计算向量
            RE::NiPoint3 v0 = c - a;
            RE::NiPoint3 v1 = b - a;
            RE::NiPoint3 v2 = p - a;

            // 计算点积
            float dot00 = DotProduct(v0, v0);
            float dot01 = DotProduct(v0, v1);
            float dot02 = DotProduct(v0, v2);
            float dot11 = DotProduct(v1, v1);
            float dot12 = DotProduct(v1, v2);

            // 计算重心坐标
            float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
            float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
            float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

            // 检查点是否在三角形内
            return (u >= 0) && (v >= 0) && (u + v < 1);
        }

        bool RayIntersectsSphere(const RE::NiPoint3& rayOrigin, const RE::NiPoint3& rayDir,
                               const RE::NiPoint3& sphereCenter, float sphereRadius)
        {
            RE::NiPoint3 oc = rayOrigin - sphereCenter;
            float a = DotProduct(rayDir, rayDir);
            float b = 2.0f * DotProduct(oc, rayDir);
            float c = DotProduct(oc, oc) - sphereRadius * sphereRadius;
            float discriminant = b * b - 4 * a * c;
            return discriminant >= 0;
        }
    }
}

float DotProduct(NiPoint3 a, NiPoint3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

float Length(NiPoint3 p)
{
	return sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
}

NiPoint3 Normalize(NiPoint3 p)
{
	NiPoint3 ret = p;
	float l = Length(p);
	if (l == 0) {
		ret.x = 1;
		ret.y = 0;
		ret.z = 0;
	} else {
		ret.x /= l;
		ret.y /= l;
		ret.z /= l;
	}
	return ret;
}

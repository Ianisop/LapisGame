#include "LapisTypes.h"
#include <random>
#include "Helpers.h"

namespace Lapis
{
	////////////////////// Color

	Color::operator DXGI_RGBA()
	{
		return this->rgba;
	}

	Color Color::hex2rgba(const char* hex)
	{
		Color res;

		size_t len = strlen(hex);

		if (len < 8)
			res.a = 1;

		size_t at = 0;

		if (hex[0] == '#')
			at = 1;

		constexpr auto char2hex = [](char c) -> int {
			int res = 0;

			if (c >= '0' && c <= '9')
				return (res = c - '0');

			if (c >= 'a' && c <= 'f')
				c = c - ('a' - 'A');

			res = (c - 'A') + 10;

			return res;
			};

		constexpr auto hex2f = [](const char* hex) -> float {
			float out = 0;

			out = (out * 0x10) + char2hex(hex[0]);
			out = (out * 0x10) + char2hex(hex[1]);

			return out / static_cast<float>(0xff);
			};

		res.r = hex2f(hex + at); at += 2;
		res.g = hex2f(hex + at); at += 2;
		res.b = hex2f(hex + at); at += 2;
		if (at < len)
			res.a = hex2f(hex + at);

		return res;
	}

	
	Color& Color::operator=(const Color& other)
	{
		this->a = other.a;
		this->b = other.b;
		this->r = other.r;
		this->g = other.g;
		this->rgba = other.rgba;
		return *this;

	}

	////////////////////// Vec2

	Vec2 Lapis::Vec2::operator-() const
	{
		Vec2 res;

		res.x = -this->x;
		res.y = -this->y;

		return res;
	}
	Vec2 Vec2::operator+(const Vec2& other) const
	{
		Vec2 res;

		res.x = this->x + other.x;
		res.y = this->y + other.y;

		return res;
	}
	Vec2 Vec2::operator-(const Vec2& other) const
	{
		Vec2 res;

		res.x = this->x - other.x;
		res.y = this->y - other.y;

		return res;
	}
	Vec2 Vec2::operator*(const float& mul) const
	{
		Vec2 res;

		res.x = x * mul;
		res.y = y * mul;

		return res;
	}

	////////////////////// Vec3

	Vec3 const Vec3::right(1, 0, 0);
	Vec3 const Vec3::up(0, 1, 0);
	Vec3 const Vec3::forward(0, 0, 1);

	Vec3 Lapis::Vec3::operator-() const
	{
		Vec3 res;

		res.x = -this->x;
		res.y = -this->y;
		res.z = -this->z;

		return res;
	}

	Vec3 Vec3::operator+(const Vec3& other) const
	{
		Vec3 res;

		res.x = x + other.x;
		res.y = y + other.y;
		res.z = z + other.z;

		return res;
	}
	Vec3 Vec3::operator-(const Vec3& other) const
	{
		Vec3 res;

		res.x = x - other.x;
		res.y = y - other.y;
		res.z = z - other.z;

		return res;
	}

	Vec3 Vec3::operator*(const float& scalar) const
	{
		Vec3 res;

		res.x = x * scalar;
		res.y = y * scalar;
		res.z = z * scalar;

		return res;
	}


	Vec3 Vec3::operator/(const float& scalar) const
	{
		Vec3 res;

		res.x = x / scalar;
		res.y = y / scalar;
		res.z = z / scalar;

		return res;
	}

	Vec3& Vec3::operator+=(const Vec3& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;

		return *this;
	}

	Vec3& Vec3::operator-=(const Vec3& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;

		return *this;
	}

	float& Vec3::operator[](int index)
	{
			
		if (index == 0) return x;
		else if (index == 1) return y;
		else if (index == 2) return z;
		else throw std::out_of_range("Index out of range");
	}

	float Vec3::operator[](int index) const {
		if (index == 0) return x;
		else if (index == 1) return y;
		else if (index == 2) return z;
		else throw std::out_of_range("Index out of range");
	}



	float Vec3::Distance(Vec3 a, Vec3 b)
	{
		return sqrt(powf((b.x - a.x), 2)+powf((b.y - a.y),2)+powf((b.z-a.z),2));
	}


	float Vec3::Dot(Vec3 a, Vec3 b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	void Vec3::Normalize() {
		float length = std::sqrt(x * x + y * y + z * z);
		if (length != 0.0f) {
			x /= length;
			y /= length;
			z /= length;
		}
	}

	float Vec3::Magnitude()
	{
		return std::sqrt(powf(this->x,2)+powf(this->y,2)+powf(this->z,2));
	}

	Vec3 Vec3::Cross(Vec3 a, Vec3 b) {
		Vec3 result;
		result.x = a.y * b.z - a.z * b.y;
		result.y = a.z * b.x - a.x * b.z;
		result.z = a.x * b.y - a.y * b.x;
		return result;
	}

	////////////////////// Vec4

	Vec4 Lapis::Vec4::operator+(const Vec4& other) const
	{
		Vec4 res;

		res.x = this->x + other.x;
		res.y = this->y + other.y;
		res.z = this->z + other.z;
		res.w = this->w + other.w;

		return res;
	}

	Vec4& Vec4::operator=(const RECT& other)
	{

		x = static_cast<float>(other.left);
		y = static_cast<float>(other.top);
		z = static_cast<float>(other.right);
		w = static_cast<float>(other.bottom);

		return *this;
	}

	Vec4& Vec4::operator=(const DirectX::XMVECTOR& other)
	{
		this->x = other.m128_f32[0];
		this->y = other.m128_f32[1];
		this->z = other.m128_f32[2];
		this->w = other.m128_f32[3];

		return *this;
	}

	Vec4::operator DirectX::XMVECTOR()
	{
		DirectX::XMVECTOR res;

		res.m128_f32[0] = this->x;
		res.m128_f32[1] = this->y;
		res.m128_f32[2] = this->z;
		res.m128_f32[3] = this->w;

		return res;
	}

	////////////////////// mat4x4

	mat4x4& mat4x4::operator=(const DirectX::XMMATRIX& other)
	{
		this->r[0] = other.r[0];
		this->r[1] = other.r[1];
		this->r[2] = other.r[2];
		this->r[3] = other.r[3];

		return *this;
	}

	mat4x4::operator DirectX::XMMATRIX()
	{
		DirectX::XMMATRIX res;

		res.r[0] = this->r[0];
		res.r[1] = this->r[1];
		res.r[2] = this->r[2];
		res.r[3] = this->r[3];

		return res;
	}

	mat4x4 const mat4x4::Identity({ 1, 0, 0, 0 }, { 0,1,0,0 }, { 0,0,1,0 }, { 0,0,0,1 });

	////////////////////// mat4x4
	
	Vec3 Transform::Forward()
	{
		Vec3 res;

		res.x = cos((rot.yaw + 90) * DEG2RAD) * cos((rot.pitch) * DEG2RAD);
		res.y = sin((rot.pitch) * DEG2RAD);
		res.z = sin((rot.yaw + 90) * DEG2RAD) * cos((rot.pitch) * DEG2RAD);

		return res;
	}
	Vec3 Transform::Right()
	{
		Vec3 res;

		res.x = cos((rot.yaw) * DEG2RAD);
		res.y = 0;
		res.z = sin((rot.yaw) * DEG2RAD);
		
		return res;
	}
	Vec3 Transform::Up()
	{
		Vec3 res = 0;

		res.x = -cos((rot.yaw + 90) * DEG2RAD) * sin(rot.pitch * DEG2RAD);
		res.y = cos(rot.pitch * DEG2RAD);
		res.z = -sin((rot.yaw + 90) * DEG2RAD) * sin(rot.pitch * DEG2RAD);

		return res;
	}


	//transform

}


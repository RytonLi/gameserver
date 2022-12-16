#pragma once

template<typename T>
class ISingleton
{
public:
	static T& Ins()
	{
		static T instance;
		return instance;
	}

	ISingleton(T&&) = delete;
	ISingleton(const T&) = delete;
	void operator= (const T&) = delete;

protected:
	ISingleton() = default;
	virtual ~ISingleton() = default;
};
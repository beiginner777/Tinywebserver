#ifndef SINGLETON_H
#define SINGLETON_H

#include "global.h"

template<typename T>
class SingleTon
{
// 这里设置为 protected是让子类对象使用父类对象的构造函数
// 因为子类对象在构造的时候，需要先构造父类对象
protected:
	// 将拷贝构造函数 和 等于赋值的方法禁用，从根本上防止构造出多个对象
	SingleTon(const SingleTon<T>&) = delete;
	SingleTon& operator = (const SingleTon<T>&) = delete;
	// 构造函数设置为protected可以使子类对象正常构造
	SingleTon() = default;
	~SingleTon() = default;
	static std::shared_ptr<T> instance_;
public:
// 这里设置为 public 是让子类对象，可以使用这个getInstance()方法
	static std::shared_ptr<T> getInstance()
	{
		// c++11 新特性
		static std::once_flag s_flag;
		std::call_once(s_flag,[&]() {
		 	instance_ = std::shared_ptr<T>(new T);
		});
		return instance_;
	}
};

// 静态的类成员，需要在类外进行初始化
template<typename T>
std::shared_ptr<T> SingleTon<T>::instance_ = nullptr;

#endif
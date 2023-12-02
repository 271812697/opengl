#pragma once
namespace DWY {
//������ʵ�����
template<class T>
struct RandomAccessIterator {
	constexpr RandomAccessIterator() :
	ptr_(nullptr)
	{

	}
	explicit constexpr RandomAccessIterator(T* ptr):
	ptr_(ptr)
	{


	}
	//���ز�����
	constexpr T* operator ->()const
	{
		return ptr_;
	}
	constexpr T& operator *()const
	{
		return *ptr_;
	}
	//����++���������ǰ��++����++

	//++p
	constexpr  RandomAccessIterator<T>& operator ++() {
		++ptr_;
		return *this;
	}
	//p++
	constexpr  RandomAccessIterator<T> operator ++(int) {
		RandomAccessIterator<T> it = *this;
		++ptr_;
		return it;
	}
	//--p
	constexpr  RandomAccessIterator<T>& operator --() {
		--ptr_;
		return *this;
	}
	//p--
	constexpr  RandomAccessIterator<T> operator --(int) {
		RandomAccessIterator<T> it = *this;
		--ptr_;
		return it;
	}
	constexpr  RandomAccessIterator<T>& operator +=(int val) {
		ptr_ += val;
		return *this;
	}
	constexpr  RandomAccessIterator<T>& operator -=(int val) {
		ptr_ -= val;
		return *this;
	}
	constexpr RandomAccessIterator<T> operator +(int val)const {
		return RandomAccessIterator<T>(ptr_+val);
	}
	constexpr RandomAccessIterator<T> operator -(int val)const {
		return RandomAccessIterator<T>(ptr_ - val);
	}
	constexpr int operator -(const RandomAccessIterator& rhs)const {
		
		return (int)(ptr_ - rhs.ptr_);
	}
	constexpr bool operator ==(const RandomAccessIterator& rhs)const {

		return rhs.ptr_ == ptr_;
	}
	
	constexpr bool operator !=(const RandomAccessIterator& rhs) const { return ptr_ != rhs.ptr_; }

	
	constexpr bool operator <(const RandomAccessIterator& rhs) const { return ptr_ < rhs.ptr_; }

	
	constexpr bool operator >(const RandomAccessIterator& rhs) const { return ptr_ > rhs.ptr_; }

	
	constexpr bool operator <=(const RandomAccessIterator& rhs) const { return ptr_ <= rhs.ptr_; }

	
	constexpr bool operator >=(const RandomAccessIterator& rhs) const { return ptr_ >= rhs.ptr_; }

	//
	/// Pointer.
	T* ptr_;
};


}
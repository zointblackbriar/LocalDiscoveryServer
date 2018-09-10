#pragma once
#include <memory>
//Based on @Microsoft ThreadPool.h 

class LocalDiscoveryThreadPool
{
public:
	template <typename T> static void QueueUserWorkItem(void (T::*function)(void), T *object, ULONG flags = WT_EXECUTELONGFUNCTION)
	{
		typedef std::pair<void (T::*)(), T *> CallbackType;
		std::auto_ptr<CallbackType> p(new CallbackType(function, object));

		if (::QueueUserWorkItem(ThreadAllocate<T>, p.get(), flags))
		{
			//delete Callback
			p.release();
		}
		else
		{
			//Throw a specified error
			throw GetLastError();
		}
	}
	template <typename T> static DWORD WINAPI ThreadAllocate (PVOID context)
	{
		typedef std::pair<void(T::*)(), T *> CallbackType;

		std::auto_ptr<CallbackType> pointer(static_cast<CallbackType *>(context));
		(pointer->second->*pointer->first)();
		return 0;
	}
};

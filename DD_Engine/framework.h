#pragma once

#if defined(_WIN32) || defined(_WIN64)
#  ifdef DD_ENGINE_EXPORTS
#    define DD_ENGINE_API __declspec(dllexport)
#  else
#    define DD_ENGINE_API __declspec(dllimport)
#  endif
#else
#  define DD_ENGINE_API
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 
#endif

#include "resource.h"
#include <windows.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <queue>

#include <wrl.h>
#include <shellapi.h>
#include <memory>
#include <chrono>

class ISystem
{
public:
	virtual void Initialize() = 0;
	virtual void Tick(float deltaTime) = 0;
	virtual void Finalize() = 0;
public:
	virtual ~ISystem() {}
};

template<typename T>
class ISingleton : public ISystem
{
public:
	static T* GetInstance()
	{
		static T instance;
		return &instance;
	}
public:
	ISingleton() {}
	virtual ~ISingleton() {}
};

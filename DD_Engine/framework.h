#pragma once

#if defined(_WIN32) || defined(_WIN64)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <memory>
#include <vector>
#include <chrono>
#include <iostream>
#include <cstdint>

#if defined(_WIN32) || defined(_WIN64)
#  ifdef DD_ENGINE_EXPORTS
#    define DD_ENGINE_API __declspec(dllexport)
#  else
#    define DD_ENGINE_API __declspec(dllimport)
#  endif
#else
#  define DD_ENGINE_API
#endif

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

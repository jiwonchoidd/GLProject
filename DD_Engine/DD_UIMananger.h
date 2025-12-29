#pragma once
#include "framework.h"
class DD_UIMananger : public ISingleton<DD_UIMananger>
{
public:
    virtual void Initialize() override;
    virtual void Tick(float deltaTime) override;
    virtual void Finalize() override;
public:
    void PreRender();
    void PostRender();
};

#define gUIMng (*DD_UIMananger::GetInstance())


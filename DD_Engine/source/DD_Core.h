#pragma once
#include "DD_Application.h"
#include "framework.h"

class DD_World;
class DD_CameraController;

class DD_ENGINE_API DD_Core : public AppBase
{
protected:
    virtual void OnInit() override;
    virtual void OnUpdate() override;
    virtual void OnRender() override;
    virtual void OnDestroy() override;
    virtual void OnResize(int width, int height) override;    
    virtual void OnTouchStart(int x, int y) override;
    virtual void OnTouchEnd(int x, int y) override;

public:
    DD_Core(int width, int height, const char* name);
    virtual ~DD_Core();

private:
    void SetupInputCallbacks();

private:
    std::vector<ISystem*> systems;
    DD_World* m_world;
    DD_CameraController* m_camController = nullptr;
    float m_deltaTime = 0.016f;
};

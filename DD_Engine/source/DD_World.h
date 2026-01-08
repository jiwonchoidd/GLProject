#pragma once
#include "DD_SimpleBox.h"
#include "DD_Camera.h"
#include <vector>

class DD_World
{
public:
    DD_World();
    ~DD_World();

    void Init(int width, int height);
    void Update(float deltaTime);
    void Render();

private:
    std::vector<DD_SimpleBox*> m_boxes;
    DD_Camera* m_camera;
};


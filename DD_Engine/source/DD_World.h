#pragma once
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
    std::vector<class DD_SimpleBox*> m_boxes;
    class DD_Camera* m_camera;
};


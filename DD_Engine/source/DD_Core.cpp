#include "DD_Core.h"
#include "DD_GLDevice.h"
#include "DD_Application.h"
#include "DD_SimpleBox.h"
#include "DD_World.h"
#include "DD_CameraController.h"

#ifdef _WIN32
#include <Windows.h>
#endif

GLuint g_cbNeverChanges = 0;
GLuint g_cbChangeOnResize = 0;

// DD_Engine ----------------------------------------------------------------------------

void DD_Core::OnInit()
{
	printf("DD_Core::OnInit()\n");

	systems.push_back(DD_GLDevice::GetInstance());

	for (const auto& system : systems)
	{
		system->Initialize();
	}

	if (!gGLDevice.CreateDevice(m_width, m_height))
	{
		printf("Failed to create GL device!\n");
		return;
	}

	m_world->Init(m_width, m_height);

	// create camera controller
	if (m_camController) delete m_camController;
	m_camController = new DD_CameraController(m_world->GetCamera());
}

void DD_Core::OnUpdate()
{
	using clock = std::chrono::steady_clock;

	static double elapsedTime = 0;
	static auto prevTime = clock::now();
	auto currentTime = clock::now();

	double deltaTime = std::chrono::duration<double>(currentTime - prevTime).count();
	prevTime = currentTime;
	elapsedTime += deltaTime;

	m_world->Update(static_cast<float>(deltaTime));

	for (const auto& system : systems)
	{
		system->Tick(static_cast<float>(deltaTime));
	}
}

void DD_Core::OnRender()
{
	gGLDevice.PreRender();

	m_world->Render();

	gGLDevice.PostRender();
}

void DD_Core::OnDestroy()
{
	printf("DD_Core::OnDestroy()\n");

	if (m_camController) { delete m_camController; m_camController = nullptr; }

	for (const auto& system : systems)
	{
		system->Finalize();
	}
}

void DD_Core::OnResize(int width, int height)
{
	m_width = width;
	m_height = height;

	gGLDevice.Resize(width, height);

	printf("Resized to %dx%d\n", width, height);
}

void DD_Core::OnTouchStart(int x, int y)
{
#ifdef _WIN32
	OutputDebugStringA("Touch Start\n");
#else
	printf("Touch Start\n");
#endif
	if (m_camController) m_camController->OnPointerDown(x, y);
}

void DD_Core::OnTouchEnd(int x, int y)
{
#ifdef _WIN32
	OutputDebugStringA("Touch End\n");
#else
	printf("Touch End\n");
#endif
	if (m_camController) m_camController->OnPointerUp(x, y);
}

void DD_Core::OnPointerMove(int x, int y)
{
	if (m_camController) m_camController->OnPointerMove(x, y);
}

void DD_Core::OnScroll(float delta)
{
	if (m_camController) m_camController->OnScroll(delta);
}

DD_Core::DD_Core(int width, int height, const char* name)
	: AppBase(width, height, name)
	, m_world(new DD_World())
{
}

DD_Core::~DD_Core()
{
	delete m_world;
}
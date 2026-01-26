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

	SetupInputCallbacks();
}

void DD_Core::SetupInputCallbacks()
{
	// Keyboard callback
	AddKeyCallback([this](int key, int action) {
		if (!m_camController) return;
		if (action == GLFW_PRESS)
			m_camController->OnKeyDown(key);
		else if (action == GLFW_RELEASE)
			m_camController->OnKeyUp(key);
	});

	// Mouse move callback
	AddMouseMoveCallback([this](int x, int y) {
		if (m_camController) m_camController->OnPointerMove(x, y);
	});

	// Scroll callback
	AddScrollCallback([this](float delta) {
		if (m_camController) m_camController->OnScroll(delta);
	});

	// Mouse button callback
	AddMouseButtonCallback([this](int button, int action, int x, int y) {
		if (!m_camController) return;
		if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT)
		{
			if (action == GLFW_PRESS)
				m_camController->OnPointerDown(x, y);
			else if (action == GLFW_RELEASE)
				m_camController->OnPointerUp(x, y);
		}
	});
}

void DD_Core::OnUpdate()
{
	using clock = std::chrono::steady_clock;

	static auto prevTime = clock::now();
	auto currentTime = clock::now();

	m_deltaTime = static_cast<float>(std::chrono::duration<double>(currentTime - prevTime).count());
	prevTime = currentTime;

	// Update camera controller
	if (m_camController) m_camController->Update(m_deltaTime);

	m_world->Update(m_deltaTime);

	for (const auto& system : systems)
	{
		system->Tick(m_deltaTime);
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
}

void DD_Core::OnTouchEnd(int x, int y)
{
#ifdef _WIN32
	OutputDebugStringA("Touch End\n");
#else
	printf("Touch End\n");
#endif
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
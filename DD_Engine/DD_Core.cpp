#include "DD_Core.h"
#include "DD_GLDevice.h"
#include "DD_WinApplication.h"
#include "DD_SimpleBox.h"

// Global Variables
GLuint g_cbNeverChanges = 0;
GLuint g_cbChangeOnResize = 0;

Matrix4 g_View;
Matrix4 g_Projection;

std::vector<DD_SimpleBox*> g_box;

// DD_Engine ----------------------------------------------------------------------------

void DD_Core::OnInit()
{
	printf("DD_Core::OnInit()\n");

	// Initialize systems FIRST (before CreateDevice)
	m_systems.push_back(DD_GLDevice::GetInstance());
	
	for (const auto& system : m_systems)
	{
		system->Initialize();
	}

	// CreateDevice 호출 (Initialize 이후)
	if (!gGLDevice.CreateDevice(m_width, m_height))
	{
		printf("Failed to create GL device!\n");
		return;
	}

	// Setup view matrix - move camera further back
	Vec3 eye(0.0f, 30.0f, -50.0f);
	Vec3 at(0.0f, 0.0f, 0.0f);
	Vec3 up(0.0f, 1.0f, 0.0f);
	g_View = glm::lookAt(eye, at, up);

	// Initialize projection matrix
	OnResize(m_width, m_height);

	DD_SimpleBox::CachePipline();

	int32_t maxBoxCount = 100; // Reduce for testing
	printf("Creating %d boxes...\n", maxBoxCount);

	for (int32_t i = 0; i < maxBoxCount; ++i)
	{
		DD_SimpleBox* box = new DD_SimpleBox();

		int gridSize = 5; // 5x5x5 = 125, we'll use 100
		float spacing = 5.0f;
		int x = i % gridSize;
		int y = (i / gridSize) % gridSize;
		int z = i / (gridSize * gridSize);

		Vec3 pos(
			(x - gridSize / 2) * spacing,
			(y - gridSize / 2) * spacing,
			(z - gridSize / 2) * spacing
		);

		box->Create(pos);
		g_box.push_back(box);
	}

	printf("Created %zu boxes\n", g_box.size());
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

	for (const auto& box : g_box)
	{
		box->AddPos(Vec3(0.f, static_cast<float>(0.5f * deltaTime), 0.f));
		box->AddRot(Vec3(0.f, static_cast<float>(0.3f * deltaTime), 0.f));
	}

	for (const auto& system : m_systems)
	{
		system->Tick(static_cast<float>(deltaTime));
	}
}

void DD_Core::OnRender()
{
	gGLDevice.PreRender();

	// View and projection are now set inside DD_SimpleBox::Render()
	for (const auto& box : g_box)
	{
		box->Render();
	}

	gGLDevice.PostRender();
}

void DD_Core::OnDestroy()
{
	printf("DD_Core::OnDestroy()\n");

	for (auto& box : g_box)
	{
		if (!box) continue;

		delete box;
		box = nullptr;
	}
	g_box.clear();

	DD_SimpleBox::ClearPipline();

	for (const auto& system : m_systems)
	{
		system->Finalize();
	}
}

void DD_Core::OnResize(int width, int height)
{
	m_width = width;
	m_height = height;
	
	float fov = 80.f * 3.14159265358979323846f / 180.0f;
	float aspect = static_cast<float>(width) / static_cast<float>(height);
	g_Projection = glm::perspective(fov, aspect, 0.1f, 1000.0f);

	gGLDevice.Resize(width, height);

	printf("Resized to %dx%d\n", width, height);
}

DD_Core::DD_Core(int width, int height, const char* name)
	: AppBase(width, height, name)
{
}

DD_Core::~DD_Core()
{
}
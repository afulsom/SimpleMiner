#include "Game/App.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/ProfileLogScope.hpp"

App::App()
	: m_game(nullptr)
	, m_isQuitting(false)
	, m_hasFocus(false)
{
	g_theInput = new InputSystem();
	g_theAudio = new AudioSystem();
	g_theRenderer = new Renderer();
	m_game = new Game();
}

App::~App()
{
	delete m_game;
	m_game = nullptr;

	delete g_theInput;
	g_theInput = nullptr;

	delete g_theAudio;
	g_theAudio = nullptr;

	delete g_theRenderer;
	g_theRenderer = nullptr;
}

void App::BeginFrame()
{
	if (g_theInput)
		g_theInput->BeginFrame();

	if(g_theRenderer)
		g_theRenderer->BeginFrame();

	if (g_theAudio)
		g_theAudio->BeginFrame();
}

void App::EndFrame()
{
	if (g_theInput)
		g_theInput->EndFrame();
	
	if (g_theRenderer)
		g_theRenderer->EndFrame();
}

void App::RunFrame()
{
	BeginFrame();

	static double timeOfLastRunFrame = GetCurrentTimeSeconds();

	double timeNow = GetCurrentTimeSeconds();
	double deltaSeconds = timeNow - timeOfLastRunFrame;
	timeOfLastRunFrame = timeNow;

	Update((float)deltaSeconds);			
	Render();		

	EndFrame();
}


bool App::IsQuitting() const
{
	return m_isQuitting;
}

bool App::HasFocus() const
{
	return m_hasFocus;
}

void App::SetIsQuitting(bool isQuitting)
{
	m_isQuitting = isQuitting;
}

void App::RegisterKeyDown(unsigned char keyCode)
{
	g_theInput->RegisterKeyDown(keyCode);
}

void App::RegisterKeyUp(unsigned char keyCode)
{
	g_theInput->RegisterKeyUp(keyCode);
}

void App::OnGainedFocus()
{
	m_hasFocus = true;
	g_theInput->ShowMouseCursor(false);
}

void App::OnLostFocus()
{
	m_hasFocus = false;
	g_theInput->ShowMouseCursor(true);
}

void App::Update(float deltaSeconds)
{
	if (m_game)
		m_game->Update(deltaSeconds);
}


void App::Render() const
{
	if (m_game)
		m_game->Render();
}

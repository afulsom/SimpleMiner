#pragma once
#include "Game/Game.hpp"
#include "Engine/Math/Vector2.hpp"

class App
{
private:
	bool m_isQuitting;
	bool m_hasFocus;

private:
	void Update(float deltaSeconds);
	void Render() const;

public:
	Game* m_game;

	App();
	~App();
	void BeginFrame();
	void EndFrame();
	void RunFrame();
	bool IsQuitting() const;
	bool HasFocus() const;
	void SetIsQuitting(bool isQuitting);
	void RegisterKeyDown(unsigned char keyCode);
	void RegisterKeyUp(unsigned char keyCode);

	void OnGainedFocus();
	void OnLostFocus();
};
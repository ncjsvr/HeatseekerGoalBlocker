#pragma once

#include "GuiBase.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

class HeatseekerGoalBlocker: public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginSettingsWindow, public BakkesMod::Plugin::PluginWindow
{
public:
	void onLoad() override;
	
	std::string GetPluginName() override { return "Heatseeker Goal Blocker"; }
	void RenderSettings() override;

	bool isWindowOpen_ = false;
	bool isMinimized_ = false;
	std::string menuTitle_ = "HeatseekerGoalBlocker";

	virtual void OnOpen() override;
	virtual void OnClose() override;
	virtual void Render() override;
	virtual std::string GetMenuName() override;
	virtual std::string GetMenuTitle() override;
	virtual bool ShouldBlockInput() override;
	virtual bool IsActiveOverlay() override;
	virtual void SetImGuiContext(uintptr_t ctx) override;
	
private:
	void OnGameStart();
	void CheckGameMode();
	void CreateGoalBlocker();
	void checkCollision(std::string eventName);
	void SetBallCanBeScored(BallWrapper ball, bool canBeScored);
	void RenderCanvas(CanvasWrapper canvas);
	void LaunchHeatseekerPractice();
	
	struct Barrier {
		Vector center;
		Vector size;
		bool collides(Vector point) const;
		Vector getCollisionNormal(Vector point) const;
	};
	
	bool isHeatseeker = false;
	bool active = false;
	bool blockBlueGoal = true;
	bool blockOrangeGoal = true;
	bool enableScoring = false;
	Barrier barrier1;
	Barrier barrier2;
	bool isLocalMatch = false;
	bool collided = false;
	int lastCollided = 0;
	int ticksBetweenCollisions = 150;
	bool isHost = false;
	bool collisionHooked = false;
	
	bool showMenuButton = true;
	Vector2F buttonPos = {50, 400};
	Vector2F buttonSize = {250, 50};
};

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
	
	// PluginSettingsWindow interface
	std::string GetPluginName() override { return "Heatseeker Goal Blocker"; }
	void RenderSettings() override;

	// PluginWindow interface
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
	
	struct Barrier {
		Vector center;
		Vector size;
		bool collides(Vector point) const;
		Vector getCollisionNormal(Vector point) const;
	};
	
	bool isHeatseeker = false;
	bool active = false;
	bool blockBothGoals = false;
	bool blockBlueGoal = false;
	bool blockOrangeGoal = false;
	Barrier barrier1;  // Blue goal blocker
	Barrier barrier2;  // Orange goal blocker
	bool isLocalMatch = false;
	bool collided = false;
	int lastCollided = 0;
	int ticksBetweenCollisions = 150;
	bool isHost = false;
	void SetBarrierState(bool blueBlocked, bool orangeBlocked);
};

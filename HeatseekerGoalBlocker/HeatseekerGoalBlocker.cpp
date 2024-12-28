#include "pch.h"
#include "HeatseekerGoalBlocker.h"

BAKKESMOD_PLUGIN(HeatseekerGoalBlocker, "Heatseeker Goal Blocker", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void HeatseekerGoalBlocker::onLoad()
{
	_globalCvarManager = cvarManager;
	
	cvarManager->registerCvar("blocker_both_goals", "0", "Block both goals", true, true, 0, true, 1)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			blockBothGoals = cvar.getBoolValue();
			if (active) {
				CreateGoalBlocker();
			}
		});
	
	gameWrapper->HookEventPost("Function TAGame.GameEvent_Soccar_TA.InitGame",
		std::bind(&HeatseekerGoalBlocker::OnGameStart, this));
}

void HeatseekerGoalBlocker::OnGameStart()
{
	CheckGameMode();
	
	if (isHeatseeker && isLANMatch) {
				CreateGoalBlocker();
	}
}

void HeatseekerGoalBlocker::CheckGameMode()
{
	if (!gameWrapper->IsInGame()) return;

	ServerWrapper server = gameWrapper->GetCurrentGameState();
	if (server.IsNull()) return;

	GameSettingPlaylistWrapper playlist = server.GetPlaylist();
	if (!playlist.IsNull()) {
		int playlistId = playlist.GetPlaylistId();
		cvarManager->log("Playlist ID: " + std::to_string(playlistId));
		cvarManager->log("Playlist Name: " + playlist.GetTitle().ToString());
		
		isHeatseeker = (playlistId == 24);
		isLANMatch = playlist.IsLanMatch();
	}

	cvarManager->log("Current game state:");
	cvarManager->log("Heatseeker mode: " + std::to_string(isHeatseeker));
	cvarManager->log("LAN match: " + std::to_string(isLANMatch));
}

void HeatseekerGoalBlocker::CreateGoalBlocker()
{
	if (!gameWrapper->IsInGame()) return;

	barrier1.center = Vector{0, -5150 - 100, 300};
	barrier1.size = Vector{2500, 400, 1200};
	
	if (blockBothGoals) {
		barrier2.center = Vector{0, 5150 + 100, 300};
		barrier2.size = Vector{2500, 400, 1200};
	}
	
	active = true;
	gameWrapper->HookEvent("Function TAGame.Car_TA.SetVehicleInput", 
		std::bind(&HeatseekerGoalBlocker::checkCollision, this, std::placeholders::_1));
	
	cvarManager->log("Goal blocker(s) created - Blocking both goals: " + std::to_string(blockBothGoals));
}

void HeatseekerGoalBlocker::checkCollision(std::string eventName)
{
	if (!active || !gameWrapper->IsInGame()) return;

	ServerWrapper server = gameWrapper->GetCurrentGameState();
	if (server.IsNull()) return;

	BallWrapper ball = server.GetBall();
	if (ball.IsNull()) return;

	Vector ballLocation = ball.GetLocation();
	Vector ballVelocity = ball.GetVelocity();
	BallGodWrapper heatseekerBall = BallGodWrapper(ball.memory_address);

	if (barrier1.collides(ballLocation)) {
		Vector normal = barrier1.getCollisionNormal(ballLocation);
		Vector reflection = ballVelocity - normal * (2 * Vector::dot(ballVelocity, normal));
		
		ball.SetVelocity(reflection);
		ball.SetLocation(ballLocation + normal * 10.0f);
		
		heatseekerBall.SetCarHitTeamNum(0);
		heatseekerBall.OnHitTeamNumChanged();
		heatseekerBall.UpdateColor();
		heatseekerBall.SetTargetSpeed(heatseekerBall.GetTargetSpeed());
	}
	
	if (blockBothGoals && barrier2.collides(ballLocation)) {
		Vector normal = barrier2.getCollisionNormal(ballLocation);
		Vector reflection = ballVelocity - normal * (2 * Vector::dot(ballVelocity, normal));
		
		ball.SetVelocity(reflection);
		ball.SetLocation(ballLocation + normal * 10.0f);
		
		heatseekerBall.SetCarHitTeamNum(1);
		heatseekerBall.OnHitTeamNumChanged();
		heatseekerBall.UpdateColor();
		heatseekerBall.SetTargetSpeed(heatseekerBall.GetTargetSpeed());
	}
}

bool HeatseekerGoalBlocker::Barrier::collides(Vector point) const 
{
	return (point.X >= center.X - size.X/2 && point.X <= center.X + size.X/2 &&
			point.Y >= center.Y - size.Y/2 && point.Y <= center.Y + size.Y/2 &&
			point.Z >= center.Z - size.Z/2 && point.Z <= center.Z + size.Z/2);
}

Vector HeatseekerGoalBlocker::Barrier::getCollisionNormal(Vector point) const 
{
	float dx = std::abs(point.X - center.X) / (size.X/2.0f);
	float dy = std::abs(point.Y - center.Y) / (size.Y/2.0f);
	float dz = std::abs(point.Z - center.Z) / (size.Z/2.0f);
	
	if (dx > dy && dx > dz)
		return Vector{(point.X > center.X) ? 1.0f : -1.0f, 0.0f, 0.0f};
	else if (dy > dz)
		return Vector{0.0f, (point.Y > center.Y) ? 1.0f : -1.0f, 0.0f};
	else
		return Vector{0.0f, 0.0f, (point.Z > center.Z) ? 1.0f : -1.0f};
}

void HeatseekerGoalBlocker::RenderSettings() 
{
	bool blockBoth = cvarManager->getCvar("blocker_both_goals").getBoolValue();
	if (ImGui::Checkbox("Block Both Goals", &blockBoth)) {
		cvarManager->getCvar("blocker_both_goals").setValue(blockBoth);
	}
	
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("When enabled, blocks both goals instead of just blue team's goal");
	}

	if (!blockBoth) {
		ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Currently blocking: Blue team's goal only");
	}
}

void HeatseekerGoalBlocker::SetImGuiContext(uintptr_t ctx)
{
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

void HeatseekerGoalBlocker::OnOpen()
{
	isWindowOpen_ = true;
}

void HeatseekerGoalBlocker::OnClose()
{
	isWindowOpen_ = false;
}

void HeatseekerGoalBlocker::Render()
{
	if (!ImGui::Begin(menuTitle_.c_str(), &isWindowOpen_, ImGuiWindowFlags_None))
	{
		ImGui::End();
		return;
	}

	RenderSettings();

	ImGui::End();
}

std::string HeatseekerGoalBlocker::GetMenuName()
{
	return "HeatseekerGoalBlocker";
}

std::string HeatseekerGoalBlocker::GetMenuTitle()
{
	return menuTitle_;
}

bool HeatseekerGoalBlocker::ShouldBlockInput()
{
	return false;
}

bool HeatseekerGoalBlocker::IsActiveOverlay()
{
	return false;
}

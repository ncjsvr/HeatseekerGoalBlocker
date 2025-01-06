#include "pch.h"
#include "HeatseekerGoalBlocker.h"

BAKKESMOD_PLUGIN(HeatseekerGoalBlocker, "Heatseeker Goal Blocker", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void HeatseekerGoalBlocker::onLoad()
{
	_globalCvarManager = cvarManager;
	
	cvarManager->registerCvar("blocker_blue_goal", "0", "Block blue goal", true, true, 0, true, 1)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			blockBlueGoal = cvar.getBoolValue();
			if (active) {
				CreateGoalBlocker();
			}
		});
	
	cvarManager->registerCvar("blocker_orange_goal", "0", "Block orange goal", true, true, 0, true, 1)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			blockOrangeGoal = cvar.getBoolValue();
			if (active) {
				CreateGoalBlocker();
			}
		});
	
	gameWrapper->HookEventPost("Function GameEvent_Soccar_TA.WaitingForPlayers.OnBallSpawned",
		std::bind(&HeatseekerGoalBlocker::OnGameStart, this));
	
	if (!gameWrapper->IsInGame() || gameWrapper->IsInOnlineGame()) return;
	OnGameStart();
}

void HeatseekerGoalBlocker::OnGameStart()
{
	CheckGameMode();
	
	if (isHeatseeker && isLocalMatch) {
		CreateGoalBlocker();
	}
}

void HeatseekerGoalBlocker::CheckGameMode()
{
	if (!gameWrapper->IsInGame() || gameWrapper->IsInOnlineGame()) return;

	ServerWrapper server = gameWrapper->GetCurrentGameState();
	if (server.IsNull()) return;

	GameSettingPlaylistWrapper playlist = server.GetPlaylist();
	if (!playlist.IsNull()) {
		int playlistId = playlist.GetPlaylistId();
		cvarManager->log("Playlist ID: " + std::to_string(playlistId));
		cvarManager->log("Playlist Name: " + playlist.GetTitle().ToString());
		bool isLanMatch = (playlistId == 24);
		bool isExhibitionMatch = (playlistId == 8);

		BallWrapper ball = server.GetBall();
		if (ball.IsNull()) { return; }

		isHeatseeker = ball.IsGodBall();
		isLocalMatch = (isExhibitionMatch) || (isLanMatch);
		active = false;
	}

	cvarManager->log("Current game state:");
	cvarManager->log("Heatseeker mode: " + std::to_string(isHeatseeker));
	cvarManager->log("LAN match: " + std::to_string(isLocalMatch));
}

void HeatseekerGoalBlocker::CreateGoalBlocker()
{
	if (!gameWrapper->IsInGame() || gameWrapper->IsInOnlineGame()) return;

	if (blockBlueGoal) {
		barrier1.center = Vector{0, -5150 - 100, 300};
		barrier1.size = Vector{2500, 400, 1200};
	}
	
	if (blockOrangeGoal) {
		barrier2.center = Vector{0, 5150 + 100, 300};
		barrier2.size = Vector{2500, 400, 1200};
	}
	
	active = true;
	gameWrapper->HookEvent("Function TAGame.Car_TA.SetVehicleInput", 
		std::bind(&HeatseekerGoalBlocker::checkCollision, this, std::placeholders::_1));
	
	cvarManager->log("Goal blockers updated - Blue: " + std::to_string(blockBlueGoal) + 
					", Orange: " + std::to_string(blockOrangeGoal));
}

void HeatseekerGoalBlocker::checkCollision(std::string eventName)
{
	if (!active || !gameWrapper->IsInGame() || gameWrapper->IsInOnlineGame()) return;

	ServerWrapper server = gameWrapper->GetCurrentGameState();
	if (server.IsNull()) return;

	BallWrapper ball = server.GetBall();
	if (ball.IsNull()) return;

	Vector ballLocation = ball.GetLocation();
	Vector ballVelocity = ball.GetVelocity();
	bool isHeatseekerBall = ball.IsGodBall();
	BallGodWrapper heatseekerBall = NULL;
	if (isHeatseekerBall) {
		heatseekerBall = BallGodWrapper(ball.memory_address);
	}

	bool collidesBarrier1 = barrier1.collides(ballLocation);
	if (collided && !collidesBarrier1 && lastCollided > ticksBetweenCollisions) {
		collided = false;
		lastCollided = 0;
	}
	else if (!collided && collidesBarrier1 && blockBlueGoal) {
		Vector normal = barrier1.getCollisionNormal(ballLocation);
		Vector reflection = ballVelocity - normal * (2 * Vector::dot(ballVelocity, normal));
		
		float incomingSpeed = ballVelocity.magnitude();
		float reflectionMagnitude = sqrt(reflection.X * reflection.X + 
									   reflection.Y * reflection.Y + 
									   reflection.Z * reflection.Z);
		if (reflectionMagnitude > 0) {
			reflection.X = (reflection.X / reflectionMagnitude) * incomingSpeed;
			reflection.Y = (reflection.Y / reflectionMagnitude) * incomingSpeed;
			reflection.Z = (reflection.Z / reflectionMagnitude) * incomingSpeed;
		}
		
		ball.SetVelocity(reflection);
		ball.SetLocation(ballLocation + normal * 1.0f);
		
		if (isHeatseekerBall && !heatseekerBall.IsNull()) {
			heatseekerBall.SetCarHitTeamNum(0);
			heatseekerBall.OnHitTeamNumChanged();
			heatseekerBall.UpdateColor();
		}
		
		collided = true;
	}

	bool collidesBarrier2 = barrier2.collides(ballLocation);
	if (collided && !collidesBarrier2 && lastCollided > ticksBetweenCollisions) {
		collided = false;
		lastCollided = 0;
	}
	else if (!collided && collidesBarrier2 && blockOrangeGoal) {
		Vector normal = barrier2.getCollisionNormal(ballLocation);
		Vector reflection = ballVelocity - normal * (2 * Vector::dot(ballVelocity, normal));
		
		float incomingSpeed = ballVelocity.magnitude();
		float reflectionMagnitude = sqrt(reflection.X * reflection.X + 
									   reflection.Y * reflection.Y + 
									   reflection.Z * reflection.Z);
		if (reflectionMagnitude > 0) {
			reflection.X = (reflection.X / reflectionMagnitude) * incomingSpeed;
			reflection.Y = (reflection.Y / reflectionMagnitude) * incomingSpeed;
			reflection.Z = (reflection.Z / reflectionMagnitude) * incomingSpeed;
		}
		
		ball.SetVelocity(reflection);
		ball.SetLocation(ballLocation + normal * 1.0f);
		
		if (isHeatseekerBall && !heatseekerBall.IsNull()) {
			heatseekerBall.SetCarHitTeamNum(1);
			heatseekerBall.OnHitTeamNumChanged();
			heatseekerBall.UpdateColor();
		}
		
		collided = true;
	}

	if (collided) {
		lastCollided++;
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
	bool blueBlocked = cvarManager->getCvar("blocker_blue_goal").getBoolValue();
	bool orangeBlocked = cvarManager->getCvar("blocker_orange_goal").getBoolValue();
	
	if (ImGui::Checkbox("Block Blue Goal", &blueBlocked)) {
		cvarManager->getCvar("blocker_blue_goal").setValue(blueBlocked);
	}
	
	if (ImGui::Checkbox("Block Orange Goal", &orangeBlocked)) {
		cvarManager->getCvar("blocker_orange_goal").setValue(orangeBlocked);
	}
	
	if (ImGui::IsItemHovered()) {
		std::string tooltip = "Currently blocking: ";
		if (!blueBlocked && !orangeBlocked) {
			tooltip += "No goals";
		} else {
			if (blueBlocked) tooltip += "Blue goal";
			if (blueBlocked && orangeBlocked) tooltip += " and ";
			if (orangeBlocked) tooltip += "Orange goal";
		}
		ImGui::SetTooltip(tooltip.c_str());
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

#include "pch.h"
#include "HeatseekerGoalBlocker.h"

BAKKESMOD_PLUGIN(HeatseekerGoalBlocker, "Heatseeker Goal Blocker", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void HeatseekerGoalBlocker::onLoad()
{
	_globalCvarManager = cvarManager;
	
	cvarManager->registerCvar("blocker_blue_goal", "1", "Block blue goal", false, true, 0, true, 1)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			blockBlueGoal = cvar.getBoolValue();
			if (active) {
				gameWrapper->Execute([this](GameWrapper* gw) { CreateGoalBlocker(); });
			}
		});
	
	cvarManager->registerCvar("blocker_orange_goal", "1", "Block orange goal", false, true, 0, true, 1)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			blockOrangeGoal = cvar.getBoolValue();
			if (active) {
				gameWrapper->Execute([this](GameWrapper* gw) { CreateGoalBlocker(); });
			}
		});
	
	cvarManager->registerCvar("blocker_enable_scoring", "0", "Enable ball scoring", false, true, 0, true, 1)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			enableScoring = cvar.getBoolValue();
			if (active && isHeatseeker && isLocalMatch) {
				gameWrapper->Execute([this](GameWrapper* gw) {
					ServerWrapper server = gameWrapper->GetCurrentGameState();
					if (!server.IsNull()) {
						BallWrapper ball = server.GetBall();
						if (!ball.IsNull()) {
							SetBallCanBeScored(ball, enableScoring);
						}
					}
				});
			}
		});
	
	blockBlueGoal = cvarManager->getCvar("blocker_blue_goal").getBoolValue();
	blockOrangeGoal = cvarManager->getCvar("blocker_orange_goal").getBoolValue();
	enableScoring = cvarManager->getCvar("blocker_enable_scoring").getBoolValue();
	
	gameWrapper->HookEventPost("Function GameEvent_Soccar_TA.WaitingForPlayers.OnBallSpawned",
		std::bind(&HeatseekerGoalBlocker::OnGameStart, this));
	
	cvarManager->registerCvar("blocker_show_menu_button", "1", "Show menu button", false, true, 0, true, 1)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			showMenuButton = cvar.getBoolValue();
		});
	showMenuButton = cvarManager->getCvar("blocker_show_menu_button").getBoolValue();
	
	gameWrapper->RegisterDrawable([this](CanvasWrapper canvas) { RenderCanvas(canvas); });
	
	if (!gameWrapper->IsInGame() || gameWrapper->IsInOnlineGame()) return;
	OnGameStart();
}

void HeatseekerGoalBlocker::OnGameStart()
{
	CheckGameMode();
	
	if (isHeatseeker && isLocalMatch) {
		CreateGoalBlocker();
		
		if (!enableScoring) {
			ServerWrapper server = gameWrapper->GetCurrentGameState();
			if (!server.IsNull()) {
				BallWrapper ball = server.GetBall();
				if (!ball.IsNull()) {
					SetBallCanBeScored(ball, false);
				}
			}
		}
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
		bool isLanMatch = (playlistId == 24);

		BallWrapper ball = server.GetBall();
		if (ball.IsNull()) return;

		isHeatseeker = ball.IsGodBall();
		isLocalMatch = isLanMatch;
		PriWrapper pri = gameWrapper->GetPlayerController().GetPRI();
		isHost = !pri.IsNull() && pri.IsLocalPlayerPRI();
		active = false;
	}
}

void HeatseekerGoalBlocker::CreateGoalBlocker()
{
	if (!gameWrapper->IsInGame() || gameWrapper->IsInOnlineGame()) return;

	if (blockBlueGoal) {
		barrier1.center = Vector{0, -5250, 300};
		barrier1.size = Vector{2500, 400, 1200};
	} else {
		barrier1.center = Vector{0, 0, 0};
		barrier1.size = Vector{0, 0, 0};
	}
	
	if (blockOrangeGoal) {
		barrier2.center = Vector{0, 5250, 300};
		barrier2.size = Vector{2500, 400, 1200};
	} else {
		barrier2.center = Vector{0, 0, 0};
		barrier2.size = Vector{0, 0, 0};
	}
	
	active = true;
	
	if (!collisionHooked) {
		gameWrapper->HookEvent("Function TAGame.Car_TA.SetVehicleInput", 
			std::bind(&HeatseekerGoalBlocker::checkCollision, this, std::placeholders::_1));
		collisionHooked = true;
	}
}

void HeatseekerGoalBlocker::checkCollision(std::string eventName)
{
	if (!active || !gameWrapper->IsInGame() || gameWrapper->IsInOnlineGame()) return;

	ServerWrapper server = gameWrapper->GetCurrentGameState();
	if (server.IsNull()) return;

	if (!isHost) return;

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
	bool scoringEnabled = cvarManager->getCvar("blocker_enable_scoring").getBoolValue();
	
	if (ImGui::Checkbox("Block Blue Goal", &blueBlocked)) {
		cvarManager->getCvar("blocker_blue_goal").setValue(blueBlocked);
	}
	
	if (ImGui::Checkbox("Block Orange Goal", &orangeBlocked)) {
		cvarManager->getCvar("blocker_orange_goal").setValue(orangeBlocked);
	}
	
	ImGui::Separator();
	
	if (ImGui::Checkbox("Enable Scoring", &scoringEnabled)) {
		cvarManager->getCvar("blocker_enable_scoring").setValue(scoringEnabled);
	}
	ImGui::TextDisabled("Allow goals to be scored if unblocked");
	ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Tip: Disable when blocking goals to prevent glitches if you have low fps");
	
	ImGui::Separator();
	ImGui::Spacing();
	
	if (ImGui::Button("Launch Heatseeker Practice")) {
		LaunchHeatseekerPractice();
	}
	ImGui::TextDisabled("Starts LAN match: Heatseeker, DFH Stadium");
	ImGui::TextDisabled("No bots, no time limit, no score limit");
	
	ImGui::Separator();
	ImGui::Spacing();
	
	bool menuBtnEnabled = cvarManager->getCvar("blocker_show_menu_button").getBoolValue();
	if (ImGui::Checkbox("Show Main Menu Button", &menuBtnEnabled)) {
		cvarManager->getCvar("blocker_show_menu_button").setValue(menuBtnEnabled);
	}
	ImGui::TextDisabled("Shows 'Start Heatseeker Freeplay' button on main menu - no controller support.");
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

void HeatseekerGoalBlocker::SetBallCanBeScored(BallWrapper ball, bool canBeScored)
{
	if (ball.IsNull()) return;
	
	constexpr uintptr_t bCanBeScoredOffset = 0x838;
	uintptr_t ballAddr = ball.memory_address;
	uint32_t* flagsPtr = reinterpret_cast<uint32_t*>(ballAddr + bCanBeScoredOffset);
	constexpr uint32_t bCanBeScoredBit = (1 << 10);
	
	if (canBeScored) {
		*flagsPtr |= bCanBeScoredBit;
	} else {
		*flagsPtr &= ~bCanBeScoredBit;
	}
}

void HeatseekerGoalBlocker::LaunchHeatseekerPractice()
{
	gameWrapper->Execute([this](GameWrapper* gw) {
		std::string command = "open Stadium_P?Playtest?game=TAGame.GameInfo_GodBall_TA"
			"?GameTags=PlayerCount8,BotsNone,UnlimitedTime,Max100"
			"?NumPublicConnections=8?NumOpenPublicConnections=8"
			"?Lan?Listen";
		gameWrapper->ExecuteUnrealCommand(command);
	});
}

void HeatseekerGoalBlocker::RenderCanvas(CanvasWrapper canvas)
{
	if (!showMenuButton) return;
	if (gameWrapper->IsInGame() || gameWrapper->IsInOnlineGame()) return;
	
	Vector2 screenSize = canvas.GetSize();
	float scale = screenSize.X / 1920.0f;
	
	
	float btnWidth = 330.0f * scale;
	float btnHeight = 38.0f * scale;
	float btnX = 40.0f * scale;
	float btnY = 370.0f * scale;  
	
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	ScreenToClient(GetForegroundWindow(), &cursorPos);
	
	bool hovering = cursorPos.x >= btnX && cursorPos.x <= btnX + btnWidth &&
					cursorPos.y >= btnY && cursorPos.y <= btnY + btnHeight;
	

	if (hovering) {
		canvas.SetColor(LinearColor{80, 160, 220, 255}); 
	} else {
		canvas.SetColor(LinearColor{30, 30, 35, 230});  
	}
	canvas.SetPosition(Vector2F{btnX, btnY});
	canvas.FillBox(Vector2F{btnWidth, btnHeight});
	

	if (hovering) {
		canvas.SetColor(LinearColor{120, 200, 255, 100});
	} else {
		canvas.SetColor(LinearColor{60, 60, 70, 100});
	}
	canvas.SetPosition(Vector2F{btnX, btnY});
	canvas.FillBox(Vector2F{btnWidth, 3.0f * scale});
	
	
	canvas.SetColor(LinearColor{255, 255, 255, 255});
	canvas.SetPosition(Vector2F{btnX + 12.0f * scale, btnY + 11.0f * scale});
	canvas.DrawString("Start Heatseeker Freeplay", 1.5f * scale, 1.5f * scale);
	
	
	if (hovering) {
		static bool wasClicked = false;
		bool isClicked = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
		if (isClicked && !wasClicked) {
			LaunchHeatseekerPractice();
		}
		wasClicked = isClicked;
	}
}

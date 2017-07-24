#pragma warning( disable : 4477) // FString to wchar_t warnings in swprintf

#include "Main.h"
#include <algorithm>
#include <stdlib.h>
#include <time.h>
//#include <chrono>

#define KEY_READ_DELAY 300

//
// Initilizations
//

App app;
UE3Sdk sdk;
WCHAR buffer[STR_BUF_SIZE];


// Config defaults
Config::Config() {
	//countPostRenders = false;
	isMenuVisible = false;

	showXHair = false;
	showNamePlate = true;
	showHealthBar = true;
	showHitBox = true;
	showTrace = false;

	enableAim = true;
	enableEsp = true;
};

//// Color
// Color definitions
const FColor Color::BLACK  = Color::make(0, 0, 0, 255);
const FColor Color::BLUE   = Color::make(0, 0, 255, 255);
const FColor Color::GREEN  = Color::make(0, 255, 0, 255);
const FColor Color::GRAY   = Color::make(128, 128, 128, 255);
const FColor Color::ORANGE = Color::make(255, 128, 0, 255);
const FColor Color::RED    = Color::make(255, 0, 0, 255);
const FColor Color::WHITE  = Color::make(255, 255, 255, 255);
const FColor Color::YELLOW = Color::make(255, 255, 0, 255);

//
const FColor Color::make(int r, int g, int b, int a)
{
	FColor color;
	color.R = r;
	color.G = g;
	color.B = b;
	color.A = a;
	return color;
}

//// UE3Sdk
UE3Sdk::UE3Sdk()
{
	//start = std::chrono::high_resolution_clock::now();
	//renderCtr = 0;
	//lastRenderCtr = 0;
	//lastSecond = 0;

	currentWeapon = NULL;
	targetPtr = NULL;
	clearTargetFlag = false;
	acquiringTargetLock = false;
	isProjectileWeapon = false;

	// Set some weapon specs - used for predictive aiming
	// Ultimately, this data should be derived from the server
	weapons = {
			{ "Bolt Launcher",          Weapon( 3820.0f,  8000.0f, 0.5f) },
			{ "Light Spinfusor",        Weapon( 3920.0f,  8000.0f, 0.5f) },
			{ "Spinfusor",              Weapon( 3920.0f,  8000.0f, 0.5f) },
			{ "Heavy Spinfusor",        Weapon( 3920.0f,  8000.0f, 0.5f) },
			{ "Blinksfusor",            Weapon( 3920.0f,  8000.0f, 1.0f) },
			{ "Heavy Blinksfusor",      Weapon( 3920.0f,  8000.0f, 1.0f) },
			{ "Light Blinksfusor",      Weapon( 3920.0f,  8000.0f, 1.0f) },
			{ "Twinfusor",              Weapon (3920.0f,  8000.0f, 0.5f) },
			{ "Heavy Twinfusor",        Weapon( 3920.0f,  8000.0f, 0.5f) },
			{ "Light Twinfusor",        Weapon( 3920.0f,  8000.0f, 0.5f) },
			{ "Grenade Launcher",       Weapon( 2700.0f,  7000.0f, 0.2f) },
			{ "Light Grenade Launcher", Weapon( 2700.0f,  7000.0f, 0.2f) },
			{ "Arx Buster",             Weapon( 3920.0f,  8000.0f, 0.2f) },
			{ "Jackal",                 Weapon( 3820.0f,  1600.0f, 0.2f) },
			{ "Thumper",                Weapon( 3520.0f,  8000.0f, 0.5f) },
			{ "Thumper DX",             Weapon( 3520.0f,  8000.0f, 0.5f) },
			{ "Honorfusor",             Weapon( 3920.0f,  8000.0f, 0.5f) },
			{ "Nova Blaster",           Weapon( 8000.0f, 18000.0f, 0.2f) },
			{ "Flak Cannon",            Weapon( 3920.0f,  8000.0f, 0.2f) },
			{ "Assault Rifle",          Weapon(21000.0f, 21000.0f, 0.0f) },
			{ "NJ4 SMG",                Weapon(21000.0f, 21000.0f, 0.0f) },
			{ "NJ5-B SMG",              Weapon(21000.0f, 21000.0f, 0.0f) },
			{ "Light Assault Rifle",    Weapon(21000.0f, 21000.0f, 0.0f) },
			{ "X1 LMG",                 Weapon(21000.0f, 21000.0f, 0.0f) },
			{ "Chain Gun",              Weapon(18000.0f, 18000.0f, 0.0f) },
			{ "Falcon",                 Weapon(21000.0f, 21000.0f, 0.0f) },
			{ "Plasma Gun",             Weapon( 3920.0f,  8000.0f, 0.5f) },
			{ "Plasma Cannon",          Weapon( 3920.0f,  8000.0f, 0.5f) },
			{ "Throwing Knives",        Weapon(15000.0f, 15000.0f, 0.0f) },
	};
}

// Finds and applies the rotation necessary to aim at the current target
void UE3Sdk::aim()
{
	FVector loc, relativeVec, targetPredictLoc;
	FRotator rot, calcAngle;

	// Get the players view angle and location
	// TODO: Test with vehicles
	playerCtrl->Pawn->eventGetActorEyesViewPoint(&loc, &rot);

	// Attempt predicitive aim with projectiles
	if (isProjectileWeapon) {

		float timeToTarget = (target.distance / (weapons[currentWeaponName].speed));

		// Ping compensation
		//if (playerCtrl->PlayerReplicationInfo->ExactPing && playerCtrl->PlayerReplicationInfo->ExactPing > 0) {
		//	timeToTarget += playerCtrl->PlayerReplicationInfo->ExactPing;
		//}

		// Acceleration compensation
		//FVector displacement = playerCtrl->Multiply_VectorVector(target.pawn->Acceleration, target.pawn->Velocity);
		//displacement = playerCtrl->Add_VectorVector(displacement, target.pawn->Velocity);

		// Velocity compensation
		targetPredictLoc.X = target.pawn->Location.X + (target.pawn->Velocity.X * timeToTarget);
		targetPredictLoc.Y = target.pawn->Location.Y + (target.pawn->Velocity.Y * timeToTarget);
		targetPredictLoc.Z = target.pawn->Location.Z + (target.pawn->Velocity.Z * timeToTarget);
		//targetPredictLoc = target->target->Location;

		// Subtract the inheritance offset from the final position
		FVector inheritanceOffset = playerCtrl->Multiply_FloatVector(weapons[currentWeaponName].inheritance, playerCtrl->Pawn->Velocity);
		FVector inheritanceOffsetTimed = playerCtrl->Multiply_FloatVector(timeToTarget, inheritanceOffset);
		targetPredictLoc = playerCtrl->Subtract_VectorVector(targetPredictLoc, inheritanceOffsetTimed);
	}
	else {
		// Otherwise, use the target's current location
		//targetPredictLoc = target->target->Mesh->GetBoneLocation(target->target->Mesh->GetBoneName(Bone::HEAD), 0);
		targetPredictLoc = target.pawn->Location;
	}

	// Get the relative vector between localPlayer and target
	relativeVec = playerCtrl->Subtract_VectorVector(loc, targetPredictLoc);

	// Calculate the rotation 
	calcAngle.Yaw = (int)((atan2f(relativeVec.Y, -relativeVec.X) * CONST_RadToUnrRot) * -1);
	calcAngle.Pitch = (int)((atan2f(-relativeVec.Z, sqrtf(relativeVec.X*relativeVec.X + relativeVec.Y*relativeVec.Y))) * CONST_RadToUnrRot);
	calcAngle.Roll = 0;

	// Perform the rotation on the client
	playerCtrl->Pawn->ClientSetRotation(calcAngle);

	// Example code of how to shoot a weapon
	//AProjectile* Projectile = (AProjectile*)playerCtrl->Pawn->Weapon->Spawn(playerCtrl->Pawn->Weapon->WeaponProjectiles.Data[0], playerCtrl->Pawn->Weapon, FName(0), FVector(), FRotator(), nullptr, 0);
}

// Draws a bounding box around another game player
void UE3Sdk::drawBoundingBox(Player &player, Player* aimTarget)
{
	FColor color = Color::WHITE;

	if (player.hasBoundingBox) {

		if ((aimTarget && aimTarget->pawn == player.pawn) || player.isXHairWithin2DBoundingBox) {
			color = Color::RED;
		}

		// Draw 3d box
		//canvas->Draw2DLine(vec[0].X, vec[0].Y, vec[4].X, vec[4].Y, color);
		//canvas->Draw2DLine(vec[1].X, vec[1].Y, vec[7].X, vec[7].Y, color);
		//canvas->Draw2DLine(vec[2].X, vec[2].Y, vec[6].X, vec[6].Y, color);
		//canvas->Draw2DLine(vec[3].X, vec[3].Y, vec[5].X, vec[5].Y, color);
		//canvas->Draw2DLine(vec[0].X, vec[0].Y, vec[2].X, vec[2].Y, color);
		//canvas->Draw2DLine(vec[0].X, vec[0].Y, vec[3].X, vec[3].Y, color);
		//canvas->Draw2DLine(vec[7].X, vec[7].Y, vec[2].X, vec[2].Y, color);
		//canvas->Draw2DLine(vec[7].X, vec[7].Y, vec[3].X, vec[3].Y, color);
		//canvas->Draw2DLine(vec[1].X, vec[1].Y, vec[5].X, vec[5].Y, color);
		//canvas->Draw2DLine(vec[1].X, vec[1].Y, vec[6].X, vec[6].Y, color);
		//canvas->Draw2DLine(vec[6].X, vec[4].Y, vec[5].X, vec[5].Y, color);
		//canvas->Draw2DLine(vec[6].X, vec[4].Y, vec[6].X, vec[6].Y, color);

		// Draw 2d box
		canvas->Draw2DLine(player.screenLeft, player.screenTop, player.screenRight, player.screenTop, color);
		canvas->Draw2DLine(player.screenRight, player.screenTop, player.screenRight, player.screenBottom, color);
		canvas->Draw2DLine(player.screenRight, player.screenBottom, player.screenLeft, player.screenBottom, color);
		canvas->Draw2DLine(player.screenLeft, player.screenBottom, player.screenLeft, player.screenTop, color);
	}
}

// Draws a custom crosshair. Go crazy.
void UE3Sdk::drawCrossHair()
{
	canvas->Draw2DLine(canvas->ClipX / 2 - 15, canvas->ClipY / 2, canvas->ClipX / 2 + 15, canvas->ClipY / 2, Color::BLUE);
	canvas->Draw2DLine(canvas->ClipX / 2, canvas->ClipY / 2 - 15, canvas->ClipX / 2, canvas->ClipY / 2 + 15, Color::BLUE);
}

// Draws player info to screen, such as their 3D and screen coordinates
void UE3Sdk::drawPlayerInfo(std::list<Player> players, Player *aimTarget)
{
	WCHAR buffer[STR_BUF_SIZE];
	float cols[5] = { 10.0f , 175.0f , 350.0f , 525.0f };
	float lineHeight = 15.0;
	float yInc = 10.0;
	int ctr = 1;
	FColor color = Color::WHITE;

	drawText(L"Player Name", Color::WHITE, cols[0], yInc);
	drawText(L"ID", Color::WHITE, cols[1], yInc);
	drawText(L"World Loc", Color::WHITE, cols[2], yInc);
	drawText(L"Screen Loc", Color::WHITE, cols[3], yInc);
	drawText(L"Distance", Color::WHITE, cols[4], yInc);
	yInc += lineHeight;

	for (std::list<Player>::iterator player = players.begin(); player != players.end(); ++player) {

		if (aimTarget && player->pawn == aimTarget->pawn) {
			color = Color::RED;
		}
		else if (player->isOnScreen) {
			color = Color::GREEN;
		}
		else {
			color = Color::WHITE;
		}

		swprintf(buffer, STR_BUF_SIZE, L"%d %s:", ctr, player->pawn->PlayerReplicationInfo->PlayerName);
		drawText(buffer, color, cols[0], yInc);

		swprintf(buffer, STR_BUF_SIZE, L"%0.0lf, %0.0lf, %0.0lf", player->pawn->Location.X, player->pawn->Location.Y, player->pawn->Location.Z);
		drawText(buffer, color, cols[1], yInc);

		swprintf(buffer, STR_BUF_SIZE, L"%0.0lf, %0.0lf", player->screenLoc.X, player->screenLoc.Y);
		drawText(buffer, color, cols[2], yInc);

		swprintf(buffer, STR_BUF_SIZE, L"%0.0lf", player->distance);
		drawText(buffer, color, cols[3], yInc);

		yInc += lineHeight;
	}

	yInc += lineHeight;

	drawText(L"Player", Color::WHITE, cols[0], yInc);
	drawText(L"World Loc", Color::WHITE, cols[1], yInc);
	drawText(L"Acceleration", Color::WHITE, cols[2], yInc);
	drawText(L"View Angle", Color::WHITE, cols[3], yInc);

	yInc += lineHeight;

	// Player name
	swprintf(buffer, STR_BUF_SIZE, L"%s:", playerCtrl->Pawn->PlayerReplicationInfo->PlayerName);
	drawText(buffer, Color::WHITE, cols[0], yInc);

	FVector pLoc;
	FRotator pRot;
	playerCtrl->Pawn->eventGetActorEyesViewPoint(&pLoc, &pRot);
	// Doesn't work in PostRender
	//pLoc = playerCtrl->PlayerCamera->CameraCache.POV.Location;
	//pRot = playerCtrl->PlayerCamera->CameraCache.POV.Rotation;

	// Player location
	swprintf(buffer, STR_BUF_SIZE, L"%0.0lf, %0.0lf, %0.0lf", pLoc.X, pLoc.Y, pLoc.Z);
	drawText(buffer, Color::WHITE, cols[1], yInc);

	// Player acceleration
	swprintf(buffer, STR_BUF_SIZE, L"%0.02f, %0.02f, %0.02f", playerCtrl->Pawn->Acceleration.X, playerCtrl->Pawn->Acceleration.Y, playerCtrl->Pawn->Acceleration.Z);
	drawText(buffer, Color::WHITE, cols[2], yInc);

	// Viewing angle
	swprintf(buffer, STR_BUF_SIZE, L"%d, %d, %d", pRot.Pitch % 360, pRot.Roll, pRot.Yaw % 360);
	drawText(buffer, Color::WHITE, cols[3], yInc);
}

// Draws target info to screen
void UE3Sdk::drawTargetInfo(Player *aimTarget)
{
	//
	float xCur = 10.0f;

	swprintf(buffer, STR_BUF_SIZE, L"Position: %0.0lf %0.0lf %0.0lf", aimTarget->pawn->Location.X, aimTarget->pawn->Location.Y, aimTarget->pawn->Location.Z);
	drawText(buffer, Color::WHITE, xCur, 25.0f);

	swprintf(buffer, STR_BUF_SIZE, L"Velocity: %0.02f %0.02f %0.02f", aimTarget->pawn->Velocity.X, aimTarget->pawn->Velocity.Y, aimTarget->pawn->Velocity.Z);
	drawText(buffer, Color::WHITE, xCur, 40.0f);

	swprintf(buffer, STR_BUF_SIZE, L"Acceleration: %0.02f %0.02f %0.02f", aimTarget->pawn->Acceleration.X, aimTarget->pawn->Acceleration.Y, aimTarget->pawn->Acceleration.Z);
	drawText(buffer, Color::WHITE, xCur, 55.0f);

	swprintf(buffer, STR_BUF_SIZE, L"Distance: %0.0lf", aimTarget->distance);
	drawText(buffer, Color::WHITE, xCur, 70.0f);

	swprintf(buffer, STR_BUF_SIZE, L"Ping: %f", playerCtrl->PlayerReplicationInfo->ExactPing);
	drawText(buffer, Color::WHITE, xCur, 85.0f);

	swprintf(buffer, STR_BUF_SIZE, L"Weapon: %s", playerCtrl->Pawn->Weapon->ItemName);
	drawText(buffer, Color::WHITE, xCur, 100.0f);

	if (isProjectileWeapon) {
		swprintf(buffer, STR_BUF_SIZE, L"Speed: %0.01f", weapons[currentWeaponName].speed);
		drawText(buffer, Color::WHITE, xCur, 115.0f);

		swprintf(buffer, STR_BUF_SIZE, L"Max Speed: %0.01f", weapons[currentWeaponName].maxSpeed);
		drawText(buffer, Color::WHITE, xCur, 130.0f);

		swprintf(buffer, STR_BUF_SIZE, L"Inheritance: %0.02f", weapons[currentWeaponName].inheritance);
		drawText(buffer, Color::WHITE, xCur, 145.0f);
	}
}

// Draws a rectangle of given position, size and color
void UE3Sdk::drawRect(float x, float y, float width, float height, FColor color)
{
	canvas->CurX = x;
	canvas->CurY = y;
	canvas->DrawColor = color;
	canvas->DrawRect(width, height, canvas->DefaultTexture);
}

// Draw shadowed text
void UE3Sdk::drawShadowedText(FString Text, FColor color, float X, float Y, float ScaleX, float ScaleY)
{
	drawText(Text, Color::BLACK, X + 0.1f, Y, ScaleX, ScaleY);
	drawText(Text, Color::BLACK, X - 0.2f, Y, ScaleX, ScaleY);
	drawText(Text, Color::BLACK, X, Y + 0.1f, ScaleX, ScaleY);
	drawText(Text, Color::BLACK, X, Y + 0.2f, ScaleX, ScaleY);
	drawText(Text, color, X, Y, ScaleX, ScaleY);
}

// Draw normal text
void UE3Sdk::drawText(FString text, FColor color, float X, float Y, float scaleX, float scaleY)
{
	canvas->CurX = X;
	canvas->CurY = Y;
	canvas->DrawColor = color;
	canvas->DrawText(text, false, scaleX, scaleY, NULL);
}

// Shows given player on screen, regardless if they are in FOV or not
void UE3Sdk::esp(Player &player)
{
	if (isEnemy(playerCtrl->Pawn, player.pawn)) {

		// Name plate
		if (cfg.showNamePlate) {
			drawShadowedText(player.pawn->PlayerReplicationInfo->PlayerName, Color::RED, player.screenLoc.X - (textWidth(player.pawn->PlayerReplicationInfo->PlayerName) / 2), player.screenTop - 25.0f);
		}

		// Trace line to player
		if (cfg.showTrace) {
			drawRect(player.screenLoc.X - 1.0f, player.screenLoc.Y - 1.0f, 3.0f, 3.0f, Color::RED);
			canvas->Draw2DLine(canvas->ClipX / 2, canvas->ClipY, player.screenLoc.X, player.screenLoc.Y, Color::GRAY);
		}

		// Health bar
		if (cfg.showHealthBar) {
			drawRect(player.screenLoc.X - 50.f, player.screenTop - 10.0f, 100.0f * player.pawn->Health / player.pawn->HealthMax, 5.0f, Color::RED);
		}

		// Hit box
		if (cfg.showHitBox) {
			drawBoundingBox(player, targetPtr);
		}
	}
}

//
void UE3Sdk::getAxes(FRotator R, FVector &X, FVector &Y, FVector &Z)
{
	FRotator R2;

	X = rotationToVector(R);
	normalize(X);

	R.Yaw += 16384;
	R2 = R;
	R2.Pitch = 0;
	Y = rotationToVector(R2);
	normalize(Y);

	Y.Z = 0.f;
	R.Yaw -= 16384;
	R.Pitch += 16384;
	Z = rotationToVector(R);
	normalize(Z);
}

// Find the object of the given class
UObject* UE3Sdk::getInstanceOf(UClass* uClass)
{
	UObject* ObjectInstance = NULL;

	for (int i = 0; i < UObject::GObjObjects()->Count; ++i) {

		UObject* checkObject = (*UObject::GObjObjects())(i);

		if (checkObject && checkObject->IsA(uClass) && !strstr(checkObject->GetFullName(), "Default")) {
			ObjectInstance = checkObject;
		}
	}

	return ObjectInstance;
}

// 
bool UE3Sdk::isEnemy(APawn* a, APawn* b)
{
	return(a->PlayerReplicationInfo->Team != b->PlayerReplicationInfo->Team);
}

// 
bool UE3Sdk::isGamePlayer(APawn* pawn)
{
	return (!pawn->bDeleteMe && !pawn->bHidden && pawn->PlayerReplicationInfo
		&& pawn->PlayerReplicationInfo && pawn->PlayerReplicationInfo->Team
		&& pawn->PlayerReplicationInfo->Team != NULL);
}

// 
bool UE3Sdk::isGameStarted()
{
	return playerCtrl && playerCtrl->Pawn && playerCtrl->Pawn->WorldInfo && playerCtrl->Pawn->WorldInfo->bBegunPlay;
}

//
void UE3Sdk::normalize(FVector &v)
{
	float size = magnitude(v);

	if (!size) {
		v.X = v.Y = v.Z = 1;
	}
	else {
		v.X /= size;
		v.Y /= size;
		v.Z /= size;
	}
}

// Post render hook
void UE3Sdk::postRender(UCanvas* uCanvas)
{
	if (!engine) { return; }

	playerCtrl = engine->GamePlayers.Data[0]->Actor;
	if (!playerCtrl) { return; }

	if (!uCanvas || !isGameStarted()) { return; }
	canvas = uCanvas;

	// Draw the number of postrenders per second. This should be around 60.
	//if (cfg.countPostRenders) {
	//	auto elapsed = std::chrono::high_resolution_clock::now() - start;
	//	long long thisSecond = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
	//	if (lastSecond != thisSecond) {
	//		lastRenderCtr = renderCtr;
	//		lastSecond = thisSecond;
	//		renderCtr = 1;
	//	}
	//	else {
	//		renderCtr++;
	//	}
	//	swprintf(buffer, STR_BUF_SIZE, L"PRPS: %d", lastRenderCtr);
	//	drawText(buffer, Color::WHITE, canvas->ClipX - 350, 10);
	//}

	// Set current weapon values if the weapon changed since the last render
	if (playerCtrl->Pawn->Weapon != NULL && playerCtrl->Pawn->Weapon != currentWeapon) {
		currentWeapon = playerCtrl->Pawn->Weapon;
		wchar_t weaponNameChar[32];
		swprintf(weaponNameChar, 32, L"%s", playerCtrl->Pawn->Weapon->ItemName);
		std::wstring ws(weaponNameChar);
		std::string weaponName(ws.begin(), ws.end());
		currentWeaponName = weaponName;
		std::unordered_map<std::string, Weapon>::const_iterator weaponsIt = weapons.find(weaponName);
		isProjectileWeapon = (weaponsIt != weapons.end());
	}

	// Check if the current target has been unselected
	if (clearTargetFlag && targetPtr != NULL || playerCtrl->Pawn->Health < 1) {
		targetPtr = NULL;
		clearTargetFlag = false;
	}

	// 
	if (playerCtrl->Pawn && playerCtrl->Pawn->Health > 0 && playerCtrl->Pawn->PlayerReplicationInfo
		&& playerCtrl->Pawn->PlayerReplicationInfo->Team && playerCtrl->Pawn->WorldInfo) {

		if (cfg.showXHair) {
			drawCrossHair();
		}

		std::list<Player> players;
		APawn* pawn = (APawn*)playerCtrl->Pawn->WorldInfo->PawnList;
		bool targetExists = false;

		// Loop through list of other players, gather some data and sync the current target, if there is one
		while (pawn != NULL) {

			if (isGamePlayer(pawn) && isEnemy(playerCtrl->Pawn, pawn)) {

				Player player;
				prepPlayerData(player, pawn);
				players.push_back(player);

				// Run enhancements
				if (pawn->Health > 0) {

					if (acquiringTargetLock && player.isXHairWithin2DBoundingBox) {
						target = player;
						targetPtr = &target;
						clearTargetFlag = false;
						acquiringTargetLock = false;
						targetExists = true;
					}

					esp(player);
				}

				// Make sure our target is still in the game
				if (!targetExists && targetPtr != NULL && target.pawn == pawn) {
					targetExists = true;
					target = player;
					targetPtr = &target;
				}
			}
			
			pawn = pawn->NextPawn;
		}

		if (!targetExists) {
			targetPtr = NULL;
		}

		// Aim enhancement
		if (targetPtr != NULL) {

			drawTargetInfo(targetPtr);

			// Stop aiming at dead players
			if (target.pawn && target.pawn->Health < 1) {
				targetPtr = NULL;
			}
			else {
				aim();
			}
		}

		// Aim text
		if (acquiringTargetLock) {
			drawText(L"Target Lock: acquiring", Color::YELLOW, canvas->ClipX - 150, 10);
		}
		else if (targetPtr != NULL && target.pawn->PlayerReplicationInfo) {
			swprintf(buffer, STR_BUF_SIZE, L"Target Lock: %s", target.pawn->PlayerReplicationInfo->PlayerName);
			drawText(buffer, Color::GREEN, canvas->ClipX - 250, 10);
		}
		else {
			drawText(L"Target Lock: inactive", Color::WHITE, canvas->ClipX - 150, 10);
		}

		//drawPlayerInfo(players, targetPtr);
	}
	// Player isn't in game -> deselect the current target if there's one selected
	else if (targetPtr != NULL) {
		targetPtr = NULL;
	}
}

// Set player calculated data
void UE3Sdk::prepPlayerData(Player &player, APawn *target)
{
	FVector minVec, maxVec, vec[8];
	FBox box;

	player.pawn = target;
	player.screenLoc = worldToScreen(target->Mesh->GetBoneLocation(target->Mesh->GetBoneName(Bone::HEAD), 0));

	// Get bounding boxes
	player.pawn->GetComponentsBoundingBox(&box);

	if (box.IsValid) {
		player.hasBoundingBox = true;
		maxVec = box.Max;
		minVec = box.Min;
		vec[2] = minVec;
		vec[2].X = maxVec.X;
		vec[3] = minVec;
		vec[3].Y = maxVec.Y;
		vec[4] = minVec;
		vec[4].Z = maxVec.Z;
		vec[5] = maxVec;
		vec[5].X = minVec.X;
		vec[6] = maxVec;
		vec[6].Y = minVec.Y;
		vec[7] = maxVec;
		vec[7].Z = minVec.Z;

		vec[0] = worldToScreen(minVec);
		vec[1] = worldToScreen(maxVec);
		vec[2] = worldToScreen(vec[2]);
		vec[3] = worldToScreen(vec[3]);
		vec[4] = worldToScreen(vec[4]);
		vec[5] = worldToScreen(vec[5]);
		vec[6] = worldToScreen(vec[6]);
		vec[7] = worldToScreen(vec[7]);

		// Store the bounding box vectors, if necessary
		//for (int i = 0; i < 8; ++i) {
		//	player.boundingVecs[i] = vec[i];
		//}

		// Find bounding box extremities for 2D
		player.screenLeft = vec[1].X;
		player.screenTop = vec[1].Y;
		player.screenRight = vec[1].X;
		player.screenBottom = vec[1].Y;

		for (int i = 0; i < 8; ++i) {
			if (player.screenLeft > vec[i].X) {
				player.screenLeft = vec[i].X;
			}

			if (player.screenTop > vec[i].Y) {
				player.screenTop = vec[i].Y;
			}

			if (player.screenRight < vec[i].X) {
				player.screenRight = vec[i].X;
			}

			if (player.screenBottom < vec[i].Y) {
				player.screenBottom = vec[i].Y;
			}
		}
	}
	else {
		player.hasBoundingBox = false;
	}

	// Flag if this target is on screen or not
	player.isOnScreen = player.screenLoc.X > 0.0f && player.screenLoc.X < canvas->ClipX && player.screenLoc.Y > 0.0f && player.screenLoc.Y < canvas->ClipY;

	// Flag if localPlayer's crosshair falls within the target's 2D bounding box
	float centerX = canvas->ClipX / 2.0f;
	float centerY = canvas->ClipY / 2.0f;
	player.isXHairWithin2DBoundingBox = player.screenLeft < centerX && player.screenRight > centerX
		&& player.screenTop < centerY && player.screenBottom > centerY;

	// Get the distance from this target to localPlayer
	player.distance = magnitude(playerCtrl->Subtract_VectorVector(target->Location, playerCtrl->Pawn->Location));
}

//
FVector UE3Sdk::rotationToVector(FRotator r)
{
	FVector Vec;
	float fYaw = r.Yaw * (float)CONST_UnrRotToRad;
	float fPitch = r.Pitch * (float)CONST_UnrRotToRad;
	float CosPitch = playerCtrl->Cos(fPitch);

	Vec.X = playerCtrl->Cos(fYaw) * CosPitch;
	Vec.Y = playerCtrl->Sin(fYaw) * CosPitch;
	Vec.Z = playerCtrl->Sin(fPitch);

	return Vec;
}

// Gets the magnitude of a vector
float UE3Sdk::magnitude(FVector &v)
{
	return playerCtrl->Sqrt(v.X*v.X + v.Y*v.Y + v.Z*v.Z);
}

// Gets the height that an Fstring will occupy on a UCanvas, assuming no clipping
float UE3Sdk::textHeight(FString str)
{
	float X, Y;
	canvas->StrLen(str, &X, &Y);
	return Y;
}

// Gets the width that an Fstring will occupy on a UCanvas, assuming no clipping
float UE3Sdk::textWidth(FString str)
{
	float X, Y;
	canvas->StrLen(str, &X, &Y);
	return X;
}

// Maps object coordinates from 3D (world) to 2D (screen)
FVector UE3Sdk::worldToScreen(FVector Location)
{
	FVector Return;
	FVector AxisX, AxisY, AxisZ, Delta, Transformed, Loc;
	FRotator Rot;

	if (playerCtrl->Pawn->IsA(ASVehicle::StaticClass())) {
		ASVehicle* Vehicle = (ASVehicle*)playerCtrl->Pawn;
		char* vehicleName = Vehicle->GetFullName();

		if (!strcmp(vehicleName, "TrVehicle_GravCycle TheWorld.PersistentLevel.TrVehicle_GravCycle")
			|| !strcmp(vehicleName, "TrVehicle_Beowulf TheWorld.PersistentLevel.TrVehicle_Beowulf")) {

			float FOV;
			float DeltaTime = 0.0f;
			Vehicle->CalcCamera(DeltaTime, &Loc, &Rot, &FOV);
		}
		else if (!strcmp(vehicleName, "TrVehicle_Shrike TheWorld.PersistentLevel.TrVehicle_Shrike")) {
			playerCtrl->eventGetPlayerViewPoint(&Loc, &Rot);
		}
	}
	else {
		playerCtrl->Pawn->eventGetActorEyesViewPoint(&Loc, &Rot);
	}

	getAxes(Rot, AxisX, AxisY, AxisZ);

	Delta = playerCtrl->Subtract_VectorVector(Location, Loc);
	Transformed.X = playerCtrl->Dot_VectorVector(Delta, AxisY);
	Transformed.Y = playerCtrl->Dot_VectorVector(Delta, AxisZ);
	Transformed.Z = playerCtrl->Dot_VectorVector(Delta, AxisX);

	if (Transformed.Z < 1.00f) {
		Transformed.Z = 1.00f;
	}

	float FOVAngle = playerCtrl->FOVAngle;

	Return.X = (canvas->ClipX / 2.0f) + Transformed.X * ((canvas->ClipX / 2.0f) / playerCtrl->Tan((float)(FOVAngle * CONST_Pi) / 360.0f)) / Transformed.Z;
	Return.Y = (canvas->ClipY / 2.0f) + -Transformed.Y * ((canvas->ClipX / 2.0f) / playerCtrl->Tan((float)(FOVAngle * CONST_Pi) / 360.0f)) / Transformed.Z;
	Return.Z = 0;

	return Return;
}


// The ASM part of the hook
void __declspec(naked) hkProcessEvent()
{
    __asm mov app.pCallObject, ecx;
    __asm {
        push eax
        mov eax, dword ptr [esp + 0x8]
        mov app.pUFunc, eax
        mov eax, dword ptr [esp + 0xC]
        mov app.pParms, eax
        mov eax, dword ptr [esp + 0x10]
        mov app.pResult, eax
        pop eax        
    } 
    _asm pushad 
    
	// Custom code here

    if ( app.pUFunc ) {

		strcpy_s(app.processEventFnName, app.pUFunc->GetFullName());

		// Hook into PostRender function
        if (!strcmp(app.processEventFnName, "Function TribesGame.TrGameViewportClient.PostRender")) {
			sdk.postRender(((UGameViewportClient_eventPostRender_Parms*)app.pParms)->Canvas);  
        }
    }

	// End custom code

    __asm popad
    __asm {
        push app.pResult
        push app.pParms
        push app.pUFunc
        call app.orgProcessEvent
        retn 0xC
    } 
}

// Restore the original ProcessEvent
void unhookUObject(UObject* obj)
{
	toolkit::VMTHook* hook = new toolkit::VMTHook(obj);
	hook->HookMethod(app.orgProcessEvent, ProcessEvent_Index);
}

//
void hookUObject(UObject* obj)
{
	toolkit::VMTHook* hook = new toolkit::VMTHook(obj);
	app.orgProcessEvent = hook->GetMethod<tProcessEvent>(ProcessEvent_Index);
	hook->HookMethod(&hkProcessEvent, ProcessEvent_Index);
}

// TrGameViewportClient Hook
DWORD WINAPI OnAttach(LPVOID lpParam)
{
	sdk.engine = (UEngine*)sdk.getInstanceOf(UEngine::StaticClass());
	app.viewport = UObject::FindObject<UObject>("TrGameViewportClient Transient.TrGameEngine.TrGameViewportClient");
	// UObject* u = UObject::FindObject<UObject>("TrPlayerController TheWorld.PersistentLevel.TrPlayerController");

	// Todo: Figure out how to hook multiple objects
	hookUObject(app.viewport);

	return 1;
}

// Intercept keyboard events
DWORD WINAPI Hotkeys(LPVOID lpParam)
{
	while (true) {

		Sleep(KEY_READ_DELAY);

		if (GetAsyncKeyState(VK_LSHIFT)) {

			// If trying to acquire a target...
			if (sdk.acquiringTargetLock) {
				sdk.acquiringTargetLock = false;
				sdk.clearTargetFlag = true;
			}
			// If currently locked on a target, untarget
			else if (sdk.targetPtr != NULL) {
				sdk.acquiringTargetLock = false;
				sdk.clearTargetFlag = true;
			}
			// Try to get a target lock
			else {
				sdk.acquiringTargetLock = true;
			}
		}
	}

	return 1;
}

// Main entry point. Create a hook on the PostRender method and run it in a thread. 
// Create another thread to intercept keyboard events.
BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	srand((unsigned)time(NULL));

	if (dwReason == DLL_PROCESS_ATTACH) {
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnAttach, &hModule, 0, NULL);
		CreateThread(NULL, 0, &Hotkeys, &hModule, 0, NULL);
	}

    return TRUE;
}

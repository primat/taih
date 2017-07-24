#ifndef MAIN_H
#define MAIN_H 1

#include <Windows.h>
#include <math.h>
#include <stdio.h>
#include <list>
#include "libs\vmthooks\vmthooks.h"
#include "libs\ue3sdk\SDK.h"
#include <algorithm>
#include <unordered_map>

#include <string>


#define STR_BUF_SIZE 256


//////
// Configuration class
class Config {

public:
	//bool countPostRenders;
	bool isMenuVisible;
	bool showXHair;
	bool showNamePlate;
	bool showHealthBar;
	bool showHitBox;
	bool showTrace;
	bool enableAim;
	bool enableEsp;

	Config();
};


// Structure for caching calculated data
struct Player {
	APawn* pawn;
	FVector screenLoc;
	FVector boundingVecs[8];
	float screenLeft;
	float screenTop;
	float screenRight;
	float screenBottom;
	float distance;
	bool hasBoundingBox;
	bool isOnScreen;
	bool isXHairWithin2DBoundingBox;
};


struct Weapon {
	float speed;
	float maxSpeed;
	float inheritance;

	Weapon() {};
	Weapon(float s, float m, float i) : speed { s }, maxSpeed{ m }, inheritance{ i } {};
};

/////
// The UE3 sdk class contains utility functions over the UE3 SDK
class UE3Sdk {

public:
	// Cache these values so we don't have to continually supply them to methods.
	// Be careful though - the drawback is that we have to test them in each 
	// method we want to use them in
	UEngine*            engine;
	UCanvas*            canvas;
	APlayerController*  playerCtrl;
	Config              cfg;

	// Variables used in counting the number of postRender calls per second
	//std::chrono::time_point<std::chrono::steady_clock> start;
	//int                 renderCtr;
	//int                 lastRenderCtr;
	//long long           lastSecond;

	// Variables used in targetting
	Player              target;
	Player              *targetPtr;
	bool                clearTargetFlag;
	bool                acquiringTargetLock;
	AWeapon            *currentWeapon;
	std::string         currentWeaponName;
	bool                isProjectileWeapon;

	std::unordered_map<std::string, Weapon> weapons;

	UE3Sdk::UE3Sdk();

	// Utilities
	UObject* getInstanceOf(UClass* uClass);
	void     prepPlayerData(Player &player, APawn *target);

	// Drawing methods
	void     drawRect(float x, float y, float width, float height, FColor color);
	void     drawShadowedText(FString text, FColor color, float x, float y, float scaleX = 1.0f, float scaleY = 1.0f);
	void     drawText(FString text, FColor color, float x, float y, float scaleX = 1.0f, float scaleY = 1.0f);
	float    textHeight(FString str);
	float    textWidth(FString str);

	// Math + geometry methods
	void     getAxes(FRotator r, FVector &x, FVector &y, FVector &z);
	void     normalize(FVector &v);
	FVector  rotationToVector(FRotator r);
	float    magnitude(FVector &v);
	FVector  worldToScreen(FVector location);

	// Game logic
	bool     isEnemy(APawn* a, APawn* b);
	bool     isGamePlayer(APawn* pawn);
	bool     isGameStarted();

	// "Enhancements"
	void     aim();
	void     drawBoundingBox(Player &player, Player* aimTarget);
	void     drawCrossHair();
	void     drawPlayerInfo(std::list<Player> players, Player *aimTarget);
	void     drawTargetInfo(Player *aimTarget);
	void     esp(Player &player);

	// Hooks
	void     postRender(UCanvas* pCanvas);
};

/////
// Colors - should probably be an enum or class
class Color {

public:
	static const FColor BLACK;
	static const FColor BLUE;
	static const FColor GRAY;
	static const FColor GREEN;
	static const FColor ORANGE;
	static const FColor RED;
	static const FColor WHITE;
	static const FColor YELLOW;

	static const FColor make(int r, int g, int b, int a);
};

////
// The app class takes care of hooking into ProcessEvent and the game's viewport
class App {

public:
	//UEngine*       engine;
	tProcessEvent  orgProcessEvent = NULL;
	UObject*       viewport;
	UFunction*     pUFunc;
	void*          pParms;
	void*          pResult;
	UObject*       pCallObject;
	Config         cfg;
	UE3Sdk         sdk;
	char  processEventFnName[STR_BUF_SIZE];
};

// (Mesh) Bone indexes. Uncomment as needed
enum Bone {
	//ROOT = 0,
	//EFFECTSSOCKET = 1,
	//PELVIS = 2,
	//SPINE = 3,
	//SPINE1 = 4,
	//NECK = 5,
	//NECK1 = 6,
	HEAD = 7
	//L_CLAVICLE = 8,
	//L_SHOULDERDEFCOUNTER = 9,
	//L_UPPERARM = 10,
	//L_FOREARM = 11,
	//L_HAND = 12,
	//L_FINGER0 = 13,
	//L_FINGER01 = 14,
	//L_FINGER02 = 15,
	//PROP2 =  16,
	//R_GRIP = 17,
	//L_FINGER1 = 18,
	//L_FINGER11 = 19,
	//L_FINGER12 = 20,
	//L_FINGER2 = 21,
	//L_FINGER21 = 22,
	//L_FINGER22 = 23,
	//L_FINGER3 = 24,
	//L_FINGER31 = 25,
	//L_FINGER32 = 26,
	//L_FINGER4 = 27,
	//L_FINGER41 = 28,
	//L_FINGER42 = 29,
	//L_FOREARMTWIST01 = 30,
	//L_FOREARMTWIST02 = 31,
	//L_FOREARMTWIST03 = 32,
	//L_FOREARMTWIST04 = 33,
	//L_FOREARMTWIST05 = 34,
	//L_ELBOW = 35,
	//L_SHOULDERPAD_PT = 36,
	//L_SHOULDERPAD = 37,
	//R_CLAVICLE = 38,
	//R_SHOULDERDEFCOUNTER = 39,
	//R_UPPERARM = 40,
	//R_FOREARM = 41,
	//R_HAND = 42,
	//R_FINGER0 = 43,
	//R_FINGER01 = 44,
	//R_FINGER02 = 45,
	//PROP1 = 46,
	//L_GRIP = 47,
	//R_FINGER1 = 48,
	//R_FINGER11 = 49,
	//R_FINGER12 = 50,
	//R_FINGER2 = 51,
	//R_FINGER21 = 52,
	//R_FINGER22 = 53,
	//R_FINGER3 = 54,
	//R_FINGER31 = 55,
	//R_FINGER32 = 56,
	//R_FINGER4 = 57,
	//R_FINGER41 = 58,
	//R_FINGER42 = 59,
	//R_FOREARMTWIST01 = 60,
	//R_FOREARMTWIST02 = 61,
	//R_FOREARMTWIST03 = 62,
	//R_FOREARMTWIST04 = 63,
	//R_FOREARMTWIST05 = 64,
	//R_ELBOW = 65,
	//R_SHOULDERPAD_PT = 66,
	//WEP_1_SWITCH = 68,
	//WEP_2_SWITCH = 69,
	//PELVISWEIGHTED = 70,
	//L_THIGH = 71,
	//L_CALF = 72,
	//L_FOOT = 73,
	//L_TOE0 = 74,
	//L_KNEEPAD = 75,
	//R_THIGH = 76,
	//R_CALF = 77,
	//R_FOOT = 78,
	//R_TOE0 = 79,
	//R_KNEEPAD = 80,
	//IKGUNLEFTHAND = 81,
	//IKGUNRIGHTHAND = 82
};

#endif

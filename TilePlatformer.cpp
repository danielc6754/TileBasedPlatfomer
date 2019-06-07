// 256 px X 240 px
using namespace std;

#include <string>
#include "ConsoleEngine.h"

class Platformer : public ConsoleTemplateEngine {
public:
	Platformer() {
		m_sAppName = L"Platformer";
	}
	
private:
	// Map
	wstring sLevel;
	int nLevelWidth;
	int nLevelHeight;

	// Camera
	float fCameraPosX = 0.0f;
	float fCameraPosY = 0.0f;

	// Player Properties
	float fPlayerPosX = 1.0f;		// Position
	float fPlayerPosY = 1.0f;
	float fPlayerVelX = 0.0f;		// Velocity
	float fPlayerVelY = 0.0f;
	bool bPlayerOnGround = false;

	//TemplateSprite* spriteTiles = nullptr;

	virtual bool OnUserCreate() {
		nLevelWidth = 64;
		nLevelHeight = 16;
		sLevel += L"................................................................";
		sLevel += L"................................................................";
		sLevel += L"................................................................";
		sLevel += L"................................................................";
		sLevel += L".........................##########.............................";
		sLevel += L"........................##..............###.....................";
		sLevel += L".......................###......................................";
		sLevel += L"......................####......................................";
		sLevel += L"#########################################.##########.....#######";
		sLevel += L"........................................#.#............##.......";
		sLevel += L".........................################.#..........##.........";
		sLevel += L".........................#................#........##...........";
		sLevel += L".........................#................#......##.............";
		sLevel += L".........................#.################....##...............";
		sLevel += L".........................#...................##.................";
		sLevel += L".........................####################...................";

		return true;
	}

	virtual bool OnUserUpdate(float fElapsedTime) {
		// Utility Lambdas
		auto GetTile = [&](int x, int y) {
			if (x >= 0 && x < nLevelWidth && y >= 0 && y < nLevelHeight)
				return sLevel[y * nLevelWidth + x];
			else
				return L' ';
		};

		auto SetTile = [&](int x, int y, wchar_t c) {
			if (x >= 0 && x < nLevelWidth && y >= 0 && y < nLevelHeight)
				sLevel[y * nLevelWidth + x] = c;
		};

		// For now reset manually
		//fPlayerVelX = 0.0f;
		//fPlayerVelY = 0.0f;

		// Handle User Input
		if (IsFocused()) {
			/*  Debugging Y axis movement
			if (GetKey(VK_UP).bHeld) {
				fPlayerVelY = -6.0f;
			}
			if (GetKey(VK_DOWN).bHeld) {
				fPlayerVelY = 6.0f;
			}*/
			if (GetKey(VK_LEFT).bHeld) {
				fPlayerVelX += (bPlayerOnGround ? -25.0f : -15.0f) * fElapsedTime;
			}
			if (GetKey(VK_RIGHT).bHeld) {
				fPlayerVelX += (bPlayerOnGround ? 25.0f : 15.0f) * fElapsedTime;
			}
			if (GetKey(VK_SPACE).bHeld) {
				if (fPlayerVelY == 0) {
					fPlayerVelY = -12.0f;
				}
			}
		}

		// Gravity
		fPlayerVelY += 20.0f * fElapsedTime;

		// Drag
		if (bPlayerOnGround) {
			fPlayerVelX += -3.0f * fPlayerVelX * fElapsedTime;
			if (fabs(fPlayerVelX) < 0.01f)
				fPlayerVelX = 0.0f;
		}

		// Clamp Player velocities
		if (fPlayerVelX > 10.0f)
			fPlayerVelX = 10.0f;
		if (fPlayerVelX < -10.0f)
			fPlayerVelX = -10.0f;
		if (fPlayerVelY > 100.0f)
			fPlayerVelY = 100.0f;
		if (fPlayerVelY < -100.0f)
			fPlayerVelY = -100.0f;

		// If unhindered, new player location
		float fNewPlayerPosX = fPlayerPosX + fPlayerVelX * fElapsedTime;
		float fNewPlayerPosY = fPlayerPosY + fPlayerVelY * fElapsedTime;
		
		// Check Collision
		if (fPlayerVelX <= 0) { // Moving Left
			// fNewPlayerPosX: Only test for x collisions, resolving issues of diagonal collision 
			// Results only pushing players back in the x-axis rather than trying to figure out for both axis
			// fPlayerPosY + 0.9f: when its truncated, keeps y value from overflowing to next cell // Shrinking cell slightly to prevent false positive collisions
			if (GetTile(fNewPlayerPosX + 0.0f, fPlayerPosY + 0.0f) != L'.' || GetTile(fNewPlayerPosX + 0.0f, fPlayerPosY + 0.9f) != L'.') {
				fNewPlayerPosX = (int)fNewPlayerPosX + 1; //casting to int because truncating makes it faster than calculating the offset to get to next whole number
				fPlayerVelX = 0;
			}
		}
		else { // Moving Right
			if (GetTile(fNewPlayerPosX + 1.0f, fPlayerPosY + 0.0f) != L'.' || GetTile(fNewPlayerPosX + 1.0f, fPlayerPosY + 0.9f) != L'.') {
				fNewPlayerPosX = (int)fNewPlayerPosX;
				fPlayerVelX = 0;
			}
		}
		
		bPlayerOnGround = false;
		if (fPlayerVelY <= 0) { // Moving Up
			if (GetTile(fNewPlayerPosX + 0.0f, fNewPlayerPosY) != L'.' || GetTile(fNewPlayerPosX + 0.9f, fNewPlayerPosY) != L'.') {
				fNewPlayerPosY = (int)fNewPlayerPosY + 1;
				fPlayerVelY = 0;
			}
		}
		else { // Moving Down
			if (GetTile(fNewPlayerPosX + 0.0f, fNewPlayerPosY + 1.0f) != L'.' || GetTile(fNewPlayerPosX + 0.9f, fNewPlayerPosY + 1.0f) != L'.')
			{
				fNewPlayerPosY = (int)fNewPlayerPosY;
				fPlayerVelY = 0;
				bPlayerOnGround = true;
			}
		}

		// Update players position
		fPlayerPosX = fNewPlayerPosX;
		fPlayerPosY = fNewPlayerPosY;
		
		fCameraPosX = fPlayerPosX;
		fCameraPosY = fPlayerPosY;


		//Draw Level
		int nTileWidth = 16;
		int nTileHeight = 16;
		int nVisibleTilesX = ScreenWidth() / nTileWidth;
		int nVisibleTilesY = ScreenHeight() / nTileHeight;

		// Calculate Top-Leftmost visible tile // Camera needs to represent middle of screen
		float fOffsetX = fCameraPosX - (float)nVisibleTilesX / 2.0f;
		float fOffsetY = fCameraPosY - (float)nVisibleTilesY / 2.0f;

		// Clamp camera to game boundaries
		if (fOffsetX < 0)
			fOffsetX = 0;
		if (fOffsetY < 0)
			fOffsetY = 0;
		if (fOffsetX > nLevelWidth - nVisibleTilesX)
			fOffsetX = nLevelWidth - nVisibleTilesX;
		if (fOffsetY > nLevelHeight - nVisibleTilesY)
			fOffsetY = nLevelHeight - nVisibleTilesY;

		// Get offsets for smooth movement
		float fTileOffsetX = (fOffsetX - (int)fOffsetX) * nTileWidth; // fOffsetX - (int)fOffset gives us the fractional amount that should be moved
		float fTileOffsetY = (fOffsetY - (int)fOffsetY) * nTileHeight;

		// Draw visible tile map
		for (int x = -1; x < nVisibleTilesX + 1; x++) {
			for (int y = -1; y < nVisibleTilesY + 1; y++) {
				wchar_t sTileID = GetTile(x + fOffsetX, y + fOffsetY);
				switch (sTileID) {
				case L'.':
					Fill(x * nTileWidth - fTileOffsetX, y * nTileHeight - fTileOffsetY, (x + 1) * nTileWidth - fTileOffsetX, (y + 1) * nTileHeight - fTileOffsetY, PIXEL_SOLID, FG_CYAN);
					break;
				case L'#':
					Fill(x * nTileWidth - fTileOffsetX, y * nTileHeight - fTileOffsetY, (x + 1) * nTileWidth - fTileOffsetX, (y + 1) * nTileHeight - fTileOffsetY, PIXEL_SOLID, FG_DARK_GREEN);
					break;
				default:
					break;
				}
			
			}
		}

		// Draw Player
		Fill((fPlayerPosX - fOffsetX) * nTileWidth, (fPlayerPosY - fOffsetY) * nTileWidth, (fPlayerPosX - fOffsetX + 1.0f) * nTileWidth, (fPlayerPosY - fOffsetY + 1.0f) * nTileHeight, PIXEL_SOLID, FG_RED);

		return true;
	}
};

int main() {
	Platformer game;
	//  Original Nintendo Console
	if (game.ConstructConsole(256, 240, 4, 4))
		game.Start();

	return 0;
}
#pragma once
#ifndef ES_APP_LAUNCHERS_GAME_LAUNCHER_FACTORY_H
#define ES_APP_LAUNCHERS_GAME_LAUNCHER_FACTORY_H

#include "launchers/IGameLauncher.h"
#include <memory>

namespace OpenConsole
{
	// Factory for creating game launchers
	class GameLauncherFactory
	{
	public:
		// Create appropriate launcher for game type
		static std::shared_ptr<IGameLauncher> createLauncher(GameType type);

		// Auto-detect game type from file path
		static GameType detectGameType(const std::string& path);

		// Check if file extension matches game type
		static bool isAppImage(const std::string& path);
		static bool isShellScript(const std::string& path);
		static bool isDebPackage(const std::string& path);
		static bool isDirectory(const std::string& path);

		// Check if directory contains Ren'Py game
		static bool isRenpyGame(const std::string& dirPath);

		// Check if directory contains Electron game
		static bool isElectronGame(const std::string& dirPath);
	};

} // namespace OpenConsole

#endif // ES_APP_LAUNCHERS_GAME_LAUNCHER_FACTORY_H

#include "launchers/GameLauncherFactory.h"
#include "launchers/AppImageLauncher.h"
#include "launchers/RenpyLauncher.h"
#include "launchers/DebLauncher.h"
#include "Log.h"
#include "utils/FileSystemUtil.h"
#include <algorithm>

namespace OpenConsole
{

std::shared_ptr<IGameLauncher> GameLauncherFactory::createLauncher(GameType type)
{
	switch (type)
	{
		case GameType::APPIMAGE:
			return std::make_shared<AppImageLauncher>();

		case GameType::RENPY:
			return std::make_shared<RenpyLauncher>();

		case GameType::DEB:
			return std::make_shared<DebLauncher>();

		case GameType::ELECTRON:
			// TODO: Implement ElectronLauncher
			LOG(LogWarning) << "Electron launcher not yet implemented";
			return nullptr;

		case GameType::DIRECTORY:
			// For generic directories, try to detect specific type
			LOG(LogWarning) << "Generic directory launcher not yet implemented";
			return nullptr;

		default:
			LOG(LogError) << "Unknown game type";
			return nullptr;
	}
}

GameType GameLauncherFactory::detectGameType(const std::string& path)
{
	// Check if path exists
	if (!Utils::FileSystem::exists(path))
	{
		LOG(LogWarning) << "Path does not exist: " << path;
		return GameType::UNKNOWN;
	}

	// Check if it's a directory
	if (Utils::FileSystem::isDirectory(path))
	{
		// Check for Ren'Py game
		if (isRenpyGame(path))
			return GameType::RENPY;

		// Check for Electron game
		if (isElectronGame(path))
			return GameType::ELECTRON;

		return GameType::DIRECTORY;
	}

	// Check file extensions
	if (isAppImage(path))
		return GameType::APPIMAGE;

	if (isDebPackage(path))
		return GameType::DEB;

	if (isShellScript(path))
	{
		// Check if it's a Ren'Py launcher script
		std::string dirPath = Utils::FileSystem::getParent(path);
		if (isRenpyGame(dirPath))
			return GameType::RENPY;

		return GameType::DIRECTORY;
	}

	return GameType::UNKNOWN;
}

bool GameLauncherFactory::isAppImage(const std::string& path)
{
	std::string extension = Utils::FileSystem::getExtension(path);
	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
	return extension == ".appimage";
}

bool GameLauncherFactory::isShellScript(const std::string& path)
{
	std::string extension = Utils::FileSystem::getExtension(path);
	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
	return extension == ".sh";
}

bool GameLauncherFactory::isDebPackage(const std::string& path)
{
	std::string extension = Utils::FileSystem::getExtension(path);
	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
	return extension == ".deb";
}

bool GameLauncherFactory::isDirectory(const std::string& path)
{
	return Utils::FileSystem::isDirectory(path);
}

bool GameLauncherFactory::isRenpyGame(const std::string& dirPath)
{
	if (!Utils::FileSystem::isDirectory(dirPath))
		return false;

	// Check for renpy/ subdirectory
	std::string renpyDir = Utils::FileSystem::combine(dirPath, "renpy");
	if (Utils::FileSystem::isDirectory(renpyDir))
		return true;

	// Check for common Ren'Py files
	std::string gameDir = Utils::FileSystem::combine(dirPath, "game");
	if (Utils::FileSystem::isDirectory(gameDir))
	{
		// Look for script.rpy or script.rpyc
		std::string scriptRpy = Utils::FileSystem::combine(gameDir, "script.rpy");
		std::string scriptRpyc = Utils::FileSystem::combine(gameDir, "script.rpyc");

		if (Utils::FileSystem::exists(scriptRpy) || Utils::FileSystem::exists(scriptRpyc))
			return true;
	}

	return false;
}

bool GameLauncherFactory::isElectronGame(const std::string& dirPath)
{
	if (!Utils::FileSystem::isDirectory(dirPath))
		return false;

	// Check for common Electron indicators
	// - package.json file
	// - resources/app.asar
	// - node_modules directory

	std::string packageJson = Utils::FileSystem::combine(dirPath, "package.json");
	if (Utils::FileSystem::exists(packageJson))
		return true;

	std::string resourcesDir = Utils::FileSystem::combine(dirPath, "resources");
	if (Utils::FileSystem::isDirectory(resourcesDir))
	{
		std::string appAsar = Utils::FileSystem::combine(resourcesDir, "app.asar");
		if (Utils::FileSystem::exists(appAsar))
			return true;
	}

	return false;
}

} // namespace OpenConsole

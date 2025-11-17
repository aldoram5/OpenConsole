#include "plugins/LocalFilesystemPlugin.h"
#include "launchers/GameLauncherFactory.h"
#include "utils/FileSystemUtil.h"
#include "Log.h"
#include <algorithm>
#include <regex>

namespace OpenConsole
{

LocalFilesystemPlugin::LocalFilesystemPlugin()
	: mRecursiveScan(true)
	, mMaxScanDepth(5)
	, mInitialized(false)
{
	// Initialize supported extensions
	mSupportedExtensions.insert(".appimage");
	mSupportedExtensions.insert(".sh");
	mSupportedExtensions.insert(".deb");
}

LocalFilesystemPlugin::~LocalFilesystemPlugin()
{
	shutdown();
}

bool LocalFilesystemPlugin::initialize()
{
	if (mInitialized)
	{
		LOG(LogWarning) << "LocalFilesystemPlugin already initialized";
		return true;
	}

	LOG(LogInfo) << "Initializing LocalFilesystemPlugin...";

	// Add default scan paths
	std::string homeDir = Utils::FileSystem::getHomePath();

	// Default game directories
	addScanPath(homeDir + "/Games");
	addScanPath(homeDir + "/.local/share/games");
	addScanPath("/usr/local/games");

	// Check for USB/SD card mount points
	addScanPath("/media/usb0/Games");
	addScanPath("/media/games");

	mInitialized = true;
	LOG(LogInfo) << "LocalFilesystemPlugin initialized with " << mScanPaths.size() << " scan paths";
	return true;
}

void LocalFilesystemPlugin::shutdown()
{
	if (!mInitialized)
		return;

	LOG(LogInfo) << "Shutting down LocalFilesystemPlugin";
	mScannedPaths.clear();
	mInitialized = false;
}

AuthResult LocalFilesystemPlugin::authenticate()
{
	AuthResult result;
	result.success = true;
	result.userId = "local";
	result.userName = "Local User";
	return result;
}

std::vector<GameMetadata> LocalFilesystemPlugin::fetchGames()
{
	std::vector<GameMetadata> allGames;

	if (!mInitialized)
	{
		LOG(LogError) << "LocalFilesystemPlugin not initialized";
		mLastError = "Plugin not initialized";
		return allGames;
	}

	LOG(LogInfo) << "Scanning for local games...";

	// Clear scanned paths to allow re-scanning
	mScannedPaths.clear();

	// Scan all configured paths
	for (const auto& path : mScanPaths)
	{
		if (!Utils::FileSystem::exists(path))
		{
			LOG(LogDebug) << "Scan path does not exist: " << path;
			continue;
		}

		LOG(LogInfo) << "Scanning path: " << path;
		scanDirectoryRecursive(path, allGames, 0);
	}

	LOG(LogInfo) << "Found " << allGames.size() << " local games";
	return allGames;
}

bool LocalFilesystemPlugin::downloadGame(const GameMetadata& game,
	const std::string& installPath,
	DownloadProgressCallback progressCallback)
{
	// Local games don't need downloading, they're already on the filesystem
	mLastError = "Local games do not support downloading";
	return false;
}

bool LocalFilesystemPlugin::canHandleGameType(GameType type) const
{
	return type == GameType::APPIMAGE ||
	       type == GameType::RENPY ||
	       type == GameType::DEB ||
	       type == GameType::DIRECTORY;
}

std::vector<GameType> LocalFilesystemPlugin::getSupportedGameTypes() const
{
	return {
		GameType::APPIMAGE,
		GameType::RENPY,
		GameType::DEB,
		GameType::DIRECTORY
	};
}

void LocalFilesystemPlugin::addScanPath(const std::string& path)
{
	// Check if path already exists
	auto it = std::find(mScanPaths.begin(), mScanPaths.end(), path);
	if (it != mScanPaths.end())
		return;

	mScanPaths.push_back(path);
	LOG(LogDebug) << "Added scan path: " << path;
}

void LocalFilesystemPlugin::removeScanPath(const std::string& path)
{
	auto it = std::find(mScanPaths.begin(), mScanPaths.end(), path);
	if (it != mScanPaths.end())
	{
		mScanPaths.erase(it);
		LOG(LogDebug) << "Removed scan path: " << path;
	}
}

void LocalFilesystemPlugin::clearScanPaths()
{
	mScanPaths.clear();
	LOG(LogDebug) << "Cleared all scan paths";
}

std::vector<std::string> LocalFilesystemPlugin::getScanPaths() const
{
	return mScanPaths;
}

std::vector<GameMetadata> LocalFilesystemPlugin::scanDirectory(const std::string& path)
{
	std::vector<GameMetadata> games;

	if (!Utils::FileSystem::exists(path))
	{
		LOG(LogWarning) << "Directory does not exist: " << path;
		return games;
	}

	scanDirectoryRecursive(path, games, 0);
	return games;
}

void LocalFilesystemPlugin::scanDirectoryRecursive(const std::string& path,
	std::vector<GameMetadata>& games,
	int currentDepth)
{
	// Check depth limit
	if (mMaxScanDepth > 0 && currentDepth >= mMaxScanDepth)
	{
		LOG(LogDebug) << "Max scan depth reached at: " << path;
		return;
	}

	// Check if already scanned
	if (isPathScanned(path))
	{
		LOG(LogDebug) << "Path already scanned: " << path;
		return;
	}

	// Mark as scanned
	mScannedPaths.insert(path);

	// Get directory contents
	std::vector<std::string> contents;
	try
	{
		contents = Utils::FileSystem::getDirContent(path);
	}
	catch (const std::exception& e)
	{
		LOG(LogWarning) << "Failed to read directory: " << path << " - " << e.what();
		return;
	}

	// Sort for consistent ordering
	std::sort(contents.begin(), contents.end());

	// Process each item (getDirContent returns full paths)
	for (const auto& fullPath : contents)
	{
		// Skip if shouldn't scan
		if (!shouldScanPath(fullPath))
			continue;

		// Detect game type
		GameType gameType = GameLauncherFactory::detectGameType(fullPath);

		if (gameType != GameType::UNKNOWN)
		{
			// Found a game!
			GameMetadata metadata = extractMetadata(fullPath, gameType);

			if (!metadata.name.empty())
			{
				LOG(LogInfo) << "Found game: " << metadata.name << " (" << metadata.executablePath << ")";
				games.push_back(metadata);
			}
		}
		else if (Utils::FileSystem::isDirectory(fullPath) && mRecursiveScan)
		{
			// Recursively scan subdirectory
			scanDirectoryRecursive(fullPath, games, currentDepth + 1);
		}
	}
}

bool LocalFilesystemPlugin::shouldScanPath(const std::string& path)
{
	std::string filename = Utils::FileSystem::getFileName(path);

	// Skip hidden files/directories
	if (!filename.empty() && filename[0] == '.')
		return false;

	// Skip common system directories
	std::vector<std::string> skipDirs = {
		"node_modules", "__pycache__", ".git", ".svn",
		"cache", "tmp", "temp", "build", "dist"
	};

	for (const auto& skip : skipDirs)
	{
		if (filename == skip)
			return false;
	}

	return true;
}

GameMetadata LocalFilesystemPlugin::extractMetadata(const std::string& path, GameType type)
{
	GameMetadata metadata;

	metadata.executablePath = path;
	metadata.gameType = type;
	metadata.source = GameSource::LOCAL;
	metadata.name = extractGameName(path);

	// For directories, store the directory as install path
	if (Utils::FileSystem::isDirectory(path))
	{
		metadata.installPath = path;

		// For Ren'Py games, look for the executable
		if (type == GameType::RENPY)
		{
			// Try to find the actual executable script
			Utils::FileSystem::stringList fileList = Utils::FileSystem::getDirContent(path);
			for (const auto& file : fileList)
			{
				std::string ext = Utils::FileSystem::getExtension(file);
				if (ext == ".sh" || ext == ".py")
				{
					// getDirContent returns full paths
					metadata.executablePath = file;
					break;
				}
			}
		}
	}
	else
	{
		metadata.installPath = Utils::FileSystem::getParent(path);
	}

	// Look for cover art
	metadata.coverArtPath = findCoverArt(metadata.installPath);

	// Set creation time to now
	metadata.createdAt = std::chrono::duration_cast<std::chrono::seconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();

	return metadata;
}

std::string LocalFilesystemPlugin::extractGameName(const std::string& path)
{
	std::string name;

	if (Utils::FileSystem::isDirectory(path))
	{
		// Use directory name
		name = Utils::FileSystem::getFileName(path);
	}
	else
	{
		// Use filename without extension
		name = Utils::FileSystem::getStem(path);
	}

	// Clean up the name
	// Replace underscores and dashes with spaces
	std::replace(name.begin(), name.end(), '_', ' ');
	std::replace(name.begin(), name.end(), '-', ' ');

	// Remove common suffixes
	std::vector<std::string> suffixes = {
		"linux", "x86_64", "x86", "amd64", "arm64",
		"appimage", "game", "v1", "v2", "final"
	};

	for (const auto& suffix : suffixes)
	{
		// Case-insensitive removal
		std::regex pattern("\\s+" + suffix + "\\s*$", std::regex::icase);
		name = std::regex_replace(name, pattern, "");
	}

	// Trim whitespace
	name.erase(0, name.find_first_not_of(" \t\n\r"));
	name.erase(name.find_last_not_of(" \t\n\r") + 1);

	// Capitalize first letter of each word
	bool capitalizeNext = true;
	for (size_t i = 0; i < name.length(); i++)
	{
		if (std::isspace(name[i]))
		{
			capitalizeNext = true;
		}
		else if (capitalizeNext)
		{
			name[i] = std::toupper(name[i]);
			capitalizeNext = false;
		}
	}

	return name;
}

std::string LocalFilesystemPlugin::findCoverArt(const std::string& gamePath)
{
	if (!Utils::FileSystem::exists(gamePath))
		return "";

	std::string searchDir = gamePath;
	if (!Utils::FileSystem::isDirectory(searchDir))
	{
		searchDir = Utils::FileSystem::getParent(searchDir);
	}

	// Common cover art filenames
	std::vector<std::string> coverNames = {
		"cover", "poster", "box", "boxart", "thumbnail", "icon"
	};

	// Common image extensions
	std::vector<std::string> imageExts = {
		".png", ".jpg", ".jpeg", ".gif", ".bmp", ".webp"
	};

	try
	{
		Utils::FileSystem::stringList fileList = Utils::FileSystem::getDirContent(searchDir);

		for (const auto& file : fileList)
		{
			std::string lowerFile = file;
			std::transform(lowerFile.begin(), lowerFile.end(), lowerFile.begin(), ::tolower);

			// Check if filename contains cover art keywords
			for (const auto& coverName : coverNames)
			{
				if (lowerFile.find(coverName) != std::string::npos)
				{
					// Check if it has an image extension
					for (const auto& ext : imageExts)
					{
						if (lowerFile.find(ext) != std::string::npos)
						{
							// getDirContent returns full paths
							LOG(LogDebug) << "Found cover art: " << file;
							return file;
						}
					}
				}
			}
		}
	}
	catch (const std::exception& e)
	{
		LOG(LogWarning) << "Failed to search for cover art: " << e.what();
	}

	return "";
}

bool LocalFilesystemPlugin::isPathScanned(const std::string& path)
{
	return mScannedPaths.find(path) != mScannedPaths.end();
}

} // namespace OpenConsole

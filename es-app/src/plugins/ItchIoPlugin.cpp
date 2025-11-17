#include "ItchIoPlugin.h"
#include "launchers/GameLauncherFactory.h"
#include "Log.h"
#include "utils/FileSystemUtil.h"
#include <boost/filesystem.hpp>
#include <memory>

namespace OpenConsole
{
	ItchIoPlugin::ItchIoPlugin()
		: mInitialized(false)
	{
		mApiClient = std::make_unique<ItchIoApiClient>();
	}

	ItchIoPlugin::~ItchIoPlugin()
	{
		shutdown();
	}

	bool ItchIoPlugin::initialize()
	{
		if (mInitialized)
			return true;

		LOG(LogInfo) << "ItchIoPlugin: Initializing";

		// Initialize token storage
		if (!TokenStorage::getInstance().initialize())
		{
			mLastError = "Failed to initialize token storage";
			LOG(LogError) << "ItchIoPlugin: " + mLastError;
			return false;
		}

		// Load API key from token storage
		std::string apiKey = TokenStorage::getInstance().getToken("itch_io");
		if (!apiKey.empty())
		{
			mApiClient->setApiKey(apiKey);
			LOG(LogInfo) << "ItchIoPlugin: Loaded API key from storage";
		}

		mInitialized = true;
		LOG(LogInfo) << "ItchIoPlugin: Initialized successfully";
		return true;
	}

	void ItchIoPlugin::shutdown()
	{
		if (!mInitialized)
			return;

		LOG(LogInfo) << "ItchIoPlugin: Shutting down";
		mInitialized = false;
	}

	bool ItchIoPlugin::isAuthenticated()
	{
		if (!mInitialized)
			return false;

		std::string apiKey = mApiClient->getApiKey();
		return !apiKey.empty();
	}

	AuthResult ItchIoPlugin::authenticate()
	{
		AuthResult result;

		if (!mInitialized)
		{
			result.errorMessage = "Plugin not initialized";
			return result;
		}

		// Test API key
		if (!mApiClient->testApiKey())
		{
			result.errorMessage = mApiClient->getLastError();
			return result;
		}

		// Get user profile
		ItchIoProfile profile;
		if (!mApiClient->getProfile(profile))
		{
			result.errorMessage = mApiClient->getLastError();
			return result;
		}

		// Save API key to token storage
		std::string apiKey = mApiClient->getApiKey();
		if (!TokenStorage::getInstance().storeToken("itch_io", apiKey))
		{
			result.errorMessage = "Failed to store API key";
			return result;
		}

		result.success = true;
		result.userId = std::to_string(profile.id);
		result.userName = profile.displayName.empty() ? profile.username : profile.displayName;

		LOG(LogInfo) << "ItchIoPlugin: Authenticated as " + result.userName;
		return result;
	}

	void ItchIoPlugin::logout()
	{
		if (!mInitialized)
			return;

		// Clear API key from client
		mApiClient->setApiKey("");

		// Remove from token storage
		TokenStorage::getInstance().removeToken("itch_io");

		LOG(LogInfo) << "ItchIoPlugin: Logged out";
	}

	void ItchIoPlugin::setApiKey(const std::string& apiKey)
	{
		mApiClient->setApiKey(apiKey);
	}

	std::string ItchIoPlugin::getApiKey() const
	{
		return mApiClient->getApiKey();
	}

	bool ItchIoPlugin::getUserProfile(ItchIoProfile& profile)
	{
		if (!isAuthenticated())
		{
			mLastError = "Not authenticated";
			return false;
		}

		return mApiClient->getProfile(profile);
	}

	std::vector<ItchIoGame> ItchIoPlugin::getItchIoGames()
	{
		if (!isAuthenticated())
		{
			mLastError = "Not authenticated";
			return {};
		}

		return mApiClient->getOwnedGames();
	}

	std::vector<GameMetadata> ItchIoPlugin::fetchGames()
	{
		std::vector<GameMetadata> games;

		if (!isAuthenticated())
		{
			mLastError = "Not authenticated with itch.io";
			LOG(LogWarning) << "ItchIoPlugin: " + mLastError;
			return games;
		}

		// Get owned games from itch.io
		std::vector<ItchIoGame> itchGames = mApiClient->getOwnedGames();
		if (itchGames.empty())
		{
			mLastError = mApiClient->getLastError();
			if (mLastError.empty())
				mLastError = "No owned games found";
			return games;
		}

		// Convert to GameMetadata
		for (const auto& itchGame : itchGames)
		{
			GameMetadata game = convertToGameMetadata(itchGame);
			games.push_back(game);
		}

		LOG(LogInfo) << "ItchIoPlugin: Fetched " << games.size() << " games";
		return games;
	}

	bool ItchIoPlugin::downloadGame(const GameMetadata& game,
		const std::string& installPath,
		DownloadProgressCallback progressCallback)
	{
		if (!isAuthenticated())
		{
			mLastError = "Not authenticated with itch.io";
			return false;
		}

		// Get download URL
		std::string downloadUrl = mApiClient->getDownloadUrl(game.uploadId);
		if (downloadUrl.empty())
		{
			mLastError = mApiClient->getLastError();
			return false;
		}

		// Create install directory
		boost::filesystem::path installDir(installPath);
		if (!boost::filesystem::exists(installDir))
		{
			boost::filesystem::create_directories(installDir);
		}

		// Determine output filename
		std::string filename = game.name;
		if (!game.downloadUrl.empty())
		{
			// Extract filename from original download filename if available
			size_t lastSlash = game.downloadUrl.find_last_of('/');
			if (lastSlash != std::string::npos)
				filename = game.downloadUrl.substr(lastSlash + 1);
		}

		// Sanitize filename
		for (char& c : filename)
		{
			if (c == '/' || c == '\\' || c == ':' || c == '*' ||
				c == '?' || c == '"' || c == '<' || c == '>' || c == '|')
			{
				c = '_';
			}
		}

		boost::filesystem::path outputPath = installDir / filename;

		// Download file
		if (!mApiClient->downloadFile(downloadUrl, outputPath.string(), progressCallback))
		{
			mLastError = mApiClient->getLastError();
			return false;
		}

		LOG(LogInfo) << "ItchIoPlugin: Downloaded " + game.name + " to " + outputPath.string();
		return true;
	}

	bool ItchIoPlugin::canHandleGameType(GameType type) const
	{
		// itch.io can provide various game types
		return type == GameType::APPIMAGE ||
		       type == GameType::RENPY ||
		       type == GameType::DEB ||
		       type == GameType::ELECTRON ||
		       type == GameType::DIRECTORY;
	}

	std::vector<GameType> ItchIoPlugin::getSupportedGameTypes() const
	{
		return {
			GameType::APPIMAGE,
			GameType::RENPY,
			GameType::DEB,
			GameType::ELECTRON,
			GameType::DIRECTORY
		};
	}

	GameMetadata ItchIoPlugin::convertToGameMetadata(const ItchIoGame& itchGame)
	{
		GameMetadata game;

		game.name = itchGame.title;
		game.description = itchGame.shortText;
		game.source = GameSource::ITCH_IO;

		// itch.io specific fields
		game.itchId = itchGame.id;
		game.uploadId = itchGame.uploadId;
		game.downloadUrl = itchGame.uploadFilename;

		// Detect game type from filename
		game.gameType = detectGameType(itchGame.uploadFilename);

		// Cover art URL (will be downloaded later if needed)
		if (!itchGame.coverUrl.empty())
		{
			game.coverArtPath = itchGame.coverUrl;
		}

		// Set install path to default itch.io downloads location
		std::string homeDir = Utils::FileSystem::getHomePath();
		game.installPath = homeDir + "/.openconsole/downloads/itch.io/" +
			std::to_string(itchGame.id);

		return game;
	}

	GameType ItchIoPlugin::detectGameType(const std::string& filename)
	{
		std::string lowerFilename = filename;
		std::transform(lowerFilename.begin(), lowerFilename.end(), lowerFilename.begin(), ::tolower);

		if (lowerFilename.find(".appimage") != std::string::npos)
			return GameType::APPIMAGE;

		if (lowerFilename.find(".deb") != std::string::npos)
			return GameType::DEB;

		if (lowerFilename.find("renpy") != std::string::npos ||
			lowerFilename.find(".ren") != std::string::npos)
			return GameType::RENPY;

		if (lowerFilename.find("electron") != std::string::npos)
			return GameType::ELECTRON;

		if (lowerFilename.find(".zip") != std::string::npos ||
			lowerFilename.find(".tar") != std::string::npos)
			return GameType::DIRECTORY;

		return GameType::UNKNOWN;
	}

	std::string ItchIoPlugin::downloadCoverArt(const ItchIoGame& itchGame, const std::string& gameInstallPath)
	{
		if (itchGame.coverUrl.empty())
			return "";

		// Create cover art path
		boost::filesystem::path installDir(gameInstallPath);
		boost::filesystem::path coverPath = installDir / "cover.jpg";

		// Download cover art
		if (mApiClient->downloadFile(itchGame.coverUrl, coverPath.string()))
		{
			return coverPath.string();
		}

		return "";
	}

} // namespace OpenConsole

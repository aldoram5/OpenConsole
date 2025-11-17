#pragma once
#ifndef ES_APP_PLUGINS_IGAME_SOURCE_PLUGIN_H
#define ES_APP_PLUGINS_IGAME_SOURCE_PLUGIN_H

#include "db/DatabaseManager.h"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace OpenConsole
{
	// Download progress callback
	// Parameters: bytesDownloaded, totalBytes
	using DownloadProgressCallback = std::function<void(size_t, size_t)>;

	// Plugin authentication result
	struct AuthResult
	{
		bool success;
		std::string errorMessage;
		std::string userId;      // Optional user identifier
		std::string userName;    // Optional user display name

		AuthResult() : success(false) {}
	};

	// Game source plugin interface
	class IGameSourcePlugin
	{
	public:
		virtual ~IGameSourcePlugin() {}

		// Get plugin name (e.g., "itch.io", "Local Filesystem", "USB Scanner")
		virtual std::string getName() const = 0;

		// Get plugin identifier (e.g., "itch_io", "local", "usb")
		virtual std::string getId() const = 0;

		// Get plugin version
		virtual std::string getVersion() const = 0;

		// Check if plugin requires authentication
		virtual bool requiresAuthentication() const = 0;

		// Check if plugin is currently authenticated
		virtual bool isAuthenticated() = 0;

		// Authenticate with the game source
		// Returns true on success, false on failure
		virtual AuthResult authenticate() = 0;

		// Logout/clear authentication
		virtual void logout() = 0;

		// Fetch available games from this source
		// Returns vector of GameMetadata
		virtual std::vector<GameMetadata> fetchGames() = 0;

		// Download and install a game
		// Parameters: game metadata, install path, progress callback
		// Returns true on success, false on failure
		virtual bool downloadGame(const GameMetadata& game,
			const std::string& installPath,
			DownloadProgressCallback progressCallback = nullptr) = 0;

		// Check if plugin can handle this game type
		virtual bool canHandleGameType(GameType type) const = 0;

		// Get supported game types
		virtual std::vector<GameType> getSupportedGameTypes() const = 0;

		// Initialize plugin (called once at startup)
		virtual bool initialize() = 0;

		// Shutdown plugin (called at exit)
		virtual void shutdown() = 0;

		// Get last error message
		virtual std::string getLastError() const = 0;
	};

	// Plugin factory function type
	using PluginFactoryFunc = std::function<std::shared_ptr<IGameSourcePlugin>()>;

} // namespace OpenConsole

#endif // ES_APP_PLUGINS_IGAME_SOURCE_PLUGIN_H

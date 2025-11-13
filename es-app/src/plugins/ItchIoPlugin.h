#pragma once
#ifndef ES_APP_PLUGINS_ITCHIO_PLUGIN_H
#define ES_APP_PLUGINS_ITCHIO_PLUGIN_H

#include "IGameSourcePlugin.h"
#include "api/ItchIoApiClient.h"
#include "utils/TokenStorage.h"
#include <memory>

namespace OpenConsole
{
	/**
	 * ItchIoPlugin - Plugin for itch.io game library integration
	 *
	 * Provides access to games purchased/claimed on itch.io.
	 * Requires API key authentication from itch.io user settings.
	 *
	 * Features:
	 * - Fetch owned games library
	 * - Download games with progress tracking
	 * - Secure token storage
	 * - Auto-detection of game types
	 *
	 * API Key Instructions:
	 * 1. Visit https://itch.io/user/settings/api-keys
	 * 2. Generate a new API key
	 * 3. Authenticate using the key in OpenConsole settings
	 */
	class ItchIoPlugin : public IGameSourcePlugin
	{
	public:
		ItchIoPlugin();
		virtual ~ItchIoPlugin();

		// IGameSourcePlugin interface
		std::string getName() const override { return "itch.io"; }
		std::string getId() const override { return "itch_io"; }
		std::string getVersion() const override { return "1.0.0"; }

		bool requiresAuthentication() const override { return true; }
		bool isAuthenticated() override;

		AuthResult authenticate() override;
		void logout() override;

		std::vector<GameMetadata> fetchGames() override;

		bool downloadGame(const GameMetadata& game,
			const std::string& installPath,
			DownloadProgressCallback progressCallback = nullptr) override;

		bool canHandleGameType(GameType type) const override;
		std::vector<GameType> getSupportedGameTypes() const override;

		bool initialize() override;
		void shutdown() override;

		std::string getLastError() const override { return mLastError; }

		// ItchIoPlugin-specific methods

		// Set API key for authentication
		// This is called by the UI when user enters their key
		void setApiKey(const std::string& apiKey);

		// Get current API key (for testing)
		std::string getApiKey() const;

		// Get user profile information
		// Returns true if authenticated and profile retrieved
		bool getUserProfile(ItchIoProfile& profile);

		// Get raw itch.io game list (before conversion to GameMetadata)
		std::vector<ItchIoGame> getItchIoGames();

	private:
		// Convert ItchIoGame to GameMetadata
		GameMetadata convertToGameMetadata(const ItchIoGame& itchGame);

		// Detect game type from filename
		GameType detectGameType(const std::string& filename);

		// Download cover art for a game
		std::string downloadCoverArt(const ItchIoGame& itchGame, const std::string& gameInstallPath);

		std::unique_ptr<ItchIoApiClient> mApiClient;
		std::string mLastError;
		bool mInitialized;
	};

} // namespace OpenConsole

#endif // ES_APP_PLUGINS_ITCHIO_PLUGIN_H

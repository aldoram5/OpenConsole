#pragma once
#ifndef ES_APP_PLUGINS_LOCAL_FILESYSTEM_PLUGIN_H
#define ES_APP_PLUGINS_LOCAL_FILESYSTEM_PLUGIN_H

#include "plugins/IGameSourcePlugin.h"
#include <set>

namespace OpenConsole
{
	// Local filesystem game scanner plugin
	// Scans configured directories for games and adds them to the database
	class LocalFilesystemPlugin : public IGameSourcePlugin
	{
	public:
		LocalFilesystemPlugin();
		virtual ~LocalFilesystemPlugin();

		// IGameSourcePlugin interface
		std::string getName() const override { return "Local Filesystem"; }
		std::string getId() const override { return "local"; }
		std::string getVersion() const override { return "1.0.0"; }

		bool requiresAuthentication() const override { return false; }
		bool isAuthenticated() override { return true; }

		AuthResult authenticate() override;
		void logout() override {}

		std::vector<GameMetadata> fetchGames() override;

		bool downloadGame(const GameMetadata& game,
			const std::string& installPath,
			DownloadProgressCallback progressCallback = nullptr) override;

		bool canHandleGameType(GameType type) const override;
		std::vector<GameType> getSupportedGameTypes() const override;

		bool initialize() override;
		void shutdown() override;

		std::string getLastError() const override { return mLastError; }

		// Local-specific methods
		void addScanPath(const std::string& path);
		void removeScanPath(const std::string& path);
		void clearScanPaths();
		std::vector<std::string> getScanPaths() const;

		// Set whether to scan recursively
		void setRecursiveScan(bool recursive) { mRecursiveScan = recursive; }
		bool isRecursiveScan() const { return mRecursiveScan; }

		// Set maximum scan depth (0 = unlimited)
		void setMaxScanDepth(int depth) { mMaxScanDepth = depth; }
		int getMaxScanDepth() const { return mMaxScanDepth; }

		// Scan specific directory (not in scan paths)
		std::vector<GameMetadata> scanDirectory(const std::string& path);

	private:
		// Recursive directory scanning
		void scanDirectoryRecursive(const std::string& path,
			std::vector<GameMetadata>& games,
			int currentDepth = 0);

		// Check if file/directory should be scanned
		bool shouldScanPath(const std::string& path);

		// Extract metadata from a game file/directory
		GameMetadata extractMetadata(const std::string& path, GameType type);

		// Extract game name from filename/directory
		std::string extractGameName(const std::string& path);

		// Look for cover art in game directory
		std::string findCoverArt(const std::string& gamePath);

		// Check if path has already been scanned
		bool isPathScanned(const std::string& path);

		// Supported game extensions
		std::set<std::string> mSupportedExtensions;

		// Directories to scan
		std::vector<std::string> mScanPaths;

		// Paths that have been scanned (to avoid duplicates)
		std::set<std::string> mScannedPaths;

		// Configuration
		bool mRecursiveScan;
		int mMaxScanDepth;

		// State
		bool mInitialized;
		std::string mLastError;
	};

} // namespace OpenConsole

#endif // ES_APP_PLUGINS_LOCAL_FILESYSTEM_PLUGIN_H

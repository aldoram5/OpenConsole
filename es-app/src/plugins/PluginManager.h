#pragma once
#ifndef ES_APP_PLUGINS_PLUGIN_MANAGER_H
#define ES_APP_PLUGINS_PLUGIN_MANAGER_H

#include "plugins/IGameSourcePlugin.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace OpenConsole
{
	// Plugin manager (Singleton)
	// Responsible for loading, registering, and managing game source plugins
	class PluginManager
	{
	public:
		// Get singleton instance
		static PluginManager& getInstance();

		// Delete copy constructor and assignment operator
		PluginManager(const PluginManager&) = delete;
		PluginManager& operator=(const PluginManager&) = delete;

		// Initialize plugin manager
		bool initialize();

		// Shutdown plugin manager and all plugins
		void shutdown();

		// Register a plugin
		bool registerPlugin(std::shared_ptr<IGameSourcePlugin> plugin);

		// Unregister a plugin by ID
		bool unregisterPlugin(const std::string& pluginId);

		// Get plugin by ID
		std::shared_ptr<IGameSourcePlugin> getPlugin(const std::string& pluginId);

		// Get all registered plugins
		std::vector<std::shared_ptr<IGameSourcePlugin>> getAllPlugins();

		// Get plugins by game type support
		std::vector<std::shared_ptr<IGameSourcePlugin>> getPluginsByGameType(GameType type);

		// Get authenticated plugins
		std::vector<std::shared_ptr<IGameSourcePlugin>> getAuthenticatedPlugins();

		// Check if a plugin is registered
		bool hasPlugin(const std::string& pluginId) const;

		// Refresh games from all authenticated plugins
		std::vector<GameMetadata> refreshAllGames();

		// Refresh games from specific plugin
		std::vector<GameMetadata> refreshGamesFromPlugin(const std::string& pluginId);

		// Get count of registered plugins
		size_t getPluginCount() const { return mPlugins.size(); }

	private:
		PluginManager();
		~PluginManager();

		// Register built-in plugins
		void registerBuiltInPlugins();

		std::map<std::string, std::shared_ptr<IGameSourcePlugin>> mPlugins;
		bool mInitialized;
	};

} // namespace OpenConsole

#endif // ES_APP_PLUGINS_PLUGIN_MANAGER_H

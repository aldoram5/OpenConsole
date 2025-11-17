#include "plugins/PluginManager.h"
#include "plugins/LocalFilesystemPlugin.h"
#include "plugins/ItchIoPlugin.h"
#include "Log.h"

namespace OpenConsole
{

PluginManager::PluginManager()
	: mInitialized(false)
{
}

PluginManager::~PluginManager()
{
	shutdown();
}

PluginManager& PluginManager::getInstance()
{
	static PluginManager instance;
	return instance;
}

bool PluginManager::initialize()
{
	if (mInitialized)
	{
		LOG(LogWarning) << "PluginManager already initialized";
		return true;
	}

	LOG(LogInfo) << "Initializing PluginManager...";

	// Register built-in plugins
	registerBuiltInPlugins();

	// Initialize all registered plugins
	for (auto& pair : mPlugins)
	{
		if (!pair.second->initialize())
		{
			LOG(LogWarning) << "Failed to initialize plugin: " << pair.first;
			// Continue with other plugins even if one fails
		}
		else
		{
			LOG(LogInfo) << "Initialized plugin: " << pair.first << " (" << pair.second->getName() << ")";
		}
	}

	mInitialized = true;
	LOG(LogInfo) << "PluginManager initialized with " << mPlugins.size() << " plugins";
	return true;
}

void PluginManager::shutdown()
{
	if (!mInitialized)
		return;

	LOG(LogInfo) << "Shutting down PluginManager...";

	// Shutdown all plugins
	for (auto& pair : mPlugins)
	{
		LOG(LogInfo) << "Shutting down plugin: " << pair.first;
		pair.second->shutdown();
	}

	mPlugins.clear();
	mInitialized = false;
	LOG(LogInfo) << "PluginManager shut down";
}

bool PluginManager::registerPlugin(std::shared_ptr<IGameSourcePlugin> plugin)
{
	if (!plugin)
	{
		LOG(LogError) << "Cannot register null plugin";
		return false;
	}

	std::string pluginId = plugin->getId();

	if (mPlugins.find(pluginId) != mPlugins.end())
	{
		LOG(LogWarning) << "Plugin already registered: " << pluginId;
		return false;
	}

	mPlugins[pluginId] = plugin;
	LOG(LogInfo) << "Registered plugin: " << pluginId << " (" << plugin->getName() << " v" << plugin->getVersion() << ")";
	return true;
}

bool PluginManager::unregisterPlugin(const std::string& pluginId)
{
	auto it = mPlugins.find(pluginId);
	if (it == mPlugins.end())
	{
		LOG(LogWarning) << "Plugin not found: " << pluginId;
		return false;
	}

	// Shutdown plugin before removing
	it->second->shutdown();
	mPlugins.erase(it);

	LOG(LogInfo) << "Unregistered plugin: " << pluginId;
	return true;
}

std::shared_ptr<IGameSourcePlugin> PluginManager::getPlugin(const std::string& pluginId)
{
	auto it = mPlugins.find(pluginId);
	if (it != mPlugins.end())
		return it->second;

	return nullptr;
}

std::vector<std::shared_ptr<IGameSourcePlugin>> PluginManager::getAllPlugins()
{
	std::vector<std::shared_ptr<IGameSourcePlugin>> plugins;
	plugins.reserve(mPlugins.size());

	for (auto& pair : mPlugins)
	{
		plugins.push_back(pair.second);
	}

	return plugins;
}

std::vector<std::shared_ptr<IGameSourcePlugin>> PluginManager::getPluginsByGameType(GameType type)
{
	std::vector<std::shared_ptr<IGameSourcePlugin>> plugins;

	for (auto& pair : mPlugins)
	{
		if (pair.second->canHandleGameType(type))
		{
			plugins.push_back(pair.second);
		}
	}

	return plugins;
}

std::vector<std::shared_ptr<IGameSourcePlugin>> PluginManager::getAuthenticatedPlugins()
{
	std::vector<std::shared_ptr<IGameSourcePlugin>> plugins;

	for (auto& pair : mPlugins)
	{
		// Include plugins that don't require auth OR are authenticated
		if (!pair.second->requiresAuthentication() || pair.second->isAuthenticated())
		{
			plugins.push_back(pair.second);
		}
	}

	return plugins;
}

bool PluginManager::hasPlugin(const std::string& pluginId) const
{
	return mPlugins.find(pluginId) != mPlugins.end();
}

std::vector<GameMetadata> PluginManager::refreshAllGames()
{
	std::vector<GameMetadata> allGames;

	LOG(LogInfo) << "Refreshing games from all authenticated plugins...";

	auto authenticatedPlugins = getAuthenticatedPlugins();
	for (auto& plugin : authenticatedPlugins)
	{
		LOG(LogInfo) << "Fetching games from: " << plugin->getName();

		try
		{
			auto games = plugin->fetchGames();
			LOG(LogInfo) << "Found " << games.size() << " games from " << plugin->getName();

			allGames.insert(allGames.end(), games.begin(), games.end());
		}
		catch (const std::exception& e)
		{
			LOG(LogError) << "Error fetching games from " << plugin->getName() << ": " << e.what();
		}
	}

	LOG(LogInfo) << "Total games found: " << allGames.size();
	return allGames;
}

std::vector<GameMetadata> PluginManager::refreshGamesFromPlugin(const std::string& pluginId)
{
	std::vector<GameMetadata> games;

	auto plugin = getPlugin(pluginId);
	if (!plugin)
	{
		LOG(LogError) << "Plugin not found: " << pluginId;
		return games;
	}

	if (plugin->requiresAuthentication() && !plugin->isAuthenticated())
	{
		LOG(LogWarning) << "Plugin requires authentication: " << pluginId;
		return games;
	}

	LOG(LogInfo) << "Fetching games from: " << plugin->getName();

	try
	{
		games = plugin->fetchGames();
		LOG(LogInfo) << "Found " << games.size() << " games from " << plugin->getName();
	}
	catch (const std::exception& e)
	{
		LOG(LogError) << "Error fetching games from " << plugin->getName() << ": " << e.what();
	}

	return games;
}

void PluginManager::registerBuiltInPlugins()
{
	LOG(LogInfo) << "Registering built-in plugins...";

	// Register LocalFilesystemPlugin
	auto localPlugin = std::make_shared<LocalFilesystemPlugin>();
	if (registerPlugin(localPlugin))
	{
		LOG(LogInfo) << "Registered LocalFilesystemPlugin";
	}
	else
	{
		LOG(LogError) << "Failed to register LocalFilesystemPlugin";
	}

	// Register ItchIoPlugin
	auto itchPlugin = std::make_shared<ItchIoPlugin>();
	if (registerPlugin(itchPlugin))
	{
		LOG(LogInfo) << "Registered ItchIoPlugin";
	}
	else
	{
		LOG(LogError) << "Failed to register ItchIoPlugin";
	}
}

} // namespace OpenConsole

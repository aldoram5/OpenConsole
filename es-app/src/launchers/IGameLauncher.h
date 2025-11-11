#pragma once
#ifndef ES_APP_LAUNCHERS_IGAME_LAUNCHER_H
#define ES_APP_LAUNCHERS_IGAME_LAUNCHER_H

#include "db/DatabaseManager.h"
#include <string>
#include <functional>

namespace OpenConsole
{
	// Launch result structure
	struct LaunchResult
	{
		bool success;
		int exitCode;
		std::string errorMessage;
		std::string stdOutput;
		std::string stdError;

		LaunchResult() : success(false), exitCode(-1) {}
	};

	// Pre-launch validation result
	struct ValidationResult
	{
		bool valid;
		std::string errorMessage;

		ValidationResult() : valid(true) {}
	};

	// Game launcher interface
	class IGameLauncher
	{
	public:
		virtual ~IGameLauncher() {}

		// Get launcher name
		virtual std::string getName() const = 0;

		// Get supported game type
		virtual GameType getSupportedType() const = 0;

		// Validate that the game can be launched
		// Checks: file exists, executable permissions, dependencies, etc.
		virtual ValidationResult validate(const GameMetadata& game) = 0;

		// Launch the game
		// Returns after game exits
		virtual LaunchResult launch(const GameMetadata& game) = 0;

		// Check if game is currently running
		virtual bool isRunning() const = 0;

		// Kill running game process (if supported)
		virtual bool killProcess() = 0;

		// Get process ID of running game (if available)
		virtual int getProcessId() const = 0;

		// Get last error message
		virtual std::string getLastError() const = 0;

		// Detect if a file/directory is this game type
		// Used for auto-detection during scanning
		static bool canLaunch(const std::string& path, GameType type);
	};

} // namespace OpenConsole

#endif // ES_APP_LAUNCHERS_IGAME_LAUNCHER_H

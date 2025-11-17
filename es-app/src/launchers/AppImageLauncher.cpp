#include "launchers/AppImageLauncher.h"
#include "utils/FileSystemUtil.h"
#include "platform.h"
#include "Log.h"
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <algorithm>

namespace OpenConsole
{

AppImageLauncher::AppImageLauncher()
	: mProcessId(-1)
{
}

AppImageLauncher::~AppImageLauncher()
{
	if (isRunning())
	{
		killProcess();
	}
}

ValidationResult AppImageLauncher::validate(const GameMetadata& game)
{
	ValidationResult result;

	// Check if file exists
	if (!Utils::FileSystem::exists(game.executablePath))
	{
		result.valid = false;
		result.errorMessage = "AppImage file does not exist: " + game.executablePath;
		return result;
	}

	// Check if it's a file (not directory)
	if (Utils::FileSystem::isDirectory(game.executablePath))
	{
		result.valid = false;
		result.errorMessage = "Path is a directory, not an AppImage file";
		return result;
	}

	// Check file extension
	std::string extension = Utils::FileSystem::getExtension(game.executablePath);
	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

	if (extension != ".appimage")
	{
		result.valid = false;
		result.errorMessage = "File does not have .appimage extension";
		return result;
	}

	// Check executable permissions
	if (!checkExecutablePermissions(game.executablePath))
	{
		LOG(LogWarning) << "AppImage is not executable, attempting to make executable: " << game.executablePath;

		if (!makeExecutable(game.executablePath))
		{
			result.valid = false;
			result.errorMessage = "Failed to make AppImage executable";
			return result;
		}
	}

	result.valid = true;
	return result;
}

LaunchResult AppImageLauncher::launch(const GameMetadata& game)
{
	LaunchResult result;
	mLastError.clear();

	// Validate first
	ValidationResult validation = validate(game);
	if (!validation.valid)
	{
		result.success = false;
		result.errorMessage = validation.errorMessage;
		mLastError = validation.errorMessage;
		return result;
	}

	LOG(LogInfo) << "Launching AppImage: " << game.name << " (" << game.executablePath << ")";

	// Fork and execute
	pid_t pid = fork();

	if (pid < 0)
	{
		// Fork failed
		result.success = false;
		result.errorMessage = "Failed to fork process";
		mLastError = result.errorMessage;
		LOG(LogError) << "Fork failed for AppImage: " << game.executablePath;
		return result;
	}
	else if (pid == 0)
	{
		// Child process - execute the AppImage
		execl(game.executablePath.c_str(), game.executablePath.c_str(), (char*)nullptr);

		// If execl returns, it failed
		LOG(LogError) << "execl failed for: " << game.executablePath;
		_exit(1);
	}
	else
	{
		// Parent process
		mProcessId = pid;
		LOG(LogInfo) << "AppImage launched with PID: " << mProcessId;

		// Wait for the game to finish
		int status;
		waitpid(pid, &status, 0);

		mProcessId = -1;

		if (WIFEXITED(status))
		{
			result.exitCode = WEXITSTATUS(status);
			result.success = (result.exitCode == 0);

			if (result.success)
			{
				LOG(LogInfo) << "AppImage exited successfully";
			}
			else
			{
				LOG(LogWarning) << "AppImage exited with code: " << result.exitCode;
				result.errorMessage = "Game exited with non-zero status";
				mLastError = result.errorMessage;
			}
		}
		else if (WIFSIGNALED(status))
		{
			result.success = false;
			result.exitCode = WTERMSIG(status);
			result.errorMessage = "Game was terminated by signal";
			mLastError = result.errorMessage;
			LOG(LogError) << "AppImage terminated by signal: " << result.exitCode;
		}
	}

	return result;
}

bool AppImageLauncher::killProcess()
{
	if (!isRunning())
		return false;

	LOG(LogWarning) << "Killing AppImage process: " << mProcessId;

	// Send SIGTERM first
	if (kill(mProcessId, SIGTERM) == 0)
	{
		// Wait a bit for graceful shutdown
		sleep(2);

		// Check if still running
		if (kill(mProcessId, 0) == 0)
		{
			// Still running, send SIGKILL
			kill(mProcessId, SIGKILL);
		}

		mProcessId = -1;
		return true;
	}

	return false;
}

bool AppImageLauncher::makeExecutable(const std::string& path)
{
	struct stat st;
	if (stat(path.c_str(), &st) != 0)
		return false;

	// Add executable bit for user, group, and others
	mode_t mode = st.st_mode | S_IXUSR | S_IXGRP | S_IXOTH;

	if (chmod(path.c_str(), mode) != 0)
	{
		LOG(LogError) << "Failed to chmod AppImage: " << path;
		return false;
	}

	LOG(LogInfo) << "Made AppImage executable: " << path;
	return true;
}

bool AppImageLauncher::checkExecutablePermissions(const std::string& path)
{
	struct stat st;
	if (stat(path.c_str(), &st) != 0)
		return false;

	// Check if user has execute permission
	return (st.st_mode & S_IXUSR) != 0;
}

} // namespace OpenConsole

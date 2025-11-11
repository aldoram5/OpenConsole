#include "launchers/RenpyLauncher.h"
#include "utils/FileSystemUtil.h"
#include "platform.h"
#include "Log.h"
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

namespace OpenConsole
{

RenpyLauncher::RenpyLauncher()
	: mProcessId(-1)
{
}

RenpyLauncher::~RenpyLauncher()
{
	if (isRunning())
	{
		killProcess();
	}
}

ValidationResult RenpyLauncher::validate(const GameMetadata& game)
{
	ValidationResult result;

	// Check if game path exists
	std::string gamePath = game.executablePath;
	if (gamePath.empty())
	{
		gamePath = game.installPath;
	}

	if (!Utils::FileSystem::exists(gamePath))
	{
		result.valid = false;
		result.errorMessage = "Ren'Py game path does not exist: " + gamePath;
		return result;
	}

	// If it's a .sh file, check if it exists and is executable
	if (Utils::FileSystem::getExtension(gamePath) == ".sh")
	{
		if (!Utils::FileSystem::exists(gamePath))
		{
			result.valid = false;
			result.errorMessage = "Ren'Py launcher script not found";
			return result;
		}
	}
	else if (Utils::FileSystem::isDirectory(gamePath))
	{
		// Find the executable in the directory
		std::string executable = findRenpyExecutable(gamePath);
		if (executable.empty())
		{
			result.valid = false;
			result.errorMessage = "Could not find Ren'Py executable in directory";
			return result;
		}
	}
	else
	{
		result.valid = false;
		result.errorMessage = "Invalid Ren'Py game path";
		return result;
	}

	result.valid = true;
	return result;
}

LaunchResult RenpyLauncher::launch(const GameMetadata& game)
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

	std::string executablePath = game.executablePath;
	if (executablePath.empty())
	{
		executablePath = game.installPath;
	}

	// If it's a directory, find the executable
	if (Utils::FileSystem::isDirectory(executablePath))
	{
		executablePath = findRenpyExecutable(executablePath);
	}

	LOG(LogInfo) << "Launching Ren'Py game: " << game.name << " (" << executablePath << ")";

	// Make sure it's executable
	makeExecutable(executablePath);

	// Get the directory to use as working directory
	std::string workingDir = Utils::FileSystem::getParent(executablePath);

	// Fork and execute
	pid_t pid = fork();

	if (pid < 0)
	{
		result.success = false;
		result.errorMessage = "Failed to fork process";
		mLastError = result.errorMessage;
		LOG(LogError) << "Fork failed for Ren'Py game: " << executablePath;
		return result;
	}
	else if (pid == 0)
	{
		// Child process
		// Change to game directory
		if (chdir(workingDir.c_str()) != 0)
		{
			LOG(LogError) << "Failed to chdir to: " << workingDir;
			_exit(1);
		}

		// Execute the game
		execl(executablePath.c_str(), executablePath.c_str(), (char*)nullptr);

		// If execl returns, it failed
		LOG(LogError) << "execl failed for: " << executablePath;
		_exit(1);
	}
	else
	{
		// Parent process
		mProcessId = pid;
		LOG(LogInfo) << "Ren'Py game launched with PID: " << mProcessId;

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
				LOG(LogInfo) << "Ren'Py game exited successfully";
			}
			else
			{
				LOG(LogWarning) << "Ren'Py game exited with code: " << result.exitCode;
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
			LOG(LogError) << "Ren'Py game terminated by signal: " << result.exitCode;
		}
	}

	return result;
}

bool RenpyLauncher::killProcess()
{
	if (!isRunning())
		return false;

	LOG(LogWarning) << "Killing Ren'Py process: " << mProcessId;

	if (kill(mProcessId, SIGTERM) == 0)
	{
		sleep(2);

		if (kill(mProcessId, 0) == 0)
		{
			kill(mProcessId, SIGKILL);
		}

		mProcessId = -1;
		return true;
	}

	return false;
}

std::string RenpyLauncher::findRenpyExecutable(const std::string& gamePath)
{
	// Common Ren'Py executable names
	std::vector<std::string> possibleNames = {
		"*.sh",
		"*.py",
		"renpy.sh",
		"start.sh",
		"game.sh",
		"launch.sh"
	};

	for (const auto& name : possibleNames)
	{
		std::string path = Utils::FileSystem::combine(gamePath, name);
		if (Utils::FileSystem::exists(path))
		{
			LOG(LogInfo) << "Found Ren'Py executable: " << path;
			return path;
		}
	}

	// If no .sh file found, look for Python scripts
	std::vector<std::string> files = Utils::FileSystem::getDirContent(gamePath);
	for (const auto& file : files)
	{
		std::string extension = Utils::FileSystem::getExtension(file);
		if (extension == ".sh" || extension == ".py")
		{
			std::string fullPath = Utils::FileSystem::combine(gamePath, file);
			LOG(LogInfo) << "Found potential Ren'Py executable: " << fullPath;
			return fullPath;
		}
	}

	return "";
}

bool RenpyLauncher::makeExecutable(const std::string& path)
{
	struct stat st;
	if (stat(path.c_str(), &st) != 0)
		return false;

	mode_t mode = st.st_mode | S_IXUSR | S_IXGRP | S_IXOTH;

	if (chmod(path.c_str(), mode) != 0)
	{
		LOG(LogError) << "Failed to chmod Ren'Py script: " << path;
		return false;
	}

	return true;
}

} // namespace OpenConsole

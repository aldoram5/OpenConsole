#include "launchers/DebLauncher.h"
#include "utils/FileSystemUtil.h"
#include "platform.h"
#include "Log.h"
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <fstream>
#include <sstream>

namespace OpenConsole
{

DebLauncher::DebLauncher()
	: mProcessId(-1)
{
}

DebLauncher::~DebLauncher()
{
	if (isRunning())
	{
		killProcess();
	}
}

ValidationResult DebLauncher::validate(const GameMetadata& game)
{
	ValidationResult result;

	// Check if .deb file exists
	if (!Utils::FileSystem::exists(game.executablePath))
	{
		result.valid = false;
		result.errorMessage = "DEB package file does not exist: " + game.executablePath;
		return result;
	}

	// Check file extension
	std::string extension = Utils::FileSystem::getExtension(game.executablePath);
	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

	if (extension != ".deb")
	{
		result.valid = false;
		result.errorMessage = "File does not have .deb extension";
		return result;
	}

	// Check if dpkg is available
	if (system("which dpkg > /dev/null 2>&1") != 0)
	{
		result.valid = false;
		result.errorMessage = "dpkg is not installed on this system";
		return result;
	}

	result.valid = true;
	return result;
}

LaunchResult DebLauncher::launch(const GameMetadata& game)
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

	LOG(LogInfo) << "Launching DEB package game: " << game.name;

	// Extract package name
	std::string packageName = extractPackageName(game.executablePath);
	if (packageName.empty())
	{
		result.success = false;
		result.errorMessage = "Failed to extract package name from .deb file";
		mLastError = result.errorMessage;
		return result;
	}

	// Check if already installed
	std::string binaryPath;
	if (!isPackageInstalled(packageName))
	{
		// Install the package
		LOG(LogInfo) << "Installing DEB package: " << packageName;

		if (!installPackage(game.executablePath, binaryPath))
		{
			result.success = false;
			result.errorMessage = "Failed to install DEB package";
			mLastError = result.errorMessage;
			return result;
		}
	}
	else
	{
		LOG(LogInfo) << "Package already installed: " << packageName;
		binaryPath = findInstalledBinary(packageName);
	}

	if (binaryPath.empty())
	{
		result.success = false;
		result.errorMessage = "Failed to find installed binary";
		mLastError = result.errorMessage;
		return result;
	}

	LOG(LogInfo) << "Launching installed binary: " << binaryPath;

	// Fork and execute
	pid_t pid = fork();

	if (pid < 0)
	{
		result.success = false;
		result.errorMessage = "Failed to fork process";
		mLastError = result.errorMessage;
		return result;
	}
	else if (pid == 0)
	{
		// Child process
		execl(binaryPath.c_str(), binaryPath.c_str(), (char*)nullptr);

		// If execl returns, it failed
		LOG(LogError) << "execl failed for: " << binaryPath;
		_exit(1);
	}
	else
	{
		// Parent process
		mProcessId = pid;
		LOG(LogInfo) << "Game launched with PID: " << mProcessId;

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
				LOG(LogInfo) << "Game exited successfully";
			}
			else
			{
				LOG(LogWarning) << "Game exited with code: " << result.exitCode;
			}
		}
		else if (WIFSIGNALED(status))
		{
			result.success = false;
			result.exitCode = WTERMSIG(status);
			result.errorMessage = "Game was terminated by signal";
			mLastError = result.errorMessage;
		}
	}

	return result;
}

bool DebLauncher::killProcess()
{
	if (!isRunning())
		return false;

	LOG(LogWarning) << "Killing process: " << mProcessId;

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

bool DebLauncher::installPackage(const std::string& debPath, std::string& installedBinary)
{
	// Note: Installing .deb packages requires root privileges
	// This is a simplified implementation that uses dpkg

	std::stringstream cmd;
	cmd << "dpkg -i " << debPath << " 2>&1";

	LOG(LogInfo) << "Installing package with command: " << cmd.str();

	FILE* pipe = popen(cmd.str().c_str(), "r");
	if (!pipe)
	{
		LOG(LogError) << "Failed to run dpkg command";
		return false;
	}

	// Read output
	char buffer[256];
	std::string output;
	while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
	{
		output += buffer;
	}

	int exitCode = pclose(pipe);

	if (exitCode != 0)
	{
		LOG(LogError) << "dpkg installation failed with exit code: " << exitCode;
		LOG(LogError) << "Output: " << output;
		return false;
	}

	LOG(LogInfo) << "Package installed successfully";

	// Extract package name and find binary
	std::string packageName = extractPackageName(debPath);
	installedBinary = findInstalledBinary(packageName);

	return !installedBinary.empty();
}

bool DebLauncher::isPackageInstalled(const std::string& packageName)
{
	std::stringstream cmd;
	cmd << "dpkg -s " << packageName << " 2>&1 | grep -q 'Status: install ok installed'";

	int result = system(cmd.str().c_str());
	return result == 0;
}

bool DebLauncher::uninstallPackage(const std::string& packageName)
{
	std::stringstream cmd;
	cmd << "dpkg -r " << packageName << " 2>&1";

	int result = system(cmd.str().c_str());
	return result == 0;
}

std::string DebLauncher::extractPackageName(const std::string& debPath)
{
	// Use dpkg-deb to extract package name
	std::stringstream cmd;
	cmd << "dpkg-deb -f " << debPath << " Package 2>&1";

	FILE* pipe = popen(cmd.str().c_str(), "r");
	if (!pipe)
		return "";

	char buffer[256];
	std::string packageName;

	if (fgets(buffer, sizeof(buffer), pipe) != nullptr)
	{
		packageName = buffer;
		// Remove trailing newline
		if (!packageName.empty() && packageName[packageName.length() - 1] == '\n')
		{
			packageName.erase(packageName.length() - 1);
		}
	}

	pclose(pipe);
	return packageName;
}

std::string DebLauncher::findInstalledBinary(const std::string& packageName)
{
	// List files installed by package
	std::stringstream cmd;
	cmd << "dpkg -L " << packageName << " 2>&1";

	FILE* pipe = popen(cmd.str().c_str(), "r");
	if (!pipe)
		return "";

	char buffer[256];
	std::string binaryPath;

	// Look for executable in /usr/bin or /usr/local/bin
	while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
	{
		std::string line = buffer;
		// Remove trailing newline
		if (!line.empty() && line[line.length() - 1] == '\n')
		{
			line.erase(line.length() - 1);
		}

		// Check if it's in /usr/bin or /usr/local/bin
		if (line.find("/usr/bin/") == 0 || line.find("/usr/local/bin/") == 0)
		{
			// Check if it's executable
			if (access(line.c_str(), X_OK) == 0)
			{
				binaryPath = line;
				break;
			}
		}
	}

	pclose(pipe);
	return binaryPath;
}

} // namespace OpenConsole

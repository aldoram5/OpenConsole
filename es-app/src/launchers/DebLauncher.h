#pragma once
#ifndef ES_APP_LAUNCHERS_DEB_LAUNCHER_H
#define ES_APP_LAUNCHERS_DEB_LAUNCHER_H

#include "launchers/IGameLauncher.h"

namespace OpenConsole
{
	// Launcher for .deb packages
	// Note: This installs the package and then launches the installed application
	class DebLauncher : public IGameLauncher
	{
	public:
		DebLauncher();
		virtual ~DebLauncher();

		std::string getName() const override { return "DEB Package Launcher"; }
		GameType getSupportedType() const override { return GameType::DEB; }

		ValidationResult validate(const GameMetadata& game) override;
		LaunchResult launch(const GameMetadata& game) override;

		bool isRunning() const override { return mProcessId > 0; }
		bool killProcess() override;
		int getProcessId() const override { return mProcessId; }
		std::string getLastError() const override { return mLastError; }

		// Install the .deb package
		bool installPackage(const std::string& debPath, std::string& installedBinary);

		// Check if package is already installed
		bool isPackageInstalled(const std::string& packageName);

		// Uninstall package
		bool uninstallPackage(const std::string& packageName);

	private:
		std::string extractPackageName(const std::string& debPath);
		std::string findInstalledBinary(const std::string& packageName);

		int mProcessId;
		std::string mLastError;
	};

} // namespace OpenConsole

#endif // ES_APP_LAUNCHERS_DEB_LAUNCHER_H

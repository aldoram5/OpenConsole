#pragma once
#ifndef ES_APP_LAUNCHERS_APPIMAGE_LAUNCHER_H
#define ES_APP_LAUNCHERS_APPIMAGE_LAUNCHER_H

#include "launchers/IGameLauncher.h"

namespace OpenConsole
{
	// Launcher for AppImage games
	class AppImageLauncher : public IGameLauncher
	{
	public:
		AppImageLauncher();
		virtual ~AppImageLauncher();

		std::string getName() const override { return "AppImage Launcher"; }
		GameType getSupportedType() const override { return GameType::APPIMAGE; }

		ValidationResult validate(const GameMetadata& game) override;
		LaunchResult launch(const GameMetadata& game) override;

		bool isRunning() const override { return mProcessId > 0; }
		bool killProcess() override;
		int getProcessId() const override { return mProcessId; }
		std::string getLastError() const override { return mLastError; }

	private:
		bool makeExecutable(const std::string& path);
		bool checkExecutablePermissions(const std::string& path);

		int mProcessId;
		std::string mLastError;
	};

} // namespace OpenConsole

#endif // ES_APP_LAUNCHERS_APPIMAGE_LAUNCHER_H

#pragma once
#ifndef ES_APP_LAUNCHERS_RENPY_LAUNCHER_H
#define ES_APP_LAUNCHERS_RENPY_LAUNCHER_H

#include "launchers/IGameLauncher.h"

namespace OpenConsole
{
	// Launcher for Ren'Py visual novel games
	class RenpyLauncher : public IGameLauncher
	{
	public:
		RenpyLauncher();
		virtual ~RenpyLauncher();

		std::string getName() const override { return "Ren'Py Launcher"; }
		GameType getSupportedType() const override { return GameType::RENPY; }

		ValidationResult validate(const GameMetadata& game) override;
		LaunchResult launch(const GameMetadata& game) override;

		bool isRunning() const override { return mProcessId > 0; }
		bool killProcess() override;
		int getProcessId() const override { return mProcessId; }
		std::string getLastError() const override { return mLastError; }

	private:
		std::string findRenpyExecutable(const std::string& gamePath);
		bool makeExecutable(const std::string& path);

		int mProcessId;
		std::string mLastError;
	};

} // namespace OpenConsole

#endif // ES_APP_LAUNCHERS_RENPY_LAUNCHER_H

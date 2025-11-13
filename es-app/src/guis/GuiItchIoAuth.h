#pragma once
#ifndef ES_APP_GUIS_GUI_ITCHIO_AUTH_H
#define ES_APP_GUIS_GUI_ITCHIO_AUTH_H

#include "GuiComponent.h"
#include "components/MenuComponent.h"
#include "components/ComponentGrid.h"
#include "components/TextComponent.h"
#include <functional>

namespace OpenConsole
{
	class ItchIoPlugin;
}

/**
 * GuiItchIoAuth - Dialog for itch.io API key authentication
 *
 * Allows users to:
 * - Enter their itch.io API key
 * - Test the API key
 * - View authentication status
 * - Access instructions for obtaining an API key
 *
 * API Key Instructions:
 * 1. Visit https://itch.io/user/settings/api-keys
 * 2. Click "Generate new API key"
 * 3. Copy the generated key
 * 4. Enter it in this dialog
 */
class GuiItchIoAuth : public GuiComponent
{
public:
	using AuthCompleteCallback = std::function<void(bool success, const std::string& username)>;

	GuiItchIoAuth(Window* window,
		AuthCompleteCallback authCompleteCallback = nullptr);

	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;
	std::vector<HelpPrompt> getHelpPrompts() override;

private:
	void enterApiKey();
	void testAuthentication();
	void viewInstructions();
	void close(bool success);

	MenuComponent mMenu;
	AuthCompleteCallback mAuthCompleteCallback;

	std::shared_ptr<TextComponent> mStatusText;
	std::shared_ptr<OpenConsole::ItchIoPlugin> mPlugin;

	std::string mApiKey;
	bool mAuthSuccess;
	std::string mUsername;
};

#endif // ES_APP_GUIS_GUI_ITCHIO_AUTH_H

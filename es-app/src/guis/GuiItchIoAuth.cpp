#include "guis/GuiItchIoAuth.h"
#include "guis/GuiMsgBox.h"
#include "guis/GuiVirtualKeyboard.h"
#include "plugins/ItchIoPlugin.h"
#include "plugins/PluginManager.h"
#include "Window.h"
#include "Log.h"

GuiItchIoAuth::GuiItchIoAuth(Window* window,
	AuthCompleteCallback authCompleteCallback)
	: GuiComponent(window),
	  mMenu(window, "ITCH.IO AUTHENTICATION"),
	  mAuthCompleteCallback(authCompleteCallback),
	  mAuthSuccess(false)
{
	addChild(&mMenu);

	// Get ItchIoPlugin
	auto& pluginManager = OpenConsole::PluginManager::getInstance();
	auto plugin = pluginManager.getPlugin("itch_io");
	if (plugin)
	{
		mPlugin = std::dynamic_pointer_cast<OpenConsole::ItchIoPlugin>(plugin);
	}

	// Status text
	mStatusText = std::make_shared<TextComponent>(mWindow, "", Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
	if (mPlugin && mPlugin->isAuthenticated())
	{
		mStatusText->setText("Status: Authenticated");
		mStatusText->setColor(0x00FF00FF); // Green
		mAuthSuccess = true;

		// Try to get username
		OpenConsole::ItchIoProfile profile;
		if (mPlugin->getUserProfile(profile))
		{
			mUsername = profile.displayName.empty() ? profile.username : profile.displayName;
			mStatusText->setText("Authenticated as: " + mUsername);
		}
	}
	else
	{
		mStatusText->setText("Status: Not authenticated");
		mStatusText->setColor(0xFF0000FF); // Red
	}

	mMenu.addWithLabel("Status", mStatusText);

	// Enter API Key button
	ComponentListRow row;
	row.addElement(std::make_shared<TextComponent>(mWindow, "ENTER API KEY", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	row.makeAcceptInputHandler([this] { enterApiKey(); });
	mMenu.addRow(row);

	// Instructions button
	row.elements.clear();
	row.addElement(std::make_shared<TextComponent>(mWindow, "VIEW INSTRUCTIONS", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	row.makeAcceptInputHandler([this] { viewInstructions(); });
	mMenu.addRow(row);

	// Test button (only if API key exists)
	if (mPlugin && !mPlugin->getApiKey().empty())
	{
		row.elements.clear();
		row.addElement(std::make_shared<TextComponent>(mWindow, "TEST CONNECTION", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
		row.makeAcceptInputHandler([this] { testAuthentication(); });
		mMenu.addRow(row);
	}

	// Logout button (only if authenticated)
	if (mPlugin && mPlugin->isAuthenticated())
	{
		row.elements.clear();
		row.addElement(std::make_shared<TextComponent>(mWindow, "LOGOUT", Font::get(FONT_SIZE_MEDIUM), 0xFF0000FF), true);
		row.makeAcceptInputHandler([this] {
			mPlugin->logout();
			mAuthSuccess = false;
			mUsername = "";
			close(false);
		});
		mMenu.addRow(row);
	}

	// Done button
	mMenu.addButton("DONE", "done", [this] { close(mAuthSuccess); });

	// Center the menu
	setSize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());
	mMenu.setPosition((mSize.x() - mMenu.getSize().x()) / 2, (mSize.y() - mMenu.getSize().y()) / 2);
}

void GuiItchIoAuth::enterApiKey()
{
	if (!mPlugin)
	{
		mWindow->pushGui(new GuiMsgBox(mWindow,
			"itch.io plugin not available",
			"OK", nullptr));
		return;
	}

	// Get current API key if any
	std::string currentKey = mPlugin->getApiKey();

	// Open virtual keyboard
	mWindow->pushGui(new GuiVirtualKeyboard(
		mWindow,
		"ENTER ITCH.IO API KEY",
		currentKey,
		[this](const std::string& apiKey) {
			// API key entered
			if (apiKey.empty())
			{
				mWindow->pushGui(new GuiMsgBox(mWindow,
					"API key cannot be empty",
					"OK", nullptr));
				return;
			}

			// Set API key
			mPlugin->setApiKey(apiKey);
			mApiKey = apiKey;

			// Try to authenticate
			testAuthentication();
		},
		false,
		"AUTHENTICATE"
	));
}

void GuiItchIoAuth::testAuthentication()
{
	if (!mPlugin)
	{
		mWindow->pushGui(new GuiMsgBox(mWindow,
			"itch.io plugin not available",
			"OK", nullptr));
		return;
	}

	// Authenticate
	OpenConsole::AuthResult result = mPlugin->authenticate();

	if (result.success)
	{
		mAuthSuccess = true;
		mUsername = result.userName;

		// Update status
		mStatusText->setText("Authenticated as: " + result.userName);
		mStatusText->setColor(0x00FF00FF); // Green

		// Show success message
		mWindow->pushGui(new GuiMsgBox(mWindow,
			"Authentication successful!\n\nLogged in as: " + result.userName,
			"OK", [this] {
				// Close the auth dialog on success
				close(true);
			}));
	}
	else
	{
		mAuthSuccess = false;
		mUsername = "";

		// Update status
		mStatusText->setText("Status: Authentication failed");
		mStatusText->setColor(0xFF0000FF); // Red

		// Show error message
		std::string errorMsg = "Authentication failed:\n" + result.errorMessage;
		errorMsg += "\n\nPlease check your API key and try again.";

		mWindow->pushGui(new GuiMsgBox(mWindow,
			errorMsg,
			"OK", nullptr));
	}
}

void GuiItchIoAuth::viewInstructions()
{
	std::string instructions =
		"HOW TO GET ITCH.IO API KEY:\n\n"
		"1. Open a web browser on another device\n"
		"2. Visit: itch.io/user/settings/api-keys\n"
		"3. Log in to your itch.io account\n"
		"4. Click 'Generate new API key'\n"
		"5. Copy the generated key\n"
		"6. Enter it using 'ENTER API KEY'\n\n"
		"Note: Keep your API key secret!\n"
		"Anyone with your key can access your games.";

	mWindow->pushGui(new GuiMsgBox(mWindow,
		instructions,
		"OK", nullptr));
}

void GuiItchIoAuth::close(bool success)
{
	if (mAuthCompleteCallback)
	{
		mAuthCompleteCallback(success, mUsername);
	}

	delete this;
}

bool GuiItchIoAuth::input(InputConfig* config, Input input)
{
	if (config->isMappedTo("b", input) && input.value != 0)
	{
		close(mAuthSuccess);
		return true;
	}

	return GuiComponent::input(config, input);
}

void GuiItchIoAuth::update(int deltaTime)
{
	GuiComponent::update(deltaTime);
}

std::vector<HelpPrompt> GuiItchIoAuth::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts = mMenu.getHelpPrompts();
	prompts.push_back(HelpPrompt("b", "back"));
	return prompts;
}

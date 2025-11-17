#pragma once
#ifndef ES_CORE_GUIS_GUI_VIRTUAL_KEYBOARD_H
#define ES_CORE_GUIS_GUI_VIRTUAL_KEYBOARD_H

#include "GuiComponent.h"
#include "components/ComponentGrid.h"
#include "components/TextComponent.h"
#include "components/NinePatchComponent.h"
#include <functional>

class ImageComponent;

// Virtual keyboard for controller-based text input
// Displays on-screen keyboard with D-pad navigation
class GuiVirtualKeyboard : public GuiComponent
{
public:
	// Callback when text entry is complete
	using OkCallback = std::function<void(const std::string&)>;

	GuiVirtualKeyboard(Window* window, const std::string& title,
		const std::string& initialText = "", const OkCallback& okCallback = nullptr,
		bool multiLine = false, const std::string& acceptBtnText = "OK");

	void onSizeChanged() override;
	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;
	std::vector<HelpPrompt> getHelpPrompts() override;

	// Get current text
	std::string getText() const { return mText; }

	// Set text programmatically
	void setText(const std::string& text);

private:
	void updateGrid();
	void updateText();
	void updateHelpPrompts();

	// Navigation
	void cursorLeft();
	void cursorRight();
	void cursorUp();
	void cursorDown();

	// Actions
	void pressKey();
	void backspace();
	void toggleShift();
	void addSpace();
	void accept();
	void cancel();

	// Keyboard layout
	void buildKeyboardLayout();
	std::string getKeyAt(int row, int col);

	NinePatchComponent mBackground;
	ComponentGrid mGrid;

	std::shared_ptr<TextComponent> mTitle;
	std::shared_ptr<TextComponent> mTextDisplay;
	std::shared_ptr<ComponentGrid> mKeyboardGrid;

	// Keyboard state
	std::string mText;
	bool mShift;
	bool mMultiLine;
	int mCursorRow;
	int mCursorCol;

	// Keyboard layout
	static const int KEYBOARD_ROWS = 5;
	static const int KEYBOARD_COLS = 11;
	std::string mKeys[KEYBOARD_ROWS][KEYBOARD_COLS];
	std::string mKeysShift[KEYBOARD_ROWS][KEYBOARD_COLS];
	std::shared_ptr<TextComponent> mKeyComponents[KEYBOARD_ROWS][KEYBOARD_COLS];

	// Button highlighting
	std::shared_ptr<ImageComponent> mCursor;

	// Callbacks
	OkCallback mOkCallback;
	std::string mAcceptBtnText;
};

#endif // ES_CORE_GUIS_GUI_VIRTUAL_KEYBOARD_H

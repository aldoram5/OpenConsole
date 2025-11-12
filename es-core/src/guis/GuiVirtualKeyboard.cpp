#include "guis/GuiVirtualKeyboard.h"
#include "components/TextComponent.h"
#include "components/ButtonComponent.h"
#include "components/MenuComponent.h"
#include "Log.h"
#include "Window.h"

GuiVirtualKeyboard::GuiVirtualKeyboard(Window* window, const std::string& title,
	const std::string& initialText, const OkCallback& okCallback,
	bool multiLine, const std::string& acceptBtnText)
	: GuiComponent(window)
	, mBackground(window, ":/frame.png")
	, mGrid(window, Vector2i(1, 5))
	, mText(initialText)
	, mShift(false)
	, mMultiLine(multiLine)
	, mCursorRow(0)
	, mCursorCol(0)
	, mOkCallback(okCallback)
	, mAcceptBtnText(acceptBtnText)
{
	addChild(&mBackground);
	addChild(&mGrid);

	// Build keyboard layout
	buildKeyboardLayout();

	// Title
	mTitle = std::make_shared<TextComponent>(mWindow, title, Font::get(FONT_SIZE_LARGE), 0x777777FF);
	mGrid.setEntry(mTitle, Vector2i(0, 0), false, true);

	// Text display
	mTextDisplay = std::make_shared<TextComponent>(mWindow, mText,
		Font::get(FONT_SIZE_MEDIUM), 0xFFFFFFFF, ALIGN_LEFT);
	mTextDisplay->setSize(0, Font::get(FONT_SIZE_MEDIUM)->getHeight() * (multiLine ? 3 : 1));
	mGrid.setEntry(mTextDisplay, Vector2i(0, 1), false, true);

	// Keyboard grid
	mKeyboardGrid = std::make_shared<ComponentGrid>(mWindow, Vector2i(KEYBOARD_COLS, KEYBOARD_ROWS));

	// Populate keyboard
	for (int row = 0; row < KEYBOARD_ROWS; row++)
	{
		for (int col = 0; col < KEYBOARD_COLS; col++)
		{
			std::string key = getKeyAt(row, col);
			if (key.empty())
				continue;

			auto keyText = std::make_shared<TextComponent>(mWindow, key,
				Font::get(FONT_SIZE_MEDIUM), 0xAAAAAAFF, ALIGN_CENTER);
			mKeyboardGrid->setEntry(keyText, Vector2i(col, row), false, false);
		}
	}

	mGrid.setEntry(mKeyboardGrid, Vector2i(0, 2), true, false);

	// Buttons
	std::vector<std::shared_ptr<ButtonComponent>> buttons;

	auto shiftBtn = std::make_shared<ButtonComponent>(mWindow, "SHIFT", "toggle shift",
		[this] { toggleShift(); });
	buttons.push_back(shiftBtn);

	auto spaceBtn = std::make_shared<ButtonComponent>(mWindow, "SPACE", "add space",
		[this] { addSpace(); });
	buttons.push_back(spaceBtn);

	auto backBtn = std::make_shared<ButtonComponent>(mWindow, "BACK", "delete character",
		[this] { backspace(); });
	buttons.push_back(backBtn);

	auto okBtn = std::make_shared<ButtonComponent>(mWindow, mAcceptBtnText, "accept",
		[this] { accept(); });
	buttons.push_back(okBtn);

	auto cancelBtn = std::make_shared<ButtonComponent>(mWindow, "CANCEL", "cancel",
		[this] { cancel(); });
	buttons.push_back(cancelBtn);

	auto buttonGrid = std::make_shared<ComponentGrid>(mWindow, Vector2i((int)buttons.size(), 1));
	for (size_t i = 0; i < buttons.size(); i++)
	{
		buttonGrid->setEntry(buttons[i], Vector2i((int)i, 0), true, false);
	}

	mGrid.setEntry(buttonGrid, Vector2i(0, 3), true, false);

	// Set grid sizes
	float titleHeight = mTitle->getFont()->getLetterHeight() + 10;
	float textHeight = mTextDisplay->getSize().y() + 10;
	float keyboardHeight = Font::get(FONT_SIZE_MEDIUM)->getHeight() * KEYBOARD_ROWS + 20;
	float buttonHeight = 40.0f;

	mGrid.setRowHeightPerc(0, titleHeight / 600.0f);
	mGrid.setRowHeightPerc(1, textHeight / 600.0f);
	mGrid.setRowHeightPerc(2, keyboardHeight / 600.0f);
	mGrid.setRowHeightPerc(3, buttonHeight / 600.0f);

	setSize(Renderer::getScreenWidth() * 0.8f, Renderer::getScreenHeight() * 0.7f);
	setPosition((Renderer::getScreenWidth() - mSize.x()) / 2,
		(Renderer::getScreenHeight() - mSize.y()) / 2);

	updateText();
}

void GuiVirtualKeyboard::buildKeyboardLayout()
{
	// Row 0: Numbers
	std::string row0[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-"};
	std::string row0Shift[] = {"!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "_"};

	// Row 1: QWERTY
	std::string row1[] = {"q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "["};
	std::string row1Shift[] = {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{"};

	// Row 2: ASDFGH
	std::string row2[] = {"a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'"};
	std::string row2Shift[] = {"A", "S", "D", "F", "G", "H", "J", "K", "L", ":", "\""};

	// Row 3: ZXCVBN
	std::string row3[] = {"z", "x", "c", "v", "b", "n", "m", ",", ".", "/", "]"};
	std::string row3Shift[] = {"Z", "X", "C", "V", "B", "N", "M", "<", ">", "?", "}"};

	// Row 4: Special
	std::string row4[] = {"", "", "", "", "", "", "", "", "", "", ""};
	std::string row4Shift[] = {"", "", "", "", "", "", "", "", "", "", ""};

	// Copy to arrays
	for (int i = 0; i < KEYBOARD_COLS; i++)
	{
		mKeys[0][i] = row0[i];
		mKeysShift[0][i] = row0Shift[i];

		mKeys[1][i] = row1[i];
		mKeysShift[1][i] = row1Shift[i];

		mKeys[2][i] = row2[i];
		mKeysShift[2][i] = row2Shift[i];

		mKeys[3][i] = row3[i];
		mKeysShift[3][i] = row3Shift[i];

		mKeys[4][i] = row4[i];
		mKeysShift[4][i] = row4Shift[i];
	}
}

std::string GuiVirtualKeyboard::getKeyAt(int row, int col)
{
	if (row < 0 || row >= KEYBOARD_ROWS || col < 0 || col >= KEYBOARD_COLS)
		return "";

	return mShift ? mKeysShift[row][col] : mKeys[row][col];
}

void GuiVirtualKeyboard::onSizeChanged()
{
	mBackground.fitTo(mSize, Vector3f::Zero(), Vector2f(-32, -32));
	mGrid.setSize(mSize);
}

bool GuiVirtualKeyboard::input(InputConfig* config, Input input)
{
	if (GuiComponent::input(config, input))
		return true;

	if (input.value != 0)
	{
		// Navigation
		if (config->isMappedTo("up", input))
		{
			cursorUp();
			return true;
		}
		if (config->isMappedTo("down", input))
		{
			cursorDown();
			return true;
		}
		if (config->isMappedTo("left", input))
		{
			cursorLeft();
			return true;
		}
		if (config->isMappedTo("right", input))
		{
			cursorRight();
			return true;
		}

		// Actions
		if (config->isMappedTo("a", input))
		{
			pressKey();
			return true;
		}
		if (config->isMappedTo("b", input))
		{
			backspace();
			return true;
		}
		if (config->isMappedTo("x", input))
		{
			addSpace();
			return true;
		}
		if (config->isMappedTo("y", input))
		{
			toggleShift();
			return true;
		}
		if (config->isMappedTo("start", input))
		{
			accept();
			return true;
		}
	}

	return false;
}

void GuiVirtualKeyboard::update(int deltaTime)
{
	GuiComponent::update(deltaTime);
}

void GuiVirtualKeyboard::cursorLeft()
{
	mCursorCol--;
	if (mCursorCol < 0)
		mCursorCol = KEYBOARD_COLS - 1;
}

void GuiVirtualKeyboard::cursorRight()
{
	mCursorCol++;
	if (mCursorCol >= KEYBOARD_COLS)
		mCursorCol = 0;
}

void GuiVirtualKeyboard::cursorUp()
{
	mCursorRow--;
	if (mCursorRow < 0)
		mCursorRow = KEYBOARD_ROWS - 1;
}

void GuiVirtualKeyboard::cursorDown()
{
	mCursorRow++;
	if (mCursorRow >= KEYBOARD_ROWS)
		mCursorRow = 0;
}

void GuiVirtualKeyboard::pressKey()
{
	std::string key = getKeyAt(mCursorRow, mCursorCol);
	if (!key.empty())
	{
		mText += key;
		updateText();

		// Auto-disable shift after one character
		if (mShift && key.length() == 1 && std::isalpha(key[0]))
		{
			mShift = false;
			updateGrid();
		}
	}
}

void GuiVirtualKeyboard::backspace()
{
	if (!mText.empty())
	{
		mText.pop_back();
		updateText();
	}
}

void GuiVirtualKeyboard::toggleShift()
{
	mShift = !mShift;
	updateGrid();
}

void GuiVirtualKeyboard::addSpace()
{
	mText += " ";
	updateText();
}

void GuiVirtualKeyboard::accept()
{
	if (mOkCallback)
		mOkCallback(mText);

	delete this;
}

void GuiVirtualKeyboard::cancel()
{
	delete this;
}

void GuiVirtualKeyboard::setText(const std::string& text)
{
	mText = text;
	updateText();
}

void GuiVirtualKeyboard::updateText()
{
	mTextDisplay->setText(mText);
}

void GuiVirtualKeyboard::updateGrid()
{
	// Update keyboard display for shift state
	for (int row = 0; row < KEYBOARD_ROWS; row++)
	{
		for (int col = 0; col < KEYBOARD_COLS; col++)
		{
			std::string key = getKeyAt(row, col);
			if (key.empty())
				continue;

			auto entry = mKeyboardGrid->getEntry(Vector2i(col, row));
			if (entry && entry->component)
			{
				auto textComp = std::dynamic_pointer_cast<TextComponent>(entry->component);
				if (textComp)
					textComp->setText(key);
			}
		}
	}
}

std::vector<HelpPrompt> GuiVirtualKeyboard::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("up/down/left/right", "navigate"));
	prompts.push_back(HelpPrompt("a", "select"));
	prompts.push_back(HelpPrompt("b", "delete"));
	prompts.push_back(HelpPrompt("x", "space"));
	prompts.push_back(HelpPrompt("y", "shift"));
	prompts.push_back(HelpPrompt("start", mAcceptBtnText));
	return prompts;
}

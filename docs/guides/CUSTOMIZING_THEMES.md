# Customizing Themes in OpenConsole

Make OpenConsole look exactly how you want with custom themes! This guide covers everything from installing existing themes to creating your own.

## Table of Contents

1. [What Are Themes?](#what-are-themes)
2. [Installing Themes](#installing-themes)
3. [Changing Themes](#changing-themes)
4. [Popular Themes](#popular-themes)
5. [Creating Custom Themes](#creating-custom-themes)
6. [Troubleshooting](#troubleshooting)

## What Are Themes?

Themes control the visual appearance of OpenConsole:
- **Layout**: How games and systems are displayed
- **Colors**: Background, text, and UI element colors
- **Fonts**: Typography and text styling
- **Graphics**: Background images, logos, icons
- **Animations**: Transitions and effects

OpenConsole is compatible with **EmulationStation themes**, which means thousands of existing themes will work!

## Installing Themes

Themes are installed to: `~/.openconsole/themes/` or `/etc/openconsole/themes/`

### Method 1: Git Clone (Recommended)

Most themes are hosted on GitHub. You can clone them directly:

```bash
# Create themes directory if it doesn't exist
mkdir -p ~/.openconsole/themes

# Clone a theme (example: Carbon theme)
cd ~/.openconsole/themes
git clone https://github.com/RetroPie/es-theme-carbon.git carbon

# Or use a different name
git clone https://github.com/RetroPie/es-theme-carbon.git my-carbon-theme
```

### Method 2: Download and Extract

1. **Download theme ZIP:**
   - Visit theme's GitHub page
   - Click **Code** → **Download ZIP**

2. **Extract to themes directory:**
   ```bash
   unzip es-theme-example.zip
   mv es-theme-example ~/.openconsole/themes/example
   ```

3. **Restart OpenConsole**

### Method 3: System-Wide Installation

Install themes for all users (requires sudo):

```bash
# Create system themes directory
sudo mkdir -p /etc/openconsole/themes

# Install theme
cd /etc/openconsole/themes
sudo git clone https://github.com/RetroPie/es-theme-carbon.git carbon
```

## Changing Themes

### Using the UI

1. **Open UI Settings:**
   - Press **Start** button
   - Navigate to **UI SETTINGS**

2. **Select Theme Set:**
   - Find **"THEME SET"** option
   - Press **A** to view available themes
   - Use D-pad to select a theme
   - Press **A** to apply

3. **Restart (if needed):**
   - Some themes may require a restart
   - Main Menu → **QUIT** → **RESTART OPENCONSOLE**

### Manual Configuration

Edit the settings file directly:

```bash
nano ~/.openconsole/es_settings.cfg
```

Find or add this line:
```xml
<string name="ThemeSet" value="carbon" />
```

Replace `"carbon"` with your theme's folder name.

## Popular Themes

Here are some excellent themes that work great with OpenConsole:

### Modern & Clean

**Carbon** (Dark, minimalist)
```bash
cd ~/.openconsole/themes
git clone https://github.com/RetroPie/es-theme-carbon.git carbon
```

**Pixel** (Retro pixel art style)
```bash
git clone https://github.com/RetroPie/es-theme-pixel.git pixel
```

### Colorful & Vibrant

**ComicBook** (Comic-style artwork)
```bash
git clone https://github.com/RetroPie/es-theme-ComicBook.git comicbook
```

**Magazinemadness** (Magazine covers)
```bash
git clone https://github.com/dmmarti/es-theme-magazinemadness.git magazinemadness
```

### Minimal & Fast

**Simple** (Ultra-minimal, fast)
```bash
git clone https://github.com/RetroPie/es-theme-simple.git simple
```

**Spare** (Clean and basic)
```bash
git clone https://github.com/mattrixk/es-theme-spare.git spare
```

### Grid Layouts

**NSC** (Nintendo Switch style)
```bash
git clone https://github.com/anthonycaccese/es-theme-nsc.git nsc
```

**Epic Noir** (Dark grid layout)
```bash
git clone https://github.com/c64-dev/es-theme-epicnoir.git epicnoir
```

> **Find More:** Browse [EmulationStation Themes on GitHub](https://github.com/topics/emulationstation-theme)

## Creating Custom Themes

Want to make your own theme? Here's how!

### Theme Structure

Basic theme directory structure:

```
my-theme/
  ├── theme.xml          # Main theme configuration
  ├── system/
  │   ├── theme.xml      # System-level overrides
  │   └── images/        # System-specific graphics
  ├── fonts/             # Custom fonts
  ├── images/            # Background images, UI elements
  └── README.md          # Theme information
```

### Creating a Simple Theme

1. **Create theme directory:**
   ```bash
   mkdir -p ~/.openconsole/themes/my-theme
   cd ~/.openconsole/themes/my-theme
   ```

2. **Create basic theme.xml:**
   ```xml
   <theme>
       <formatVersion>7</formatVersion>

       <!-- General view configuration -->
       <view name="system, basic, detailed, video">
           <!-- Background color -->
           <image name="background">
               <path>./images/background.png</path>
               <tile>true</tile>
           </image>

           <!-- Logo/header -->
           <image name="logo">
               <path>./system/images/${system.theme}.svg</path>
               <pos>0.5 0.15</pos>
               <maxSize>0.4 0.2</maxSize>
               <origin>0.5 0.5</origin>
           </image>

           <!-- Game list styling -->
           <textlist name="gamelist">
               <pos>0.1 0.3</pos>
               <size>0.8 0.6</size>
               <fontPath>./fonts/Ubuntu-Bold.ttf</fontPath>
               <fontSize>0.04</fontSize>
               <primaryColor>FFFFFF</primaryColor>
               <secondaryColor>AAAAAA</secondaryColor>
               <selectedColor>00FF00</selectedColor>
           </textlist>
       </view>
   </theme>
   ```

3. **Add images:**
   ```bash
   mkdir -p images system/images

   # Add your background image
   cp /path/to/background.png images/

   # Add system logos (PC theme icon)
   cp /path/to/pc.svg system/images/
   ```

4. **Test theme:**
   - Select your theme in UI Settings
   - Restart OpenConsole
   - Make adjustments as needed

### Theme XML Reference

**Common Elements:**

```xml
<!-- Colors (RRGGBBAA format, AA = transparency) -->
<primaryColor>FFFFFF</primaryColor>      <!-- White -->
<secondaryColor>00000080</secondaryColor> <!-- Black, 50% transparent -->

<!-- Positions (0.0 to 1.0, relative to screen size) -->
<pos>0.5 0.5</pos>  <!-- Center of screen -->
<pos>0 0</pos>      <!-- Top-left corner -->
<pos>1 1</pos>      <!-- Bottom-right corner -->

<!-- Sizes (0.0 to 1.0, relative to screen size) -->
<size>0.8 0.6</size>   <!-- 80% width, 60% height -->
<maxSize>0.4 0.2</maxSize> <!-- Max 40% width, 20% height -->

<!-- Fonts -->
<fontPath>./fonts/Ubuntu-Bold.ttf</fontPath>
<fontSize>0.04</fontSize>  <!-- 4% of screen height -->

<!-- Images -->
<path>./images/background.png</path>
<tile>true</tile>    <!-- Tile image across screen -->
<origin>0.5 0.5</origin>  <!-- Center image on position -->
```

**View Types:**

- `system`: System selection screen (main menu)
- `basic`: Simple game list
- `detailed`: Detailed game list with metadata
- `video`: Video preview game list

📖 **Full Documentation:** See [THEMES.md](../THEMES.md) for complete theme XML reference.

### Using Existing Themes as Templates

The best way to learn is by studying existing themes:

```bash
# Clone a simple theme to use as a base
cd ~/.openconsole/themes
git clone https://github.com/RetroPie/es-theme-simple.git my-theme-base

# Rename to your theme name
mv my-theme-base my-custom-theme

# Edit and customize
cd my-custom-theme
nano theme.xml
```

## Troubleshooting

### Theme Doesn't Appear in List

**Check theme location:**
```bash
ls -la ~/.openconsole/themes/
```

**Verify theme.xml exists:**
```bash
cat ~/.openconsole/themes/my-theme/theme.xml
```

**Check permissions:**
```bash
chmod -R 755 ~/.openconsole/themes/my-theme/
```

### Theme Looks Broken

**Check logs:**
```bash
tail -f ~/.openconsole/es_log.txt
```

Look for errors like:
- "Could not find theme..."
- "Error parsing theme..."
- "Missing image..."

**Verify image paths:**
- Make sure all referenced images exist
- Check that paths in theme.xml are correct
- Paths are relative to theme directory

**Test with default theme:**
- Switch to a known working theme (like Carbon)
- If that works, the issue is with your custom theme

### Missing System Icons

Some themes may not have icons for all systems. For OpenConsole specifically:

1. **Create PC system icon:**
   ```bash
   mkdir -p ~/.openconsole/themes/my-theme/system/images/
   # Add pc.svg, indie.svg, etc.
   ```

2. **Or use fallback:**
   - Most themes have a fallback/default icon
   - Edit theme.xml to specify fallback

### Performance Issues

**Use simpler themes:**
- Themes with many high-res images can slow down navigation
- Try themes marked as "lightweight" or "fast"

**Disable video previews:**
- UI Settings → Video Screensaver → OFF
- This can significantly improve performance

**Reduce theme image sizes:**
```bash
# Resize images in theme directory
cd ~/.openconsole/themes/my-theme/images/
mogrify -resize 1920x1080 *.png
```

## Theme Best Practices

### ✅ Do:
- Start with an existing theme as a template
- Test on target resolution (1920x1080 for most setups)
- Use relative positioning (0.0-1.0 values)
- Include README.md with credits and info
- Use web-safe fonts or include font files
- Optimize images (compress PNGs, use appropriate resolutions)

### ❌ Don't:
- Use absolute pixel positions (breaks on different resolutions)
- Forget to include all referenced images
- Use copyrighted artwork without permission
- Make text too small to read from a couch
- Overcomplicate – simple often looks best

## Sharing Your Theme

Created an awesome theme? Share it!

1. **Create GitHub repository:**
   ```bash
   cd ~/.openconsole/themes/my-theme
   git init
   git add .
   git commit -m "Initial theme release"
   ```

2. **Push to GitHub:**
   - Create repo on GitHub: `es-theme-mytheme`
   - Follow GitHub's push instructions

3. **Add README with:**
   - Screenshots
   - Installation instructions
   - Credits for any artwork/fonts used
   - Compatible systems list

4. **Share:**
   - Post on r/EmulationStation
   - Submit to RetroPie forums
   - Add to EmulationStation theme lists

## Resources

- 📖 **Theme XML Spec:** [THEMES.md](../THEMES.md)
- 🎨 **Theme Examples:** [RetroPie Themes](https://github.com/RetroPie/RetroPie-Setup/wiki/themes)
- 🖼️ **Free Graphics:** [OpenGameArt](https://opengameart.org/)
- 🔤 **Free Fonts:** [Google Fonts](https://fonts.google.com/)
- 🐛 **Get Help:** [GitHub Discussions](https://github.com/aldoram5/OpenConsole/discussions)

---

**Next Steps:** Explore the [Configuration Guide](CONFIGURATION.md) to learn about advanced OpenConsole customization!

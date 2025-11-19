# GitHub Pages Setup Instructions

This document explains how to enable and configure GitHub Pages for OpenConsole documentation.

## Automatic Deployment

The documentation will automatically deploy to GitHub Pages when:
- Changes are pushed to the `main` or `master` branch
- Files in the `docs/` directory are modified
- The workflow can also be triggered manually

## One-Time Setup

Follow these steps to enable GitHub Pages for your repository:

### 1. Enable GitHub Pages

1. Go to your repository on GitHub: https://github.com/aldoram5/OpenConsole
2. Click **Settings** (top menu)
3. Scroll down to **Pages** in the left sidebar
4. Under **Build and deployment**:
   - **Source**: Select "GitHub Actions"
   - (Don't use the old "Deploy from branch" option)

### 2. Verify Workflow Permissions

1. Still in **Settings**, go to **Actions** → **General**
2. Scroll to **Workflow permissions**
3. Ensure **"Read and write permissions"** is selected
4. Check **"Allow GitHub Actions to create and approve pull requests"**
5. Click **Save**

### 3. Trigger Initial Deployment

**Option A: Push to main branch**
```bash
# Merge your current branch to main
git checkout main
git merge your-branch-name
git push origin main
```

**Option B: Manual trigger**
1. Go to **Actions** tab on GitHub
2. Select **"Deploy Documentation to GitHub Pages"** workflow
3. Click **"Run workflow"**
4. Select branch: `main`
5. Click **"Run workflow"**

### 4. Wait for Deployment

- The workflow takes ~2-3 minutes to complete
- You can monitor progress in the **Actions** tab
- Once complete, your docs will be live!

### 5. Access Your Documentation

Your documentation will be available at:

**https://aldoram5.github.io/OpenConsole/**

## Local Testing

To test the documentation locally before pushing:

### Install Dependencies

```bash
cd docs

# Install Ruby and Bundler (if not already installed)
# On Ubuntu/Debian:
sudo apt-get install ruby-full build-essential

# Install bundler
gem install bundler

# Install dependencies
bundle install
```

### Run Local Server

```bash
cd docs
bundle exec jekyll serve
```

Visit: http://localhost:4000/OpenConsole/

The site will auto-reload when you make changes to markdown files.

### Build Locally

```bash
cd docs
bundle exec jekyll build
```

This creates the site in `docs/_site/`

## Workflow Configuration

The workflow is defined in `.github/workflows/deploy-docs.yml`:

```yaml
on:
  push:
    branches:
      - main
      - master
    paths:
      - 'docs/**'
      - '.github/workflows/deploy-docs.yml'
  workflow_dispatch:
```

**This means:**
- ✅ Deploys on push to `main` or `master`
- ✅ Only runs if `docs/` files changed (efficient!)
- ✅ Can be triggered manually
- ❌ Won't run on feature branches (saves resources)

## Customization

### Change Theme

Edit `docs/_config.yml`:

```yaml
theme: jekyll-theme-cayman  # Try: minimal, slate, modernist, etc.
```

[Browse Jekyll themes](https://pages.github.com/themes/)

### Update Site Info

Edit `docs/_config.yml`:

```yaml
title: OpenConsole Documentation
description: Your custom description
baseurl: "/OpenConsole"  # Must match repo name
url: "https://aldoram5.github.io"
```

### Add Pages

1. Create markdown file in `docs/`
2. Add front matter:
   ```yaml
   ---
   layout: default
   title: Page Title
   ---
   ```
3. Push to main branch
4. Auto-deploys!

## Troubleshooting

### Workflow Fails

**Check the Actions tab** for error details:
1. Go to **Actions** tab
2. Click on the failed workflow run
3. Click on the failed job (usually "build")
4. Expand the failed step to see errors

**Common issues:**

**Missing permissions:**
- Go to Settings → Actions → General
- Enable "Read and write permissions"

**Jekyll build errors:**
- Check `docs/_config.yml` syntax
- Ensure all referenced files exist
- Run `bundle exec jekyll build` locally to test

### Site Not Updating

1. **Check workflow ran:**
   - Actions tab should show successful run

2. **Clear browser cache:**
   - Hard refresh: Ctrl+Shift+R (or Cmd+Shift+R on Mac)

3. **Check GitHub Pages status:**
   - Settings → Pages
   - Should show green checkmark: "Your site is live at..."

4. **Verify branch:**
   - Workflow only runs on `main`/`master` branches
   - Check you pushed to the correct branch

### 404 Errors on Links

**Incorrect baseurl:**
- In `_config.yml`, `baseurl` must match your repo name
- Should be: `baseurl: "/OpenConsole"`

**Case sensitivity:**
- GitHub Pages is case-sensitive
- `GETTING_STARTED.md` ≠ `getting_started.md`

## Advanced Configuration

### Custom Domain

To use a custom domain (e.g., docs.openconsole.org):

1. **Add CNAME file:**
   ```bash
   echo "docs.openconsole.org" > docs/CNAME
   git add docs/CNAME
   git commit -m "Add custom domain"
   git push
   ```

2. **Configure DNS:**
   - Add CNAME record: `docs` → `aldoram5.github.io`
   - Wait for DNS propagation (~10 minutes)

3. **Update in GitHub:**
   - Settings → Pages
   - Enter custom domain: `docs.openconsole.org`
   - Check "Enforce HTTPS"

### Add Google Analytics

Edit `docs/_config.yml`:

```yaml
google_analytics: UA-XXXXXXXXX-X
```

### Add Search

Install jekyll-algolia plugin:

```ruby
# docs/Gemfile
gem 'jekyll-algolia'
```

Configure in `_config.yml` and add search box to layout.

## CI/CD Best Practices

### Only Deploy from Main

The workflow already does this:
```yaml
on:
  push:
    branches:
      - main
      - master
```

### Skip Unnecessary Builds

Path filters ensure workflow only runs when docs change:
```yaml
paths:
  - 'docs/**'
  - '.github/workflows/deploy-docs.yml'
```

### Manual Deployments

The `workflow_dispatch` trigger allows manual runs:
- Useful for testing
- Useful for re-deploying after config changes

## Monitoring

### Check Build Status

Add badge to README.md:

```markdown
[![Docs](https://github.com/aldoram5/OpenConsole/actions/workflows/deploy-docs.yml/badge.svg)](https://github.com/aldoram5/OpenConsole/actions/workflows/deploy-docs.yml)
```

### View Deployment History

- **Actions** tab → **Deploy Documentation to GitHub Pages**
- Shows all past deployments
- Can re-run failed deployments

## Support

- **Jekyll Docs:** https://jekyllrb.com/docs/
- **GitHub Pages:** https://docs.github.com/en/pages
- **GitHub Actions:** https://docs.github.com/en/actions

---

**That's it!** Your documentation is now automatically deployed to GitHub Pages. 🎉

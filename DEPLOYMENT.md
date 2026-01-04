# GitHub Pages Deployment Guide

## Project Structure

```
GLProject/
戍式式 .github/workflows/
弛   戌式式 deploy.yml              # GitHub Actions workflow
戍式式 DD_Engine/                  # Engine library
弛   戍式式 CMakeLists.txt          # Engine build configuration
弛   戍式式 DD_Core.cpp
弛   戍式式 DD_GLDevice.cpp
弛   戌式式 ...
戌式式 DD_Sample/                  # Sample application
    戍式式 CMakeLists.txt          # Sample build configuration
    戍式式 DD_Sample.cpp
    戍式式 shell.html              # Emscripten HTML template
    戌式式 docs/
        戌式式 index.html          # GitHub Pages landing page
```

---

## Auto Deployment Setup

### 1. Enable GitHub Pages (One-time setup)

1. Go to GitHub repository:
   ```
   https://github.com/jiwonchoidd/GLProject
   ```

2. Click **Settings** ⊥ **Pages**

3. Set **Source**:
   - Select **GitHub Actions**
   - Not "Deploy from a branch"

4. Click **Save**

### 2. Push Code for Auto Build

```bash
# 1. Modify code in Visual Studio
# Edit DD_Sample.cpp or engine files

# 2. Commit & Push
git add .
git commit -m "Update game"
git push origin main

# 3. GitHub Actions automatically:
#    - Installs Emscripten & GLM
#    - Builds DD_Engine library
#    - Builds DD_Sample application
#    - Generates .wasm, .js files
#    - Deploys to GitHub Pages
```

### 3. Check Build Status

1. Go to **GitHub** ⊥ **Actions** tab
2. Check latest workflow:
   - In progress (yellow)
   - Success (green check)
   - Failed (red X - check logs)

3. Wait 3-5 minutes after success

4. Visit website:
   ```
   https://jiwonchoidd.github.io/GLProject/
   ```

---

## Workflow Process

```
Local PC (git push)
    ⊿
GitHub Actions starts
    ⊿
忙式式式式式式式式式式式式式式式式式式式式式式式式式式式式式忖
弛 1. Create Ubuntu VM         弛
弛 2. Checkout code            弛
弛 3. Install Emscripten       弛
弛 4. Download GLM             弛
弛 5. Build DD_Engine          弛
弛 6. Build DD_Sample          弛
弛 7. Generate .wasm/.js       弛
弛 8. Deploy to Pages          弛
戌式式式式式式式式式式式式式式式式式式式式式式式式式式式式式戎
    ⊿
Website updated!
```

---

## Troubleshooting

### Build Failure

1. Click failed workflow in **Actions** tab
2. Click step with red X
3. Check logs
4. Common causes:
   - CMakeLists.txt path error
   - Missing source files
   - Compilation errors
   - Missing dependencies

### Key Log Locations

```yaml
Install GLM           # GLM download issues
Build Web Version     # Compilation errors
Prepare deployment    # File copy issues
Deploy to Pages       # Deployment errors
```

---

## Local Development Workflow

### Recommended:

```bash
# 1. Develop on Windows
Visual Studio ⊥ F5 (run/test desktop version)

# 2. Commit when ready
git add .
git commit -m "Add new feature"

# 3. Push
git push origin main

# 4. GitHub Actions builds web version automatically
# 5. Check result on website
```

### Benefits:
- No Emscripten installation needed locally
- No build environment management
- Auto-deploy on push
- Email notification on build failure
- Consistent build environment

---

## Private Repository Limits

- **2,000 minutes/month** (Free plan)
- Build time: ~3-5 minutes per run
- **400-600 builds per month available**
- More than enough for development!

---

## Local Testing (Optional)

To test with Emscripten locally:

```bash
# 1. Install Emscripten (once)
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
emsdk install latest
emsdk activate latest

# 2. Build from DD_Engine folder
cd DD_Engine
mkdir web-build
cd web-build
emcmake cmake ..
emmake make

# 3. Run local server
cd DD_Sample
python -m http.server 8000

# 4. Test in browser
http://localhost:8000/DD_Sample.html
```

But **GitHub Actions makes this unnecessary!**

---

## File Management

### Keep in DD_Sample:
- `CMakeLists.txt` - Sample build config
- `DD_Sample.cpp` - Sample code
- `shell.html` - Emscripten template
- `docs/index.html` - Landing page

### Keep in DD_Engine:
- `CMakeLists.txt` - Engine build config
- Source files (*.cpp, *.h)

### Keep in Root:
- `.github/workflows/deploy.yml` - CI/CD config
- `DEPLOYMENT.md` - This file
- `.gitignore` - Git ignore rules

---

## Checklist

- [x] Create `.github/workflows/deploy.yml`
- [x] Create `DD_Engine/CMakeLists.txt`
- [x] Create `DD_Sample/CMakeLists.txt`
- [x] Commit and push code
- [ ] GitHub ⊥ Settings ⊥ Pages ⊥ Select **GitHub Actions**
- [ ] Check build success in Actions tab
- [ ] Test website access

---

Simple, clean, and easy to manage!

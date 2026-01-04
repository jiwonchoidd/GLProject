# GitHub Pages Deployment Guide

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
# 1. Modify code
# Edit DD_Sample.cpp in Visual Studio

# 2. Commit & Push
git add .
git commit -m "Update game"
git push origin main

# 3. GitHub Actions automatically:
#    - Installs Emscripten
#    - Builds for web (.wasm, .js)
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
忙式式式式式式式式式式式式式式式式式式式式式式式式式忖
弛 1. Create Ubuntu VM     弛
弛 2. Checkout code        弛
弛 3. Install Emscripten   弛
弛 4. CMake build          弛
弛 5. Generate .wasm/.js   弛
弛 6. Deploy to Pages      弛
戌式式式式式式式式式式式式式式式式式式式式式式式式式戎
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

### Key Log Locations

```yaml
Build Web Version     # Build errors here
Prepare deployment    # File copy issues here
Deploy to Pages       # Deployment errors here
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

---

## Private Repository Limits

- **2,000 minutes/month** (Free plan)
- Build time: ~3-5 minutes per run
- **400-600 builds per month available**
- More than enough!

---

## Local Testing (Optional)

To test with Emscripten locally:

```bash
# 1. Install Emscripten (once)
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
emsdk install latest
emsdk activate latest

# 2. Build
cd DD_Sample
build-web.bat

# 3. Run local server
cd web-build
python -m http.server 8000

# 4. Test in browser
http://localhost:8000/DD_Sample.html
```

But **GitHub Actions makes this unnecessary!**

---

## Checklist

- [x] Create `.github/workflows/deploy.yml`
- [x] Commit and push code
- [ ] GitHub ⊥ Settings ⊥ Pages ⊥ Select **GitHub Actions**
- [ ] Check build success in Actions tab
- [ ] Test website access

---

Simple and easy to manage!

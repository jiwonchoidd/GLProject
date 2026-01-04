# GitHub Pages 배포 가이드 (로컬 빌드)

## ?? 빠른 배포 (Emscripten 없이도 가능!)

### 방법 1: 미리 빌드된 파일 사용 (가장 쉬움)
이미 빌드된 파일이 있다면:

```bash
# docs/ 폴더에 빌드 결과물 복사
# DD_Sample.html, DD_Sample.js, DD_Sample.wasm

git add docs/
git commit -m "Add web build files"
git push origin main
```

### 방법 2: 직접 빌드

#### Windows에서 Emscripten 설치 (한 번만)
```bash
# emsdk 다운로드
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
emsdk install latest
emsdk activate latest
```

#### 빌드 실행
```bash
# DD_Sample 폴더로 이동
cd DD_Sample

# 빌드
build-web.bat

# 결과물 복사
copy web-build\DD_Sample.* docs\
```

## ?? GitHub Pages 설정

### 1. 코드 푸시
```bash
git add .
git commit -m "Add web game files"
git push origin main
```

### 2. GitHub Pages 활성화
1. GitHub 저장소 → **Settings** → **Pages**
2. **Source**: **Deploy from a branch**
3. **Branch**: **main** → **/docs** 폴더 선택
4. **Save**

### 3. 확인
3-5분 후:
- `https://jiwonchoidd.github.io/GLProject/`

## ?? 로컬 테스트

```bash
cd docs
python -m http.server 8000
# http://localhost:8000/
```

## ?? 파일 구조

```
docs/
├── index.html          # 랜딩 페이지
├── DD_Sample.html      # 게임 (빌드 후 생성)
├── DD_Sample.js        # (빌드 후 생성)
└── DD_Sample.wasm      # (빌드 후 생성)
```

## ?? 팁

**Emscripten 없이 테스트하려면?**
- Windows 데스크탑 버전(`.exe`)으로 먼저 개발
- 완성되면 한 번만 Emscripten 빌드
- 빌드 결과물만 GitHub에 올리기

**온라인 빌드 서비스 이용:**
- GitHub Codespaces
- Replit
- CodeSandbox

더 간단하죠? ??

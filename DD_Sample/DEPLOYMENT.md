# GitHub Pages 배포 가이드

## 1?? GitHub 저장소 설정

### 코드 푸시
```bash
# 현재 디렉토리에서
git add .
git commit -m "Add web build support and GitHub Pages deployment"
git push origin main
```

## 2?? GitHub Pages 활성화

1. GitHub 저장소로 이동
2. **Settings** > **Pages** 클릭
3. **Source** 섹션에서:
   - Source: **GitHub Actions** 선택
4. 저장

## 3?? 자동 배포 확인

1. **Actions** 탭으로 이동
2. "Build and Deploy to GitHub Pages" 워크플로우 확인
3. 빌드 완료 후 사이트 URL에서 확인:
   - `https://jiwonchoidd.github.io/GLProject/`

## ?? 트러블슈팅

### 빌드 실패 시
- **Actions** 탭에서 로그 확인
- Emscripten 버전 호환성 확인
- CMakeLists.txt 경로 확인

### 페이지가 표시되지 않을 때
- Settings > Pages에서 GitHub Actions가 선택되었는지 확인
- 배포가 완료될 때까지 5-10분 대기
- 브라우저 캐시 삭제 후 재시도

### 로컬 빌드 테스트
```bash
# Emscripten이 설치되어 있다면
cd DD_Sample
./build-web.sh         # Linux/Mac
# 또는
build-web.bat          # Windows

# 빌드 결과 테스트
cd web-build
python -m http.server 8000
# http://localhost:8000/DD_Sample.html
```

## ?? 주의사항

1. **첫 푸시 후**: GitHub Actions가 자동으로 빌드를 시작합니다
2. **빌드 시간**: 첫 빌드는 3-5분 정도 소요됩니다
3. **브랜치**: `main` 브랜치에 푸시할 때마다 자동 배포됩니다

## ? 체크리스트

- [ ] 코드를 GitHub에 푸시
- [ ] Settings > Pages에서 GitHub Actions 활성화
- [ ] Actions 탭에서 빌드 성공 확인
- [ ] 배포된 사이트 URL 접속 테스트
- [ ] README.md의 데모 링크 확인

## ?? 성공!

모든 것이 정상적으로 작동하면:
- 메인 페이지: `https://jiwonchoidd.github.io/GLProject/`
- 데모: `https://jiwonchoidd.github.io/GLProject/DD_Sample.html`

에서 엔진이 실행됩니다!

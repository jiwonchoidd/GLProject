# ?? DD_Engine

OpenGL 기반 크로스 플랫폼 게임 엔진 (Windows & Web)

## ? 특징

- ?? **크로스 플랫폼**: Windows 네이티브 및 웹 브라우저 지원 (Emscripten)
- ? **모던 OpenGL**: OpenGL 3.3+ / WebGL 2.0
- ?? **C++14**: 깔끔하고 현대적인 C++ 코드베이스
- ?? **확장 가능**: 시스템 기반 아키텍처로 쉬운 확장
- ?? **간단한 API**: 직관적인 인터페이스

## ?? 빠른 시작

### Windows (Visual Studio)

1. Visual Studio 2022로 `DD_Sample.sln` 열기
2. NuGet 패키지 복원 (자동)
3. 빌드 및 실행 (F5)

### Web (Emscripten)

#### 사전 요구사항
- [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html)
- CMake 3.10+

#### 빌드

**Linux/Mac:**
```bash
cd DD_Sample
chmod +x build-web.sh
./build-web.sh
```

**Windows:**
```cmd
cd DD_Sample
build-web.bat
```

#### 로컬 테스트
```bash
cd DD_Sample/web-build
python -m http.server 8000
```

브라우저에서 `http://localhost:8000/DD_Sample.html` 열기

## ?? 사용 예제

```cpp
#include "../DD_Engine/DD_Core.h"
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow)
{
    DD_Core app(1280, 720, "DD_Engine Desktop");
    return DD_Application::Run(&app);
}
```

## ??? 프로젝트 구조

```
DD_Sample/
├── DD_Sample.cpp          # 메인 엔트리 포인트
├── CMakeLists.txt         # CMake 빌드 설정
├── shell.html             # Emscripten HTML 템플릿
├── build-web.sh/bat       # 웹 빌드 스크립트
└── docs/
    └── index.html         # GitHub Pages 랜딩 페이지

DD_Engine/
├── DD_Core.h/cpp          # 코어 엔진 클래스
├── DD_WinApplication.h/cpp # 애플리케이션 관리
├── DD_GLDevice.h/cpp      # OpenGL 디바이스 관리
├── DD_SimpleBox.h/cpp     # 예제: 박스 렌더링
├── DD_GLHelper.h          # OpenGL 유틸리티
└── framework.h            # 공통 정의 및 시스템 인터페이스
```

## ?? 온라인 데모

GitHub Actions를 통해 자동으로 빌드되고 배포됩니다:
**[데모 보기](https://jiwonchoidd.github.io/GLProject/)**

## ?? 시스템 요구사항

### Windows
- Windows 10 이상
- Visual Studio 2022
- OpenGL 3.3 지원 GPU

### Web
- 모던 브라우저 (Chrome, Firefox, Safari, Edge)
- WebGL 2.0 지원

## ?? 의존성

- **GLFW 3.4.0**: 윈도우 및 입력 처리
- **GLEW 2.2.0**: OpenGL 확장 로딩
- **GLM 1.0.3**: 수학 라이브러리
- **Emscripten**: 웹 빌드 (선택사항)

## ?? 기여

이슈와 풀 리퀘스트를 환영합니다!

## ?? 라이선스

이 프로젝트는 오픈 소스입니다.

## ?? 작성자

[@jiwonchoidd](https://github.com/jiwonchoidd)

---

Made with ?? using DD_Engine

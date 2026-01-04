새 샘플 만들기
===============

필요한 파일
-----------
YourSample/
├── CMakeLists.txt
└── YourSample.cpp

shell.html과 index.html은 자동으로 생성됩니다.


만드는 법
---------

1. 폴더 생성
   mkdir YourSample

2. CMakeLists.txt 복사 후 3곳 수정
   cp DD_Sample/CMakeLists.txt YourSample/
   
   수정할 곳:
   - project(YourSample)
   - add_executable(YourSample YourSample.cpp)
   - set_target_properties(YourSample ...)

3. YourSample.cpp 작성
   DD_Sample.cpp 참고해서 만들기

4. 푸시
   git add YourSample
   git commit -m "Add YourSample"
   git push
   
   GitHub Actions가 자동으로 빌드 및 배포


테스트
------
웹: https://username.github.io/project/


주의사항
--------
CMakeLists.txt의 프로젝트 이름 3곳 모두 동일하게 작성

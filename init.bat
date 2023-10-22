@echo off
set VCPKG_DEFAULT_TRIPLET=x64-windows-static
git clone https://github.com/Microsoft/vcpkg
cd vcpkg
git checkout 830f86fb309ad7167468a433a890b7415fbb90a5
call bootstrap-vcpkg.bat
vcpkg install sdl2 glm
rmdir /s /q .git
rmdir /s /q buildtrees
rmdir /s /q downloads
rmdir /s /q packages
cd ..
pause
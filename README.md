# OpenGL Lab 1 — 2D-лес и 3D-примитивы

Приложение на C++17 с использованием OpenGL и GLFW.
Два режима: 2D-сцена (лес из ёлочек) и 3D-сцена (куб, пирамида, сфера с освещением и прозрачностью).

Без зависимостей от GLU и GLUT — все вспомогательные функции (`gluPerspective`, `gluLookAt`, `glutSolidCube`, `gluSphere`) реализованы вручную.

## Зависимости

- CMake >= 3.10
- OpenGL
- GLFW3

## Сборка на Linux (gcc + CMake)

### Установка зависимостей

**Ubuntu / Debian:**
```bash
sudo apt update
sudo apt install build-essential cmake libglfw3-dev
```

**Fedora:**
```bash
sudo dnf install gcc-c++ cmake glfw-devel
```

**Arch Linux:**
```bash
sudo pacman -S base-devel cmake glfw
```

### Сборка и запуск

```bash
mkdir build
cd build
cmake ..
make
./lab1
```

## Сборка на Windows (Visual Studio + CMake)

### Установка зависимостей

1. Установите [CMake](https://cmake.org/download/) (добавьте в PATH).
2. Установите [Visual Studio](https://visualstudio.microsoft.com/) с компонентом «Разработка классических приложений на C++».
3. Установите GLFW одним из способов:
   - Через [vcpkg](https://vcpkg.io/): `vcpkg install glfw3`
   - Или скачайте бинарные файлы с [glfw.org](https://www.glfw.org/download.html) и задайте `glfw3_DIR` при вызове CMake.

### Сборка и запуск

```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
Release\lab1.exe
```

При использовании vcpkg добавьте флаг:
```cmd
cmake .. -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=[путь_к_vcpkg]/scripts/buildsystems/vcpkg.cmake
```

Либо откройте сгенерированный `.sln` файл в Visual Studio и соберите проект.

## Управление

| Клавиша       | Действие                        |
|---------------|----------------------------------|
| `1`           | Режим 2D (лес из ёлочек)        |
| `2`           | Режим 3D (примитивы)            |
| `W/A/S/D`     | Перемещение 2D-сцены             |
| Стрелки       | Перемещение камеры (3D)          |
| `Q` / `E`     | Камера ближе / дальше            |
| `+` / `-`     | Яркость освещения                |
| `[` / `]`     | Прозрачность объектов            |
| `ESC`         | Выход                            |

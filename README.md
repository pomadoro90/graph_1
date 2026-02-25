# OpenGL Lab 1 — 2D-лес и 3D-примитивы

Приложение на C++17 с использованием OpenGL, GLU и freeglut.
Два режима: 2D-сцена (лес из ёлочек) и 3D-сцена (куб, пирамида, сфера с освещением и прозрачностью).

## Зависимости

- CMake >= 3.10
- OpenGL
- GLU
- freeglut (или GLUT)

## Сборка на Linux (gcc + CMake)

### Установка зависимостей

**Ubuntu / Debian:**
```bash
sudo apt update
sudo apt install build-essential cmake freeglut3-dev
```

**Fedora:**
```bash
sudo dnf install gcc-c++ cmake freeglut-devel
```

**Arch Linux:**
```bash
sudo pacman -S base-devel cmake freeglut
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
3. Скачайте [freeglut](https://freeglut.sourceforge.net/) (предварительно скомпилированные бинарные файлы для MSVC).
4. Распакуйте freeglut и задайте переменную окружения `GLUT_ROOT_PATH`, указывающую на папку freeglut.

### Сборка и запуск

```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
Release\lab1.exe
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

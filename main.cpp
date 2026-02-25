// ═══════════════════════════════════════
// OpenGL Lab 1 — 2D-лес и 3D-примитивы
// Сборка: CMake + OpenGL + GLFW
// Единственный внешний заголовок: GLFW/glfw3.h
// ═══════════════════════════════════════

#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdlib>

// M_PI может отсутствовать на MSVC
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ═══════════════════════════════════════
// Именованные константы
// ═══════════════════════════════════════

// Размер окна по умолчанию
static const int WINDOW_WIDTH  = 800;
static const int WINDOW_HEIGHT = 600;

// Цвет фона (голубое небо)
static const float SKY_R = 0.53f;
static const float SKY_G = 0.81f;
static const float SKY_B = 0.92f;

// Ограничения смещения 2D-сцены
static const float OFFSET_X_MIN = -0.5f;
static const float OFFSET_X_MAX =  0.5f;
static const float OFFSET_Y_MIN = -0.3f;
static const float OFFSET_Y_MAX =  0.3f;

// Шаг смещения 2D-сцены
static const float OFFSET_STEP = 0.05f;

// Начальная позиция камеры
static const float CAM_X_INIT = 0.0f;
static const float CAM_Y_INIT = 1.0f;
static const float CAM_Z_INIT = 5.0f;

// Ограничения камеры
static const float CAM_X_MIN = -5.0f;
static const float CAM_X_MAX =  5.0f;
static const float CAM_Y_MIN = -3.0f;
static const float CAM_Y_MAX =  5.0f;
static const float CAM_Z_MIN =  1.0f;
static const float CAM_Z_MAX = 10.0f;

// Шаги камеры
static const float CAM_XY_STEP = 0.1f;
static const float CAM_Z_STEP  = 0.2f;

// Ограничения яркости
static const float BRIGHTNESS_MIN  = 0.1f;
static const float BRIGHTNESS_MAX  = 2.0f;
static const float BRIGHTNESS_STEP = 0.1f;

// Ограничения прозрачности
static const float TRANSPARENCY_MIN  = 0.1f;
static const float TRANSPARENCY_MAX  = 1.0f;
static const float TRANSPARENCY_STEP = 0.05f;

// Параметры пирамиды
static const float PYRAMID_HALF_BASE = 0.5f;
static const float PYRAMID_HEIGHT    = 1.0f;

// Параметры сферы
static const float SPHERE_RADIUS = 0.5f;
static const int   SPHERE_SLICES = 32;
static const int   SPHERE_STACKS = 32;

// Размер куба
static const float CUBE_SIZE = 0.8f;

// Цвет ствола дерева
static const float TRUNK_R = 0.4f;
static const float TRUNK_G = 0.2f;
static const float TRUNK_B = 0.05f;

// Цвет земли
static const float GROUND_R = 0.1f;
static const float GROUND_G = 0.3f;
static const float GROUND_B = 0.05f;

// ═══════════════════════════════════════
// Глобальные переменные состояния
// ═══════════════════════════════════════

// Указатель на окно GLFW
GLFWwindow* gWindow = nullptr;

// Смещение 2D-сцены
float offsetX = 0.0f;
float offsetY = 0.0f;

// Позиция камеры
float camX = CAM_X_INIT;
float camY = CAM_Y_INIT;
float camZ = CAM_Z_INIT;

// Яркость источника света
float lightBrightness = 1.0f;

// Прозрачность 3D-объектов
float transparency = 0.8f;

// Режим отображения: 0 = 2D-лес, 1 = 3D-объекты
int sceneMode = 0;

// ═══════════════════════════════════════
// Вспомогательная функция ограничения значения
// ═══════════════════════════════════════

static float clampf(float val, float lo, float hi) {
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

// ═══════════════════════════════════════
// Замена gluPerspective — перспективная проекция через glFrustum
// ═══════════════════════════════════════

static void myPerspective(double fovy, double aspect, double zNear, double zFar) {
    double top   = zNear * std::tan(fovy * M_PI / 360.0);
    double right = top * aspect;
    glFrustum(-right, right, -top, top, zNear, zFar);
}

// ═══════════════════════════════════════
// Замена gluLookAt — установка матрицы камеры вручную
// ═══════════════════════════════════════

static void myLookAt(double eyeX, double eyeY, double eyeZ,
                     double centerX, double centerY, double centerZ,
                     double upX, double upY, double upZ) {
    // Вектор направления взгляда (forward)
    double fx = centerX - eyeX;
    double fy = centerY - eyeY;
    double fz = centerZ - eyeZ;
    double fLen = std::sqrt(fx * fx + fy * fy + fz * fz);
    fx /= fLen; fy /= fLen; fz /= fLen;

    // Боковой вектор (side = forward x up)
    double sx = fy * upZ - fz * upY;
    double sy = fz * upX - fx * upZ;
    double sz = fx * upY - fy * upX;
    double sLen = std::sqrt(sx * sx + sy * sy + sz * sz);
    sx /= sLen; sy /= sLen; sz /= sLen;

    // Вертикальный вектор (u = side x forward)
    double ux = sy * fz - sz * fy;
    double uy = sz * fx - sx * fz;
    double uz = sx * fy - sy * fx;

    // Матрица в column-major порядке для OpenGL
    double m[16] = {
         sx,  ux, -fx, 0.0,
         sy,  uy, -fy, 0.0,
         sz,  uz, -fz, 0.0,
        0.0, 0.0, 0.0, 1.0
    };
    glMultMatrixd(m);
    glTranslated(-eyeX, -eyeY, -eyeZ);
}

// ═══════════════════════════════════════
// Замена glutSolidCube — куб из шести граней с нормалями
// ═══════════════════════════════════════

static void mySolidCube(float size) {
    float h = size / 2.0f;

    glBegin(GL_QUADS);
        // Передняя грань (+Z)
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(-h, -h,  h);
        glVertex3f( h, -h,  h);
        glVertex3f( h,  h,  h);
        glVertex3f(-h,  h,  h);

        // Задняя грань (-Z)
        glNormal3f(0.0f, 0.0f, -1.0f);
        glVertex3f( h, -h, -h);
        glVertex3f(-h, -h, -h);
        glVertex3f(-h,  h, -h);
        glVertex3f( h,  h, -h);

        // Верхняя грань (+Y)
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-h,  h,  h);
        glVertex3f( h,  h,  h);
        glVertex3f( h,  h, -h);
        glVertex3f(-h,  h, -h);

        // Нижняя грань (-Y)
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(-h, -h, -h);
        glVertex3f( h, -h, -h);
        glVertex3f( h, -h,  h);
        glVertex3f(-h, -h,  h);

        // Правая грань (+X)
        glNormal3f(1.0f, 0.0f, 0.0f);
        glVertex3f( h, -h,  h);
        glVertex3f( h, -h, -h);
        glVertex3f( h,  h, -h);
        glVertex3f( h,  h,  h);

        // Левая грань (-X)
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glVertex3f(-h, -h, -h);
        glVertex3f(-h, -h,  h);
        glVertex3f(-h,  h,  h);
        glVertex3f(-h,  h, -h);
    glEnd();
}

// ═══════════════════════════════════════
// Замена gluSphere — сфера из параметрических полос
// ═══════════════════════════════════════

static void mySolidSphere(float radius, int slices, int stacks) {
    for (int i = 0; i < stacks; ++i) {
        // Широтные углы для текущей и следующей полосы
        float lat0 = static_cast<float>(M_PI) * (-0.5f + static_cast<float>(i) / stacks);
        float lat1 = static_cast<float>(M_PI) * (-0.5f + static_cast<float>(i + 1) / stacks);

        float y0  = std::sin(lat0);
        float yr0 = std::cos(lat0);
        float y1  = std::sin(lat1);
        float yr1 = std::cos(lat1);

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= slices; ++j) {
            float lng = 2.0f * static_cast<float>(M_PI) * static_cast<float>(j) / slices;
            float x = std::cos(lng);
            float z = std::sin(lng);

            // Нижняя вершина полосы
            glNormal3f(x * yr0, y0, z * yr0);
            glVertex3f(radius * x * yr0, radius * y0, radius * z * yr0);

            // Верхняя вершина полосы
            glNormal3f(x * yr1, y1, z * yr1);
            glVertex3f(radius * x * yr1, radius * y1, radius * z * yr1);
        }
        glEnd();
    }
}

// ═══════════════════════════════════════
// Инициализация OpenGL
// ═══════════════════════════════════════

void initGL() {
    glClearColor(SKY_R, SKY_G, SKY_B, 1.0f); // голубое небо
    glEnable(GL_DEPTH_TEST);                   // тест глубины
    glEnable(GL_BLEND);                        // прозрачность
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_NORMALIZE);                    // нормализация нормалей
    glShadeModel(GL_SMOOTH);                   // плавное затенение
}

// ═══════════════════════════════════════
// Перестроение проекции при изменении размера окна
// ═══════════════════════════════════════

void reshape(int w, int h) {
    if (h == 0) h = 1; // защита от деления на ноль

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (sceneMode == 0) {
        // Ортографическая проекция для 2D
        glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    } else {
        // Перспективная проекция для 3D
        myPerspective(45.0, static_cast<double>(w) / h, 0.1, 100.0);
    }

    glMatrixMode(GL_MODELVIEW);
}

// Обёртка-колбэк для GLFW (framebuffer size)
static void framebufferSizeCallback(GLFWwindow* /*window*/, int w, int h) {
    reshape(w, h);
}

// ═══════════════════════════════════════
// Рисование одной ёлочки из трёх ярусов и ствола
// ═══════════════════════════════════════

void drawTree(float x, float y, float scale) {
    // Ствол — коричневый прямоугольник
    glColor3f(TRUNK_R, TRUNK_G, TRUNK_B);
    glBegin(GL_QUADS);
        glVertex2f(x - 0.03f * scale, y);
        glVertex2f(x + 0.03f * scale, y);
        glVertex2f(x + 0.03f * scale, y + 0.15f * scale);
        glVertex2f(x - 0.03f * scale, y + 0.15f * scale);
    glEnd();

    // Ярус 1 (нижний) — тёмно-зелёный
    glColor3f(0.05f, 0.4f, 0.05f);
    glBegin(GL_TRIANGLES);
        glVertex2f(x - 0.35f * scale, y + 0.1f * scale);
        glVertex2f(x + 0.35f * scale, y + 0.1f * scale);
        glVertex2f(x, y + 0.45f * scale);
    glEnd();

    // Ярус 2 (средний) — зелёный
    glColor3f(0.1f, 0.55f, 0.1f);
    glBegin(GL_TRIANGLES);
        glVertex2f(x - 0.27f * scale, y + 0.3f * scale);
        glVertex2f(x + 0.27f * scale, y + 0.3f * scale);
        glVertex2f(x, y + 0.6f * scale);
    glEnd();

    // Ярус 3 (верхний) — светло-зелёный
    glColor3f(0.15f, 0.65f, 0.15f);
    glBegin(GL_TRIANGLES);
        glVertex2f(x - 0.18f * scale, y + 0.48f * scale);
        glVertex2f(x + 0.18f * scale, y + 0.48f * scale);
        glVertex2f(x, y + 0.75f * scale);
    glEnd();
}

// ═══════════════════════════════════════
// Рисование леса из семи ёлочек и земли
// ═══════════════════════════════════════

void drawForest() {
    // Применить смещение 2D-сцены
    glTranslatef(offsetX, offsetY, 0.0f);

    // Земля — тёмно-зелёный прямоугольник
    glColor3f(GROUND_R, GROUND_G, GROUND_B);
    glBegin(GL_QUADS);
        glVertex2f(-1.0f, -1.0f);
        glVertex2f( 1.0f, -1.0f);
        glVertex2f( 1.0f, -0.6f);
        glVertex2f(-1.0f, -0.6f);
    glEnd();

    // Семь ёлочек с разными позициями и масштабами
    drawTree(-0.85f, -0.6f, 0.55f);
    drawTree(-0.55f, -0.65f, 0.45f);
    drawTree(-0.25f, -0.6f, 0.65f);
    drawTree( 0.05f, -0.7f, 0.38f);
    drawTree( 0.32f, -0.6f, 0.58f);
    drawTree( 0.62f, -0.65f, 0.42f);
    drawTree( 0.88f, -0.6f, 0.52f);
}

// ═══════════════════════════════════════
// Настройка источника света (только для 3D-режима)
// ═══════════════════════════════════════

void setupLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Позиция источника света
    GLfloat pos[]  = {2.0f, 3.0f, 4.0f, 1.0f};

    // Компоненты освещения, масштабированные яркостью
    GLfloat amb[]  = {0.2f * lightBrightness, 0.2f * lightBrightness,
                      0.2f * lightBrightness, 1.0f};
    GLfloat diff[] = {1.0f * lightBrightness, 1.0f * lightBrightness,
                      1.0f * lightBrightness, 1.0f};
    GLfloat spec[] = {0.8f * lightBrightness, 0.8f * lightBrightness,
                      0.8f * lightBrightness, 1.0f};

    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  diff);
    glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

    // Двустороннее освещение
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
}

// ═══════════════════════════════════════
// Рисование 3D-примитивов: куб, пирамида, сфера
// ═══════════════════════════════════════

void draw3DObjects() {
    // --- Куб (слева) ---
    glPushMatrix();
    glTranslatef(-1.5f, 0.0f, 0.0f);
    glRotatef(30.0f, 1.0f, 1.0f, 0.0f);

    // Материал куба — синий
    GLfloat cubeDiff[] = {0.2f, 0.4f, 0.8f, transparency};
    GLfloat cubeSpec[] = {0.9f, 0.9f, 0.9f, 1.0f};
    GLfloat cubeShin[] = {64.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,  cubeDiff);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, cubeSpec);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, cubeShin);

    mySolidCube(CUBE_SIZE);
    glPopMatrix();

    // --- Пирамида (по центру) ---
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f);

    // Материал пирамиды — оранжевый
    GLfloat pyrDiff[] = {0.8f, 0.3f, 0.1f, transparency};
    GLfloat pyrSpec[] = {0.9f, 0.9f, 0.9f, 1.0f};
    GLfloat pyrShin[] = {32.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,  pyrDiff);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, pyrSpec);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, pyrShin);

    // Вершины пирамиды
    float b = PYRAMID_HALF_BASE; // полуразмер основания
    float h = PYRAMID_HEIGHT;    // высота

    // Вершина
    float apex[3] = {0.0f, h, 0.0f};

    // Углы основания (Y = 0)
    float v0[3] = {-b, 0.0f, -b};
    float v1[3] = { b, 0.0f, -b};
    float v2[3] = { b, 0.0f,  b};
    float v3[3] = {-b, 0.0f,  b};

    // Нормали для боковых граней (вычислены аналитически)
    // Передняя грань (v2, v3, apex) — нормаль смотрит в +Z
    float nFront[3] = {0.0f, b, b};
    float lenF = std::sqrt(nFront[1] * nFront[1] + nFront[2] * nFront[2]);
    nFront[1] /= lenF; nFront[2] /= lenF;

    // Задняя грань (v0, v1, apex) — нормаль смотрит в -Z
    float nBack[3] = {0.0f, b, -b};
    float lenB = std::sqrt(nBack[1] * nBack[1] + nBack[2] * nBack[2]);
    nBack[1] /= lenB; nBack[2] /= lenB;

    // Правая грань (v1, v2, apex) — нормаль смотрит в +X
    float nRight[3] = {b, b, 0.0f};
    float lenR = std::sqrt(nRight[0] * nRight[0] + nRight[1] * nRight[1]);
    nRight[0] /= lenR; nRight[1] /= lenR;

    // Левая грань (v3, v0, apex) — нормаль смотрит в -X
    float nLeft[3] = {-b, b, 0.0f};
    float lenL = std::sqrt(nLeft[0] * nLeft[0] + nLeft[1] * nLeft[1]);
    nLeft[0] /= lenL; nLeft[1] /= lenL;

    // Боковые грани пирамиды
    glBegin(GL_TRIANGLES);
        // Передняя грань
        glNormal3fv(nFront);
        glVertex3fv(v2);
        glVertex3fv(v3);
        glVertex3fv(apex);

        // Задняя грань
        glNormal3fv(nBack);
        glVertex3fv(v0);
        glVertex3fv(v1);
        glVertex3fv(apex);

        // Правая грань
        glNormal3fv(nRight);
        glVertex3fv(v1);
        glVertex3fv(v2);
        glVertex3fv(apex);

        // Левая грань
        glNormal3fv(nLeft);
        glVertex3fv(v3);
        glVertex3fv(v0);
        glVertex3fv(apex);
    glEnd();

    // Основание пирамиды (нормаль вниз)
    glBegin(GL_QUADS);
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3fv(v0);
        glVertex3fv(v3);
        glVertex3fv(v2);
        glVertex3fv(v1);
    glEnd();

    glPopMatrix();

    // --- Сфера (справа) ---
    glPushMatrix();
    glTranslatef(1.5f, 0.0f, 0.0f);

    // Материал сферы — зелёный
    GLfloat sphDiff[] = {0.1f, 0.7f, 0.2f, transparency};
    GLfloat sphSpec[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat sphShin[] = {128.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,  sphDiff);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, sphSpec);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, sphShin);

    mySolidSphere(SPHERE_RADIUS, SPHERE_SLICES, SPHERE_STACKS);

    glPopMatrix();
}

// ═══════════════════════════════════════
// Основная функция отрисовки
// ═══════════════════════════════════════

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (sceneMode == 0) {
        // Режим 2D — отключить освещение, нарисовать лес
        glDisable(GL_LIGHTING);
        glPushMatrix();
        drawForest();
        glPopMatrix();
    } else {
        // Режим 3D — включить освещение, установить камеру
        setupLighting();
        myLookAt(camX, camY, camZ,
                 0.0, 0.0, 0.0,
                 0.0, 1.0, 0.0);
        draw3DObjects();
        glDisable(GL_LIGHTING);
    }
}

// ═══════════════════════════════════════
// Обработка клавиш (колбэк GLFW)
// ═══════════════════════════════════════

void keyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    // Реагируем на нажатие и повтор
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    switch (key) {
        // Перемещение 2D-сцены
        case GLFW_KEY_A:
            offsetX -= OFFSET_STEP;
            offsetX = clampf(offsetX, OFFSET_X_MIN, OFFSET_X_MAX);
            break;
        case GLFW_KEY_D:
            offsetX += OFFSET_STEP;
            offsetX = clampf(offsetX, OFFSET_X_MIN, OFFSET_X_MAX);
            break;
        case GLFW_KEY_W:
            offsetY += OFFSET_STEP;
            offsetY = clampf(offsetY, OFFSET_Y_MIN, OFFSET_Y_MAX);
            break;
        case GLFW_KEY_S:
            offsetY -= OFFSET_STEP;
            offsetY = clampf(offsetY, OFFSET_Y_MIN, OFFSET_Y_MAX);
            break;

        // Камера ближе / дальше (ось Z)
        case GLFW_KEY_Q:
            camZ -= CAM_Z_STEP;
            camZ = clampf(camZ, CAM_Z_MIN, CAM_Z_MAX);
            break;
        case GLFW_KEY_E:
            camZ += CAM_Z_STEP;
            camZ = clampf(camZ, CAM_Z_MIN, CAM_Z_MAX);
            break;

        // Яркость освещения (+ / =)
        case GLFW_KEY_EQUAL:
        case GLFW_KEY_KP_ADD:
            lightBrightness += BRIGHTNESS_STEP;
            lightBrightness = clampf(lightBrightness, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
            break;
        // Яркость освещения (-)
        case GLFW_KEY_MINUS:
        case GLFW_KEY_KP_SUBTRACT:
            lightBrightness -= BRIGHTNESS_STEP;
            lightBrightness = clampf(lightBrightness, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
            break;

        // Прозрачность 3D-объектов
        case GLFW_KEY_LEFT_BRACKET:
            transparency -= TRANSPARENCY_STEP;
            transparency = clampf(transparency, TRANSPARENCY_MIN, TRANSPARENCY_MAX);
            break;
        case GLFW_KEY_RIGHT_BRACKET:
            transparency += TRANSPARENCY_STEP;
            transparency = clampf(transparency, TRANSPARENCY_MIN, TRANSPARENCY_MAX);
            break;

        // Переключение режимов
        case GLFW_KEY_1: {
            sceneMode = 0;
            offsetX = 0.0f;
            offsetY = 0.0f;
            int w, h;
            glfwGetFramebufferSize(window, &w, &h);
            reshape(w, h);
            break;
        }
        case GLFW_KEY_2: {
            sceneMode = 1;
            int w, h;
            glfwGetFramebufferSize(window, &w, &h);
            reshape(w, h);
            break;
        }

        // Стрелки — перемещение камеры (3D)
        case GLFW_KEY_LEFT:
            camX -= CAM_XY_STEP;
            camX = clampf(camX, CAM_X_MIN, CAM_X_MAX);
            break;
        case GLFW_KEY_RIGHT:
            camX += CAM_XY_STEP;
            camX = clampf(camX, CAM_X_MIN, CAM_X_MAX);
            break;
        case GLFW_KEY_UP:
            camY += CAM_XY_STEP;
            camY = clampf(camY, CAM_Y_MIN, CAM_Y_MAX);
            break;
        case GLFW_KEY_DOWN:
            camY -= CAM_XY_STEP;
            camY = clampf(camY, CAM_Y_MIN, CAM_Y_MAX);
            break;

        // Выход по ESC
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;

        default:
            break;
    }
}

// ═══════════════════════════════════════
// Точка входа
// ═══════════════════════════════════════

int main(int /*argc*/, char** /*argv*/) {
    // Инициализация GLFW
    if (!glfwInit()) {
        return -1;
    }

    // Создание окна
    gWindow = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenGL Lab 1", nullptr, nullptr);
    if (!gWindow) {
        glfwTerminate();
        return -1;
    }

    // Установить контекст OpenGL
    glfwMakeContextCurrent(gWindow);

    // Регистрация колбэков
    glfwSetFramebufferSizeCallback(gWindow, framebufferSizeCallback);
    glfwSetKeyCallback(gWindow, keyCallback);

    // Инициализация OpenGL
    initGL();

    // Первоначальная настройка проекции
    {
        int w, h;
        glfwGetFramebufferSize(gWindow, &w, &h);
        reshape(w, h);
    }

    // Главный цикл отрисовки
    while (!glfwWindowShouldClose(gWindow)) {
        display();
        glfwSwapBuffers(gWindow);
        glfwPollEvents();
    }

    // Освобождение ресурсов
    glfwDestroyWindow(gWindow);
    glfwTerminate();
    return 0;
}

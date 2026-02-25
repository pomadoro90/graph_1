// ═══════════════════════════════════════
// OpenGL Lab 1 — 2D-лес и 3D-примитивы
// Сборка: CMake + OpenGL + freeglut
// ═══════════════════════════════════════

#include <cmath>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

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
static const float SPHERE_RADIUS  = 0.5f;
static const int   SPHERE_SLICES  = 32;
static const int   SPHERE_STACKS  = 32;

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
        gluPerspective(45.0, static_cast<double>(w) / h, 0.1, 100.0);
    }

    glMatrixMode(GL_MODELVIEW);
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
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   cubeDiff);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  cubeSpec);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS,  cubeShin);

    glutSolidCube(CUBE_SIZE);
    glPopMatrix();

    // --- Пирамида (по центру) ---
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f);

    // Материал пирамиды — оранжевый
    GLfloat pyrDiff[] = {0.8f, 0.3f, 0.1f, transparency};
    GLfloat pyrSpec[] = {0.9f, 0.9f, 0.9f, 1.0f};
    GLfloat pyrShin[] = {32.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   pyrDiff);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  pyrSpec);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS,  pyrShin);

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
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   sphDiff);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  sphSpec);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS,  sphShin);

    GLUquadricObj* q = gluNewQuadric();
    gluQuadricNormals(q, GLU_SMOOTH);
    gluSphere(q, SPHERE_RADIUS, SPHERE_SLICES, SPHERE_STACKS);
    gluDeleteQuadric(q);

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
        gluLookAt(camX, camY, camZ,
                  0.0, 0.0, 0.0,
                  0.0, 1.0, 0.0);
        draw3DObjects();
        glDisable(GL_LIGHTING);
    }

    glutSwapBuffers();
}

// ═══════════════════════════════════════
// Обработка обычных клавиш
// ═══════════════════════════════════════

void keyboard(unsigned char key, int /*x*/, int /*y*/) {
    switch (key) {
        // Перемещение 2D-сцены
        case 'a': case 'A':
            offsetX -= OFFSET_STEP;
            offsetX = clampf(offsetX, OFFSET_X_MIN, OFFSET_X_MAX);
            break;
        case 'd': case 'D':
            offsetX += OFFSET_STEP;
            offsetX = clampf(offsetX, OFFSET_X_MIN, OFFSET_X_MAX);
            break;
        case 'w': case 'W':
            offsetY += OFFSET_STEP;
            offsetY = clampf(offsetY, OFFSET_Y_MIN, OFFSET_Y_MAX);
            break;
        case 's': case 'S':
            offsetY -= OFFSET_STEP;
            offsetY = clampf(offsetY, OFFSET_Y_MIN, OFFSET_Y_MAX);
            break;

        // Камера ближе / дальше (ось Z)
        case 'q': case 'Q':
            camZ -= CAM_Z_STEP;
            camZ = clampf(camZ, CAM_Z_MIN, CAM_Z_MAX);
            break;
        case 'e': case 'E':
            camZ += CAM_Z_STEP;
            camZ = clampf(camZ, CAM_Z_MIN, CAM_Z_MAX);
            break;

        // Яркость освещения
        case '+': case '=':
            lightBrightness += BRIGHTNESS_STEP;
            lightBrightness = clampf(lightBrightness, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
            break;
        case '-':
            lightBrightness -= BRIGHTNESS_STEP;
            lightBrightness = clampf(lightBrightness, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
            break;

        // Прозрачность 3D-объектов
        case '[':
            transparency -= TRANSPARENCY_STEP;
            transparency = clampf(transparency, TRANSPARENCY_MIN, TRANSPARENCY_MAX);
            break;
        case ']':
            transparency += TRANSPARENCY_STEP;
            transparency = clampf(transparency, TRANSPARENCY_MIN, TRANSPARENCY_MAX);
            break;

        // Переключение режимов
        case '1':
            sceneMode = 0;
            offsetX = 0.0f;
            offsetY = 0.0f;
            reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
            break;
        case '2':
            sceneMode = 1;
            reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
            break;

        // Выход по ESC
        case 27:
            exit(0);
            break;

        default:
            break;
    }

    glutPostRedisplay();
}

// ═══════════════════════════════════════
// Обработка специальных клавиш (стрелки)
// ═══════════════════════════════════════

void specialKeys(int key, int /*x*/, int /*y*/) {
    switch (key) {
        case GLUT_KEY_LEFT:
            camX -= CAM_XY_STEP;
            camX = clampf(camX, CAM_X_MIN, CAM_X_MAX);
            break;
        case GLUT_KEY_RIGHT:
            camX += CAM_XY_STEP;
            camX = clampf(camX, CAM_X_MIN, CAM_X_MAX);
            break;
        case GLUT_KEY_UP:
            camY += CAM_XY_STEP;
            camY = clampf(camY, CAM_Y_MIN, CAM_Y_MAX);
            break;
        case GLUT_KEY_DOWN:
            camY -= CAM_XY_STEP;
            camY = clampf(camY, CAM_Y_MIN, CAM_Y_MAX);
            break;
        default:
            break;
    }

    glutPostRedisplay();
}

// ═══════════════════════════════════════
// Точка входа
// ═══════════════════════════════════════

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("OpenGL Lab 1");

    // Регистрация функций обратного вызова
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);

    // Инициализация OpenGL
    initGL();

    // Главный цикл обработки событий
    glutMainLoop();

    return 0;
}

#ifdef __cplusplus
#include <cmath>
#else
#include <math.h>
#include <stdbool.h>
#endif
#include <GL/glut.h>

const float PI = 3.14159265359f;
bool isDay = true;

float cloudX = -1.2f;
float birdX = 1.2f;
float wavePhase = 0.0f;
float birdFlapPhase = 0.0f;
float rainPhase = 0.0f;
bool isRaining = false;

void drawCircle(float cx, float cy, float r, int segments = 120) {
    glBegin(GL_POLYGON);
    for (int i = 0; i < segments; i++) {
        float a = 2.0f * PI * i / segments;
        glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
    }
    glEnd();
}

void drawEllipse(float cx, float cy, float rx, float ry, int segments = 140) {
    glBegin(GL_POLYGON);
    for (int i = 0; i < segments; i++) {
        float a = 2.0f * PI * i / segments;
        glVertex2f(cx + rx * cosf(a), cy + ry * sinf(a));
    }
    glEnd();
}

void drawLine(float x1, float y1, float x2, float y2, float w = 2.0f) {
    glLineWidth(w);
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}

void drawArc(float cx, float cy, float r, float startA, float endA, float w = 2.0f) {
    glLineWidth(w);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 24; i++) {
        float t = startA + (endA - startA) * (float)i / 24.0f;
        glVertex2f(cx + r * cosf(t), cy + r * sinf(t));
    }
    glEnd();
}

// ============================================================================
// LINE DRAWING ALGORITHMS - DDA and Bresenham for 2D and 3D
// ============================================================================

// DDA (Digital Differential Analyzer) Algorithm - 2D
void drawLineDDA2D(float x1, float y1, float x2, float y2, float w = 2.0f) {
    glLineWidth(w);
    glBegin(GL_LINE_STRIP);

    float dx = x2 - x1;
    float dy = y2 - y1;
    int steps = (int)((fabsf(dx) > fabsf(dy)) ? fabsf(dx) : fabsf(dy));

    if (steps == 0) {
        glVertex2f(x1, y1);
    } else {
        float xIncrement = dx / steps;
        float yIncrement = dy / steps;
        float x = x1;
        float y = y1;

        for (int i = 0; i <= steps; i++) {
            glVertex2f(x, y);
            x += xIncrement;
            y += yIncrement;
        }
    }
    glEnd();
}

// Bresenham's Line Algorithm - 2D (Integer-based for efficiency)
void drawLineBresenham2D(int x1, int y1, int x2, int y2, float w = 2.0f) {
    glLineWidth(w);
    glBegin(GL_POINTS);

    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    int x = x1, y = y1;
    while (true) {
        glVertex2f((float)x / 1000.0f, (float)y / 1000.0f);  // Normalize to screen coords
        if (x == x2 && y == y2) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
    glEnd();
}

// DDA Algorithm - 3D Version
void drawLineDDA3D(float x1, float y1, float z1, float x2, float y2, float z2, float w = 2.0f) {
    glLineWidth(w);
    glBegin(GL_LINE_STRIP);

    float dx = x2 - x1;
    float dy = y2 - y1;
    float dz = z2 - z1;

    float maxDelta = (fabsf(dx) > fabsf(dy)) ? fabsf(dx) : fabsf(dy);
    maxDelta = (maxDelta > fabsf(dz)) ? maxDelta : fabsf(dz);

    int steps = (int)maxDelta;
    if (steps == 0) {
        glVertex3f(x1, y1, z1);
    } else {
        float xIncrement = dx / steps;
        float yIncrement = dy / steps;
        float zIncrement = dz / steps;
        float x = x1;
        float y = y1;
        float z = z1;

        for (int i = 0; i <= steps; i++) {
            glVertex3f(x, y, z);
            x += xIncrement;
            y += yIncrement;
            z += zIncrement;
        }
    }
    glEnd();
}

// Bresenham's Line Algorithm - 3D (Generalized for 3D space)
// Uses octant-based approach similar to 2D but extends to 3D
void drawLineBresenham3D(int x1, int y1, int z1, int x2, int y2, int z2, float w = 2.0f) {
    glLineWidth(w);
    glBegin(GL_LINE_STRIP);

    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int dz = abs(z2 - z1);

    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int sz = (z1 < z2) ? 1 : -1;

    // Determine primary axis (largest difference)
    int x = x1, y = y1, z = z1;

    if (dx >= dy && dx >= dz) {
        // X is primary axis
        int err1 = dx / 2;
        int err2 = dx / 2;

        while (x != x2) {
            glVertex3f((float)x / 1000.0f, (float)y / 1000.0f, (float)z / 1000.0f);

            err1 -= dy;
            if (err1 < 0) {
                y += sy;
                err1 += dx;
            }
            err2 -= dz;
            if (err2 < 0) {
                z += sz;
                err2 += dx;
            }
            x += sx;
        }
    } else if (dy >= dx && dy >= dz) {
        // Y is primary axis
        int err1 = dy / 2;
        int err2 = dy / 2;

        while (y != y2) {
            glVertex3f((float)x / 1000.0f, (float)y / 1000.0f, (float)z / 1000.0f);

            err1 -= dx;
            if (err1 < 0) {
                x += sx;
                err1 += dy;
            }
            err2 -= dz;
            if (err2 < 0) {
                z += sz;
                err2 += dy;
            }
            y += sy;
        }
    } else {
        // Z is primary axis
        int err1 = dz / 2;
        int err2 = dz / 2;

        while (z != z2) {
            glVertex3f((float)x / 1000.0f, (float)y / 1000.0f, (float)z / 1000.0f);

            err1 -= dx;
            if (err1 < 0) {
                x += sx;
                err1 += dz;
            }
            err2 -= dy;
            if (err2 < 0) {
                y += sy;
                err2 += dz;
            }
            z += sz;
        }
    }
    glVertex3f((float)x2 / 1000.0f, (float)y2 / 1000.0f, (float)z2 / 1000.0f);
    glEnd();
}

// ============================================================================

void drawVerticalGradient(float x1, float y1, float x2, float y2,
                          unsigned char rTop, unsigned char gTop, unsigned char bTop,
                          unsigned char rBottom, unsigned char gBottom, unsigned char bBottom) {
    glBegin(GL_QUADS);
    glColor3ub(rBottom, gBottom, bBottom);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glColor3ub(rTop, gTop, bTop);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

void drawSoilTexture() {
    // Deterministic micro-speckles for matte realistic soil
    glPointSize(1.8f);
    glBegin(GL_POINTS);
    for (int i = 0; i < 220; i++) {
        float x = -0.98f + (1.96f * (i % 37) / 36.0f);
        float y = -0.96f + (1.04f * ((i * 7) % 41) / 40.0f);
        if (y > 0.11f) {
            continue;
        }
        float tone = 0.52f + 0.18f * ((i % 9) / 8.0f);
        glColor4f(0.55f * tone, 0.46f * tone, 0.35f * tone, 0.22f);
        glVertex2f(x, y);
    }
    glEnd();
}

void drawSkyAndCelestialBody() {
    if (isDay) {
        drawVerticalGradient(-1.0f, 0.12f, 1.0f, 1.0f, 150, 210, 240, 212, 236, 252);

        // Very visible sun
        glColor3ub(255, 214, 58);
        drawCircle(0.00f, 0.78f, 0.085f);
        glColor3ub(255, 245, 170);
        drawCircle(0.00f, 0.78f, 0.045f);

        // Soft sun glow
        glColor4f(1.0f, 0.85f, 0.35f, 0.30f);
        drawCircle(0.00f, 0.78f, 0.14f);

        // Sun rays (day only)
        glColor4f(1.0f, 0.78f, 0.25f, 0.55f);
        for (int i = 0; i < 12; i++) {
            float a = 2.0f * PI * i / 12.0f;
            drawLine(0.00f + 0.10f * cosf(a), 0.78f + 0.10f * sinf(a),
                     0.00f + 0.16f * cosf(a), 0.78f + 0.16f * sinf(a), 2.2f);
        }
    } else {
        drawVerticalGradient(-1.0f, 0.12f, 1.0f, 1.0f, 10, 16, 34, 36, 48, 78);

        // Moon (night only)
        glColor3ub(238, 238, 220);
        drawCircle(0.00f, 0.78f, 0.080f);
        glColor3ub(28, 38, 66);
        drawCircle(0.03f, 0.80f, 0.064f); // crescent cutout
        glColor4f(0.95f, 0.95f, 0.80f, 0.28f);
        drawCircle(0.00f, 0.78f, 0.14f);

        glColor3ub(240, 240, 220);
        drawCircle(-0.75f, 0.82f, 0.008f);
        drawCircle(-0.55f, 0.70f, 0.006f);
        drawCircle(-0.20f, 0.88f, 0.007f);
        drawCircle(0.15f, 0.83f, 0.006f);
        drawCircle(0.42f, 0.90f, 0.007f);
        drawCircle(0.85f, 0.86f, 0.006f);
    }
}

void drawCloud(float x, float y, float s) {
    glColor3ub(250, 250, 250);
    drawCircle(x, y, 0.06f * s);
    drawCircle(x + 0.06f * s, y + 0.02f * s, 0.055f * s);
    drawCircle(x - 0.06f * s, y + 0.02f * s, 0.055f * s);
    drawCircle(x + 0.02f * s, y + 0.05f * s, 0.05f * s);

    // Cloud underside shadow
    glColor4f(0.78f, 0.82f, 0.88f, 0.35f);
    drawEllipse(x + 0.01f * s, y - 0.01f * s, 0.10f * s, 0.028f * s);
}

void drawRain() {
    if (!isRaining) {
        return;
    }

    glColor4f(0.82f, 0.90f, 0.98f, 0.72f);
    for (int i = 0; i < 560; i++) {
        float nx = -1.08f + 2.16f * ((i % 67) / 66.0f);
        float nyBase = 1.10f - 2.35f * (((i * 11) % 71) / 70.0f);
        float fall = fmodf(rainPhase + (i % 13) * 0.07f, 2.2f);
        float yTop = nyBase - fall;
        if (yTop < -1.02f || yTop > 1.10f) {
            continue;
        }

        float xDrift = 0.012f + 0.008f * ((i % 5) / 4.0f);
        float dropLen = 0.040f + 0.026f * ((i % 7) / 6.0f);
        drawLine(nx, yTop, nx + xDrift, yTop - dropLen, 1.8f);
    }

    // Secondary faint layer adds depth and stronger storm feel.
    glColor4f(0.74f, 0.84f, 0.95f, 0.42f);
    for (int i = 0; i < 320; i++) {
        float nx = -1.06f + 2.12f * ((i % 59) / 58.0f);
        float nyBase = 1.08f - 2.30f * (((i * 7) % 67) / 66.0f);
        float fall = fmodf(rainPhase * 1.07f + (i % 17) * 0.05f, 2.3f);
        float yTop = nyBase - fall;
        if (yTop < -1.02f || yTop > 1.08f) {
            continue;
        }
        drawLine(nx, yTop, nx + 0.010f, yTop - 0.034f, 1.0f);
    }

    // Ground splash streaks.
    glColor4f(0.84f, 0.92f, 1.0f, 0.38f);
    for (int i = 0; i < 110; i++) {
        float sx = -0.96f + 1.92f * ((i % 55) / 54.0f);
        float sy = -0.89f + 0.08f * ((i * 3) % 17) / 16.0f;
        float w = 0.010f + 0.010f * ((i % 6) / 5.0f);
        drawLine(sx - w, sy, sx + w, sy, 1.0f);
    }

    // Soft wet haze during rain
    glBegin(GL_QUADS);
    glColor4f(0.56f, 0.66f, 0.78f, 0.10f);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(1.0f, -1.0f);
    glColor4f(0.66f, 0.78f, 0.90f, 0.16f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();
}

void drawGround() {
    drawVerticalGradient(-1.0f, -1.0f, 1.0f, 0.12f, 188, 170, 146, 214, 200, 178);
    drawSoilTexture();
}

void drawBigTree(float x, float y, float s) {
    // Tapered trunk
    glColor3ub(82, 48, 25);
    glBegin(GL_POLYGON);
    glVertex2f(x - 0.060f * s, y);
    glVertex2f(x + 0.060f * s, y);
    glVertex2f(x + 0.046f * s, y + 0.18f * s);
    glVertex2f(x + 0.034f * s, y + 0.33f * s);
    glVertex2f(x + 0.020f * s, y + 0.45f * s);
    glVertex2f(x - 0.020f * s, y + 0.45f * s);
    glVertex2f(x - 0.034f * s, y + 0.33f * s);
    glVertex2f(x - 0.046f * s, y + 0.18f * s);
    glEnd();

    // Bark highlight strip
    glColor3ub(108, 72, 46);
    glBegin(GL_QUADS);
    glVertex2f(x - 0.012f * s, y + 0.03f * s);
    glVertex2f(x + 0.004f * s, y + 0.03f * s);
    glVertex2f(x - 0.003f * s, y + 0.40f * s);
    glVertex2f(x - 0.016f * s, y + 0.40f * s);
    glEnd();

    // Major branches
    drawLine(x, y + 0.25f * s, x - 0.18f * s, y + 0.44f * s, 2.8f);
    drawLine(x, y + 0.25f * s, x + 0.18f * s, y + 0.44f * s, 2.8f);
    drawLine(x, y + 0.31f * s, x - 0.24f * s, y + 0.52f * s, 2.4f);
    drawLine(x, y + 0.31f * s, x + 0.24f * s, y + 0.52f * s, 2.4f);
    drawLine(x, y + 0.35f * s, x - 0.10f * s, y + 0.54f * s, 2.2f);
    drawLine(x, y + 0.35f * s, x + 0.10f * s, y + 0.54f * s, 2.2f);

    // Secondary branch network for complexity
    drawLine(x - 0.18f * s, y + 0.44f * s, x - 0.26f * s, y + 0.55f * s, 1.7f);
    drawLine(x - 0.18f * s, y + 0.44f * s, x - 0.10f * s, y + 0.57f * s, 1.7f);
    drawLine(x + 0.18f * s, y + 0.44f * s, x + 0.26f * s, y + 0.55f * s, 1.7f);
    drawLine(x + 0.18f * s, y + 0.44f * s, x + 0.10f * s, y + 0.57f * s, 1.7f);
    drawLine(x - 0.24f * s, y + 0.52f * s, x - 0.31f * s, y + 0.59f * s, 1.5f);
    drawLine(x + 0.24f * s, y + 0.52f * s, x + 0.31f * s, y + 0.59f * s, 1.5f);
    drawLine(x - 0.10f * s, y + 0.54f * s, x - 0.04f * s, y + 0.62f * s, 1.4f);
    drawLine(x + 0.10f * s, y + 0.54f * s, x + 0.04f * s, y + 0.62f * s, 1.4f);
    drawLine(x - 0.26f * s, y + 0.55f * s, x - 0.33f * s, y + 0.63f * s, 1.2f);
    drawLine(x + 0.26f * s, y + 0.55f * s, x + 0.33f * s, y + 0.63f * s, 1.2f);
    drawLine(x - 0.10f * s, y + 0.57f * s, x - 0.16f * s, y + 0.66f * s, 1.1f);
    drawLine(x + 0.10f * s, y + 0.57f * s, x + 0.16f * s, y + 0.66f * s, 1.1f);
    drawLine(x - 0.04f * s, y + 0.62f * s, x - 0.09f * s, y + 0.69f * s, 1.0f);
    drawLine(x + 0.04f * s, y + 0.62f * s, x + 0.09f * s, y + 0.69f * s, 1.0f);

    // Roots
    drawLine(x - 0.01f * s, y, x - 0.09f * s, y - 0.04f * s, 2.0f);
    drawLine(x + 0.01f * s, y, x + 0.09f * s, y - 0.04f * s, 2.0f);
    drawLine(x - 0.03f * s, y + 0.01f * s, x - 0.12f * s, y - 0.02f * s, 1.4f);
    drawLine(x + 0.03f * s, y + 0.01f * s, x + 0.12f * s, y - 0.02f * s, 1.4f);

    // Canopy (multi-cluster with sharper irregular silhouette)
    glColor3ub(48, 140, 58);
    drawCircle(x, y + 0.60f * s, 0.19f * s);
    drawCircle(x - 0.20f * s, y + 0.56f * s, 0.13f * s);
    drawCircle(x + 0.20f * s, y + 0.56f * s, 0.13f * s);
    drawCircle(x - 0.09f * s, y + 0.67f * s, 0.10f * s);
    drawCircle(x + 0.09f * s, y + 0.67f * s, 0.10f * s);
    drawCircle(x - 0.28f * s, y + 0.52f * s, 0.09f * s);
    drawCircle(x + 0.28f * s, y + 0.52f * s, 0.09f * s);
    drawCircle(x, y + 0.48f * s, 0.12f * s);
    drawCircle(x - 0.33f * s, y + 0.57f * s, 0.06f * s);
    drawCircle(x + 0.33f * s, y + 0.57f * s, 0.06f * s);
    drawCircle(x - 0.22f * s, y + 0.70f * s, 0.055f * s);
    drawCircle(x + 0.22f * s, y + 0.70f * s, 0.055f * s);
    drawCircle(x, y + 0.73f * s, 0.060f * s);

    // Leaf-edge wedges for sharper crown
    glColor3ub(36, 124, 50);
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 0.32f * s, y + 0.60f * s);
    glVertex2f(x - 0.24f * s, y + 0.58f * s);
    glVertex2f(x - 0.28f * s, y + 0.67f * s);
    glVertex2f(x + 0.32f * s, y + 0.60f * s);
    glVertex2f(x + 0.24f * s, y + 0.58f * s);
    glVertex2f(x + 0.28f * s, y + 0.67f * s);
    glVertex2f(x - 0.08f * s, y + 0.73f * s);
    glVertex2f(x, y + 0.67f * s);
    glVertex2f(x + 0.08f * s, y + 0.73f * s);
    glEnd();

    // Foliage texture speckles
    glColor3ub(34, 118, 46);
    drawCircle(x - 0.08f * s, y + 0.62f * s, 0.020f * s);
    drawCircle(x + 0.07f * s, y + 0.60f * s, 0.018f * s);
    drawCircle(x - 0.15f * s, y + 0.50f * s, 0.017f * s);
    drawCircle(x + 0.14f * s, y + 0.51f * s, 0.016f * s);
    drawCircle(x - 0.22f * s, y + 0.58f * s, 0.014f * s);
    drawCircle(x + 0.22f * s, y + 0.58f * s, 0.014f * s);
    drawCircle(x - 0.03f * s, y + 0.70f * s, 0.013f * s);
    drawCircle(x + 0.05f * s, y + 0.68f * s, 0.012f * s);
    drawCircle(x - 0.28f * s, y + 0.61f * s, 0.012f * s);
    drawCircle(x + 0.28f * s, y + 0.61f * s, 0.012f * s);

    // Fine twig tips visible through canopy
    glColor3ub(62, 36, 18);
    drawLine(x - 0.29f * s, y + 0.59f * s, x - 0.34f * s, y + 0.66f * s, 0.9f);
    drawLine(x + 0.29f * s, y + 0.59f * s, x + 0.34f * s, y + 0.66f * s, 0.9f);
    drawLine(x - 0.07f * s, y + 0.66f * s, x - 0.11f * s, y + 0.73f * s, 0.8f);
    drawLine(x + 0.07f * s, y + 0.66f * s, x + 0.11f * s, y + 0.73f * s, 0.8f);

    // Bark texture lines
    glColor3ub(70, 40, 20);
    drawLine(x - 0.015f * s, y + 0.05f * s, x - 0.020f * s, y + 0.40f * s, 1.2f);
    drawLine(x + 0.010f * s, y + 0.07f * s, x + 0.016f * s, y + 0.39f * s, 1.1f);
    drawLine(x - 0.032f * s, y + 0.12f * s, x - 0.028f * s, y + 0.31f * s, 1.0f);
    drawLine(x + 0.030f * s, y + 0.14f * s, x + 0.025f * s, y + 0.30f * s, 1.0f);
}

void drawConiferTree(float x, float y, float s) {
    // Natural conifer trunk with bark tint
    glColor3ub(82, 48, 25);
    glBegin(GL_POLYGON);
    glVertex2f(x - 0.030f * s, y);
    glVertex2f(x + 0.030f * s, y);
    glVertex2f(x + 0.022f * s, y + 0.11f * s);
    glVertex2f(x - 0.022f * s, y + 0.11f * s);
    glEnd();
    glColor3ub(106, 68, 43);
    drawLine(x - 0.008f * s, y + 0.01f * s, x - 0.010f * s, y + 0.10f * s, 1.1f);

    // Conifer foliage layers (top to bottom) with irregular edges
    glColor3ub(30, 108, 44);
    glBegin(GL_TRIANGLES);
    glVertex2f(x, y + 0.56f * s);
    glVertex2f(x - 0.09f * s, y + 0.36f * s);
    glVertex2f(x + 0.09f * s, y + 0.36f * s);
    glEnd();
    drawCircle(x - 0.045f * s, y + 0.43f * s, 0.020f * s);
    drawCircle(x + 0.045f * s, y + 0.44f * s, 0.018f * s);

    glColor3ub(34, 118, 47);
    glBegin(GL_TRIANGLES);
    glVertex2f(x, y + 0.46f * s);
    glVertex2f(x - 0.14f * s, y + 0.25f * s);
    glVertex2f(x + 0.14f * s, y + 0.25f * s);
    glEnd();
    drawCircle(x - 0.070f * s, y + 0.33f * s, 0.022f * s);
    drawCircle(x + 0.072f * s, y + 0.34f * s, 0.020f * s);

    glColor3ub(42, 132, 53);
    glBegin(GL_TRIANGLES);
    glVertex2f(x, y + 0.36f * s);
    glVertex2f(x - 0.19f * s, y + 0.14f * s);
    glVertex2f(x + 0.19f * s, y + 0.14f * s);
    glEnd();
    drawCircle(x - 0.10f * s, y + 0.23f * s, 0.024f * s);
    drawCircle(x + 0.11f * s, y + 0.24f * s, 0.023f * s);

    glColor3ub(50, 145, 60);
    glBegin(GL_TRIANGLES);
    glVertex2f(x, y + 0.28f * s);
    glVertex2f(x - 0.22f * s, y + 0.07f * s);
    glVertex2f(x + 0.22f * s, y + 0.07f * s);
    glEnd();
    drawCircle(x - 0.13f * s, y + 0.15f * s, 0.026f * s);
    drawCircle(x + 0.14f * s, y + 0.16f * s, 0.025f * s);

    // Needle texture lines and twig detail
    glColor3ub(28, 102, 40);
    for (int i = 0; i < 14; i++) {
        float yy = y + 0.09f * s + 0.034f * s * i;
        float halfW = 0.21f * s - 0.0125f * s * i;
        if (halfW < 0.03f * s) {
            halfW = 0.03f * s;
        }
        drawLine(x - halfW, yy, x + halfW, yy, 1.2f);
        drawLine(x - halfW * 0.65f, yy + 0.01f * s, x - halfW * 0.35f, yy - 0.005f * s, 0.9f);
        drawLine(x + halfW * 0.35f, yy - 0.005f * s, x + halfW * 0.65f, yy + 0.01f * s, 0.9f);
    }

    // Edge accents for crisp silhouette
    glColor3ub(20, 88, 34);
    drawLine(x, y + 0.50f * s, x - 0.12f * s, y + 0.30f * s, 1.5f);
    drawLine(x, y + 0.50f * s, x + 0.12f * s, y + 0.30f * s, 1.5f);
    drawLine(x, y + 0.38f * s, x - 0.16f * s, y + 0.18f * s, 1.5f);
    drawLine(x, y + 0.38f * s, x + 0.16f * s, y + 0.18f * s, 1.5f);
    drawLine(x, y + 0.28f * s, x - 0.20f * s, y + 0.08f * s, 1.5f);
    drawLine(x, y + 0.28f * s, x + 0.20f * s, y + 0.08f * s, 1.5f);

    // Natural foliage depth patches (no ornaments/star)
    glColor3ub(22, 86, 34);
    drawCircle(x - 0.08f * s, y + 0.22f * s, 0.015f * s);
    drawCircle(x + 0.09f * s, y + 0.26f * s, 0.014f * s);
    drawCircle(x - 0.05f * s, y + 0.40f * s, 0.012f * s);
    drawCircle(x + 0.04f * s, y + 0.46f * s, 0.010f * s);
}

void drawSmallBackgroundTrees() {
    // Two-layer distant woodland for depth and realism.
    for (int layer = 0; layer < 2; layer++) {
        float depth = (layer == 0) ? 0.76f : 1.0f;
        float alpha = (layer == 0) ? 0.70f : 0.94f;
        float yLift = (layer == 0) ? 0.022f : 0.0f;
        float xShift = (layer == 0) ? 0.035f : 0.0f;
        int count = (layer == 0) ? 18 : 22;

        for (int i = 0; i < count; i++) {
            float x = -1.02f + i * (2.06f / (count - 1)) + xShift;
            float y = 0.118f + yLift + 0.010f * sinf(i * 0.68f + layer * 0.9f);
            float h = (0.11f + 0.06f * ((i + 2 * layer) % 6) / 5.0f) * depth;
            float lean = 0.005f * sinf(i * 1.53f + layer * 0.6f);

            // Low-contrast contact shadow to anchor each tree.
            glColor4f(0.08f, 0.08f, 0.08f, 0.10f * alpha);
            drawEllipse(x + 0.003f * depth, y - 0.030f * depth, 0.022f * depth, 0.0045f * depth);

            // Thin tapered trunk with muted bark tones.
            glColor4f(0.33f, 0.22f, 0.14f, 0.92f * alpha);
            glBegin(GL_POLYGON);
            glVertex2f(x - 0.0048f * depth, y - 0.028f * depth);
            glVertex2f(x + 0.0048f * depth, y - 0.028f * depth);
            glVertex2f(x + 0.0040f * depth + lean, y - 0.010f * depth);
            glVertex2f(x + 0.0030f * depth + lean, y + 0.018f * depth);
            glVertex2f(x - 0.0030f * depth + lean, y + 0.018f * depth);
            glVertex2f(x - 0.0040f * depth + lean, y - 0.010f * depth);
            glEnd();

            glColor4f(0.44f, 0.30f, 0.18f, 0.62f * alpha);
            drawLine(x - 0.001f * depth, y - 0.024f * depth,
                     x - 0.001f * depth + lean, y + 0.016f * depth, 0.6f);

            // Fine branch hints under crowns.
            glColor4f(0.26f, 0.17f, 0.10f, 0.50f * alpha);
            drawLine(x + lean, y + 0.002f * depth, x - 0.010f * depth + lean, y + 0.030f * depth, 0.55f);
            drawLine(x + lean, y + 0.003f * depth, x + 0.010f * depth + lean, y + 0.028f * depth, 0.55f);

            // Crown collar: bridge between trunk tip and foliage mass.
            glColor4f(0.16f, 0.30f, 0.19f, 0.78f * alpha);
            drawCircle(x + lean * 0.6f, y + 0.030f * depth, 0.009f * depth);
            drawCircle(x - 0.008f * depth + lean * 0.6f, y + 0.032f * depth, 0.006f * depth);
            drawCircle(x + 0.008f * depth + lean * 0.6f, y + 0.031f * depth, 0.006f * depth);

            bool isConifer = ((i + layer) % 4 == 1);
            if (isConifer) {
                // Multi-tier conifer with softened silhouette.
                glColor4f(0.17f, 0.36f, 0.22f, 0.95f * alpha);
                glBegin(GL_TRIANGLES);
                glVertex2f(x, y + h * 0.96f);
                glVertex2f(x - 0.020f * depth, y + h * 0.68f);
                glVertex2f(x + 0.020f * depth, y + h * 0.68f);
                glEnd();

                glColor4f(0.15f, 0.32f, 0.21f, 0.95f * alpha);
                glBegin(GL_TRIANGLES);
                glVertex2f(x, y + h * 0.82f);
                glVertex2f(x - 0.027f * depth, y + h * 0.50f);
                glVertex2f(x + 0.027f * depth, y + h * 0.50f);
                glEnd();

                glColor4f(0.13f, 0.28f, 0.19f, 0.95f * alpha);
                glBegin(GL_TRIANGLES);
                glVertex2f(x, y + h * 0.66f);
                glVertex2f(x - 0.033f * depth, y + h * 0.30f);
                glVertex2f(x + 0.033f * depth, y + h * 0.30f);
                glEnd();

                glColor4f(0.11f, 0.24f, 0.16f, 0.85f * alpha);
                glBegin(GL_TRIANGLES);
                glVertex2f(x, y + h * 0.54f);
                glVertex2f(x - 0.037f * depth, y + h * 0.20f);
                glVertex2f(x + 0.037f * depth, y + h * 0.20f);
                glEnd();

                // Lower connector tier so foliage visibly starts from trunk.
                glColor4f(0.12f, 0.26f, 0.17f, 0.90f * alpha);
                glBegin(GL_TRIANGLES);
                glVertex2f(x, y + h * 0.42f);
                glVertex2f(x - 0.026f * depth, y + h * 0.16f);
                glVertex2f(x + 0.026f * depth, y + h * 0.16f);
                glEnd();

                // Needle bands and tiny edge twigs.
                glColor4f(0.10f, 0.20f, 0.14f, 0.62f * alpha);
                drawLine(x - 0.018f * depth, y + h * 0.74f, x + 0.018f * depth, y + h * 0.74f, 0.65f);
                drawLine(x - 0.022f * depth, y + h * 0.58f, x + 0.022f * depth, y + h * 0.58f, 0.65f);
                drawLine(x - 0.026f * depth, y + h * 0.43f, x + 0.026f * depth, y + h * 0.43f, 0.65f);
            } else {
                // Irregular deciduous crown with layered clumps.
                float crownY = y + h * 0.48f;
                glColor4f(0.22f, 0.42f, 0.25f, 0.96f * alpha);
                drawCircle(x, crownY, 0.026f * depth);
                drawCircle(x - 0.018f * depth, crownY - 0.006f * depth, 0.020f * depth);
                drawCircle(x + 0.018f * depth, crownY - 0.007f * depth, 0.020f * depth);

                // Base tuft to connect trunk top and main crown.
                glColor4f(0.18f, 0.34f, 0.22f, 0.88f * alpha);
                drawCircle(x + lean * 0.5f, crownY - 0.018f * depth, 0.012f * depth);
                drawCircle(x - 0.010f * depth + lean * 0.5f, crownY - 0.016f * depth, 0.009f * depth);
                drawCircle(x + 0.010f * depth + lean * 0.5f, crownY - 0.017f * depth, 0.009f * depth);

                glColor4f(0.18f, 0.36f, 0.23f, 0.94f * alpha);
                drawCircle(x - 0.026f * depth, crownY + 0.002f * depth, 0.013f * depth);
                drawCircle(x + 0.025f * depth, crownY + 0.006f * depth, 0.013f * depth);
                drawCircle(x, crownY + 0.017f * depth, 0.015f * depth);

                glColor4f(0.13f, 0.28f, 0.17f, 0.78f * alpha);
                drawCircle(x - 0.009f * depth, crownY + 0.009f * depth, 0.008f * depth);
                drawCircle(x + 0.010f * depth, crownY + 0.003f * depth, 0.008f * depth);

                // Uneven silhouette chips for non-round crown outline.
                glBegin(GL_TRIANGLES);
                glVertex2f(x - 0.031f * depth, crownY - 0.002f * depth);
                glVertex2f(x - 0.020f * depth, crownY - 0.006f * depth);
                glVertex2f(x - 0.024f * depth, crownY + 0.015f * depth);

                glVertex2f(x + 0.031f * depth, crownY - 0.002f * depth);
                glVertex2f(x + 0.020f * depth, crownY - 0.006f * depth);
                glVertex2f(x + 0.024f * depth, crownY + 0.015f * depth);
                glEnd();
            }
        }
    }
}

void drawBenchIron(float x, float y, float s) {
    float halfWidth = 0.19f * s;
    float seatY0 = y;
    float seatY1 = y + 0.038f * s;
    float backY0 = y + 0.082f * s;
    float backY1 = y + 0.176f * s;
    float footY = y - 0.075f * s;

    // Grounded shadows for the full body and each leg.
    glColor4f(0.08f, 0.08f, 0.08f, 0.22f);
    drawEllipse(x + 0.002f * s, footY - 0.002f * s, 0.232f * s, 0.032f * s);
    glColor4f(0.05f, 0.05f, 0.05f, 0.18f);
    drawEllipse(x - 0.135f * s, footY - 0.001f * s, 0.036f * s, 0.010f * s);
    drawEllipse(x + 0.135f * s, footY - 0.001f * s, 0.036f * s, 0.010f * s);

    // Cast-iron side frames.
    glColor3ub(34, 36, 40);
    glBegin(GL_POLYGON);
    glVertex2f(x - 0.153f * s, footY);
    glVertex2f(x - 0.108f * s, footY);
    glVertex2f(x - 0.100f * s, y + 0.006f * s);
    glVertex2f(x - 0.106f * s, y + 0.082f * s);
    glVertex2f(x - 0.111f * s, y + 0.150f * s);
    glVertex2f(x - 0.145f * s, y + 0.150f * s);
    glVertex2f(x - 0.153f * s, y + 0.092f * s);
    glVertex2f(x - 0.160f * s, y + 0.008f * s);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(x + 0.108f * s, footY);
    glVertex2f(x + 0.153f * s, footY);
    glVertex2f(x + 0.160f * s, y + 0.008f * s);
    glVertex2f(x + 0.153f * s, y + 0.092f * s);
    glVertex2f(x + 0.145f * s, y + 0.150f * s);
    glVertex2f(x + 0.111f * s, y + 0.150f * s);
    glVertex2f(x + 0.106f * s, y + 0.082f * s);
    glVertex2f(x + 0.100f * s, y + 0.006f * s);
    glEnd();

    // Side feet pads to remove any floating look.
    glColor3ub(27, 29, 32);
    glBegin(GL_QUADS);
    glVertex2f(x - 0.156f * s, footY - 0.006f * s);
    glVertex2f(x - 0.104f * s, footY - 0.006f * s);
    glVertex2f(x - 0.104f * s, footY + 0.004f * s);
    glVertex2f(x - 0.156f * s, footY + 0.004f * s);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(x + 0.104f * s, footY - 0.006f * s);
    glVertex2f(x + 0.156f * s, footY - 0.006f * s);
    glVertex2f(x + 0.156f * s, footY + 0.004f * s);
    glVertex2f(x + 0.104f * s, footY + 0.004f * s);
    glEnd();

    // Decorative iron cutouts.
    glColor3ub(18, 20, 23);
    drawEllipse(x - 0.129f * s, y + 0.048f * s, 0.012f * s, 0.019f * s);
    drawEllipse(x + 0.129f * s, y + 0.048f * s, 0.012f * s, 0.019f * s);
    drawArc(x - 0.129f * s, y + 0.096f * s, 0.016f * s, 0.10f * PI, 0.90f * PI, 1.6f);
    drawArc(x + 0.129f * s, y + 0.096f * s, 0.016f * s, 0.10f * PI, 0.90f * PI, 1.6f);

    // Frame rails and center support.
    glColor3ub(30, 32, 36);
    glBegin(GL_QUADS);
    glVertex2f(x - 0.136f * s, y - 0.011f * s);
    glVertex2f(x + 0.136f * s, y - 0.011f * s);
    glVertex2f(x + 0.136f * s, y + 0.004f * s);
    glVertex2f(x - 0.136f * s, y + 0.004f * s);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(x - 0.133f * s, y + 0.070f * s);
    glVertex2f(x + 0.133f * s, y + 0.070f * s);
    glVertex2f(x + 0.133f * s, y + 0.083f * s);
    glVertex2f(x - 0.133f * s, y + 0.083f * s);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(x - 0.006f * s, y - 0.011f * s);
    glVertex2f(x + 0.006f * s, y - 0.011f * s);
    glVertex2f(x + 0.006f * s, y + 0.083f * s);
    glVertex2f(x - 0.006f * s, y + 0.083f * s);
    glEnd();

    // Seat slats with subtle tone variation.
    for (int i = 0; i < 5; i++) {
        float y0 = seatY0 + 0.002f * s + i * 0.0072f * s;
        float y1 = y0 + 0.0052f * s;
        unsigned char r = (unsigned char)(120 + i * 7);
        unsigned char g = (unsigned char)(76 + i * 4);
        unsigned char b = (unsigned char)(44 + i * 3);

        glColor3ub(r, g, b);
        glBegin(GL_QUADS);
        glVertex2f(x - halfWidth, y0);
        glVertex2f(x + halfWidth, y0);
        glVertex2f(x + halfWidth, y1);
        glVertex2f(x - halfWidth, y1);
        glEnd();

        glColor4f(0.90f, 0.76f, 0.53f, 0.22f);
        drawLine(x - halfWidth, y1, x + halfWidth, y1, 0.8f);
        glColor4f(0.20f, 0.14f, 0.10f, 0.22f);
        drawLine(x - halfWidth, y0, x + halfWidth, y0, 0.8f);
    }

    // Backrest slats.
    for (int i = 0; i < 5; i++) {
        float y0 = backY0 + i * 0.0175f * s;
        float y1 = y0 + 0.0125f * s;
        unsigned char r = (unsigned char)(115 + i * 6);
        unsigned char g = (unsigned char)(73 + i * 4);
        unsigned char b = (unsigned char)(42 + i * 3);

        glColor3ub(r, g, b);
        glBegin(GL_QUADS);
        glVertex2f(x - halfWidth, y0);
        glVertex2f(x + halfWidth, y0);
        glVertex2f(x + halfWidth, y1);
        glVertex2f(x - halfWidth, y1);
        glEnd();

        glColor4f(0.88f, 0.73f, 0.50f, 0.20f);
        drawLine(x - halfWidth, y1, x + halfWidth, y1, 0.7f);
    }

    // Wood grain and weathering streaks.
    glColor4f(0.28f, 0.18f, 0.11f, 0.26f);
    for (int i = 0; i < 7; i++) {
        float gx = x - 0.16f * s + i * 0.053f * s;
        drawLine(gx, seatY0 + 0.004f * s, gx + 0.008f * s, seatY1 - 0.004f * s, 0.6f);
        drawLine(gx - 0.003f * s, backY0 + 0.009f * s, gx + 0.006f * s, backY1 - 0.006f * s, 0.6f);
    }

    // Curved armrests and posts.
    glColor3ub(31, 33, 37);
    drawArc(x - 0.167f * s, y + 0.088f * s, 0.035f * s, 1.05f * PI, 1.90f * PI, 2.0f);
    drawArc(x + 0.167f * s, y + 0.088f * s, 0.035f * s, 1.10f * PI, 1.95f * PI, 2.0f);
    drawLine(x - 0.164f * s, y + 0.049f * s, x - 0.164f * s, y + 0.124f * s, 2.0f);
    drawLine(x + 0.164f * s, y + 0.049f * s, x + 0.164f * s, y + 0.124f * s, 2.0f);

    // Uprights for backrest.
    glColor3ub(36, 38, 43);
    glBegin(GL_QUADS);
    glVertex2f(x - 0.123f * s, y + 0.030f * s);
    glVertex2f(x - 0.111f * s, y + 0.030f * s);
    glVertex2f(x - 0.111f * s, backY1);
    glVertex2f(x - 0.123f * s, backY1);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(x + 0.111f * s, y + 0.030f * s);
    glVertex2f(x + 0.123f * s, y + 0.030f * s);
    glVertex2f(x + 0.123f * s, backY1);
    glVertex2f(x + 0.111f * s, backY1);
    glEnd();

    // Rivets and minor imperfections.
    glColor3ub(118, 124, 130);
    drawCircle(x - 0.116f * s, y + 0.077f * s, 0.0040f * s, 24);
    drawCircle(x + 0.116f * s, y + 0.077f * s, 0.0040f * s, 24);
    drawCircle(x - 0.116f * s, y + 0.136f * s, 0.0038f * s, 24);
    drawCircle(x + 0.116f * s, y + 0.136f * s, 0.0038f * s, 24);
    drawCircle(x, y + 0.076f * s, 0.0034f * s, 24);
    drawCircle(x, y + 0.136f * s, 0.0032f * s, 24);

    glColor4f(0.42f, 0.30f, 0.18f, 0.22f);
    drawLine(x - 0.150f * s, y - 0.010f * s, x - 0.146f * s, y + 0.030f * s, 0.8f);
    drawLine(x + 0.150f * s, y - 0.005f * s, x + 0.145f * s, y + 0.030f * s, 0.8f);

    // Metallic highlights and underside occlusion.
    glColor4f(0.80f, 0.84f, 0.90f, 0.22f);
    drawLine(x - 0.133f * s, y + 0.083f * s, x + 0.133f * s, y + 0.083f * s, 0.9f);
    glColor4f(0.03f, 0.03f, 0.03f, 0.24f);
    drawLine(x - 0.142f * s, y - 0.002f * s, x + 0.142f * s, y - 0.002f * s, 0.9f);
}

void drawBird(float x, float y, float s) {
    float flap = 0.025f * sinf(birdFlapPhase + x * 2.1f);

    // Main body as a compact aerodynamic shape.
    glColor3ub(18, 18, 18);
    drawEllipse(x, y, 0.019f * s, 0.010f * s, 54);

    // Neck-head profile.
    drawEllipse(x + 0.017f * s, y + 0.003f * s, 0.007f * s, 0.006f * s, 40);

    // Beak (pointed for sharper realism).
    glColor3ub(225, 164, 58);
    glBegin(GL_TRIANGLES);
    glVertex2f(x + 0.022f * s, y + 0.003f * s);
    glVertex2f(x + 0.039f * s, y + 0.007f * s);
    glVertex2f(x + 0.022f * s, y - 0.001f * s);
    glEnd();

    // Split tail with a sharp fork.
    glColor3ub(16, 16, 16);
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 0.017f * s, y + 0.002f * s);
    glVertex2f(x - 0.040f * s, y + 0.013f * s);
    glVertex2f(x - 0.030f * s, y + 0.000f * s);
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 0.017f * s, y - 0.001f * s);
    glVertex2f(x - 0.040f * s, y - 0.014f * s);
    glVertex2f(x - 0.030f * s, y - 0.003f * s);
    glEnd();

    // Wings: pointed primary feathers and inner shoulder mass.
    glColor3ub(22, 22, 22);
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 0.002f * s, y + 0.003f * s);
    glVertex2f(x - 0.072f * s, y + (0.060f + flap) * s);
    glVertex2f(x - 0.018f * s, y + (0.015f + flap * 0.35f) * s);
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex2f(x + 0.002f * s, y + 0.003f * s);
    glVertex2f(x + 0.072f * s, y + (0.060f + flap) * s);
    glVertex2f(x + 0.018f * s, y + (0.015f + flap * 0.35f) * s);
    glEnd();

    glColor3ub(28, 28, 28);
    drawEllipse(x - 0.010f * s, y + (0.010f + flap * 0.25f) * s, 0.015f * s, 0.008f * s, 36);
    drawEllipse(x + 0.010f * s, y + (0.010f + flap * 0.25f) * s, 0.015f * s, 0.008f * s, 36);

    // Feather cuts near wing tips for a crisp silhouette.
    glColor4f(0.05f, 0.05f, 0.05f, 0.55f);
    drawLine(x - 0.040f * s, y + (0.032f + flap * 0.7f) * s,
             x - 0.071f * s, y + (0.060f + flap) * s, 0.9f);
    drawLine(x + 0.040f * s, y + (0.032f + flap * 0.7f) * s,
             x + 0.071f * s, y + (0.060f + flap) * s, 0.9f);

    // Eye + tiny highlight so bird stays legible.
    glColor3ub(0, 0, 0);
    drawCircle(x + 0.019f * s, y + 0.004f * s, 0.0018f * s, 16);
    glColor4f(0.95f, 0.95f, 0.95f, 0.22f);
    drawLine(x - 0.009f * s, y + 0.009f * s, x + 0.014f * s, y + 0.011f * s, 0.9f);
}

void drawLampPost(float x, float y, float s) {
    glColor3ub(20, 20, 20);
    glBegin(GL_QUADS); // stem
    glVertex2f(x - 0.01f * s, y);
    glVertex2f(x + 0.01f * s, y);
    glVertex2f(x + 0.01f * s, y + 0.45f * s);
    glVertex2f(x - 0.01f * s, y + 0.45f * s);
    glEnd();

    // Base
    glColor3ub(35, 35, 35);
    glBegin(GL_QUADS);
    glVertex2f(x - 0.030f * s, y - 0.01f * s);
    glVertex2f(x + 0.030f * s, y - 0.01f * s);
    glVertex2f(x + 0.020f * s, y + 0.03f * s);
    glVertex2f(x - 0.020f * s, y + 0.03f * s);
    glEnd();

    glBegin(GL_QUADS); // head
    glVertex2f(x - 0.045f * s, y + 0.45f * s);
    glVertex2f(x + 0.045f * s, y + 0.45f * s);
    glVertex2f(x + 0.020f * s, y + 0.53f * s);
    glVertex2f(x - 0.020f * s, y + 0.53f * s);
    glEnd();

    // Glass panel effect
    glColor4f(0.75f, 0.83f, 0.90f, 0.22f);
    glBegin(GL_QUADS);
    glVertex2f(x - 0.015f * s, y + 0.465f * s);
    glVertex2f(x + 0.015f * s, y + 0.465f * s);
    glVertex2f(x + 0.010f * s, y + 0.510f * s);
    glVertex2f(x - 0.010f * s, y + 0.510f * s);
    glEnd();

    if (!isDay) {
        glColor3ub(230, 190, 80);
        drawCircle(x, y + 0.49f * s, 0.017f * s);
        glColor4f(0.9f, 0.75f, 0.35f, 0.25f);
        drawCircle(x, y + 0.49f * s, 0.06f * s);
        glColor4f(0.9f, 0.75f, 0.35f, 0.12f);
        drawCircle(x, y + 0.49f * s, 0.11f * s);
    }
}

void drawPond(float x, float y, float s) {
    glColor3ub(145, 145, 150);
    drawEllipse(x, y, 0.40f * s, 0.19f * s);

    glColor3ub(120, 120, 125);
    drawEllipse(x, y, 0.36f * s, 0.16f * s);
    glColor4f(0.75f, 0.75f, 0.78f, 0.35f);
    drawEllipse(x, y + 0.01f * s, 0.35f * s, 0.13f * s);

    glColor3ub(135, 206, 235);
    drawEllipse(x, y, 0.32f * s, 0.13f * s);

    // Water highlight strip
    glColor4f(0.86f, 0.95f, 1.0f, 0.35f);
    drawEllipse(x - 0.03f * s, y + 0.03f * s, 0.20f * s, 0.04f * s);

    glColor3ub(90, 170, 210);
    for (int i = -2; i <= 2; i++) {
        float yy = y + i * 0.03f * s;
        glBegin(GL_LINE_STRIP);
        for (int k = 0; k <= 60; k++) {
            float t = -0.27f + k * (0.54f / 60.0f);
            float wx = x + t * s;
            float wy = yy + 0.006f * sinf(18.0f * t + wavePhase + i);
            glVertex2f(wx, wy);
        }
        glEnd();
    }

    // Two larger lotus blooms for clearer visibility
    glColor3ub(255, 182, 193);
    drawCircle(x - 0.10f * s, y - 0.015f * s, 0.026f * s);
    drawCircle(x + 0.11f * s, y + 0.018f * s, 0.026f * s);

    // Lotus petals
    glColor3ub(255, 168, 186);
    drawEllipse(x - 0.10f * s, y - 0.015f * s, 0.014f * s, 0.028f * s);
    drawEllipse(x + 0.11f * s, y + 0.018f * s, 0.014f * s, 0.028f * s);
    drawEllipse(x - 0.10f * s, y - 0.015f * s, 0.028f * s, 0.014f * s);
    drawEllipse(x + 0.11f * s, y + 0.018f * s, 0.028f * s, 0.014f * s);

    glColor3ub(255, 228, 135);
    drawCircle(x - 0.10f * s, y - 0.015f * s, 0.007f * s);
    drawCircle(x + 0.11f * s, y + 0.018f * s, 0.007f * s);

    glColor3ub(20, 100, 40);
    drawEllipse(x - 0.15f * s, y - 0.035f * s, 0.045f * s, 0.020f * s);
    drawEllipse(x + 0.16f * s, y - 0.012f * s, 0.045f * s, 0.020f * s);

    // Bushy grass-like greenery around pond edge
    glColor3ub(146, 206, 125);
    for (int i = 0; i < 40; i++) {
        float a = 2.0f * PI * i / 40.0f;
        float px = x + 0.43f * s * cosf(a);
        float py = y + 0.21f * s * sinf(a);
        drawCircle(px, py, 0.014f * s);

        glColor3ub(110, 180, 95);
        drawLine(px, py - 0.010f * s, px - 0.010f * s, py + 0.016f * s, 1.2f);
        drawLine(px, py - 0.010f * s, px + 0.010f * s, py + 0.017f * s, 1.2f);
        glColor3ub(146, 206, 125);
    }

    // Water tiny sparkles
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < 36; i++) {
        float a = 2.0f * PI * i / 36.0f;
        float rr = 0.18f * s + 0.07f * s * ((i % 5) / 4.0f);
        float px = x + rr * cosf(a);
        float py = y + 0.05f * s * sinf(a * 1.7f + wavePhase * 0.2f);
        glColor4f(0.88f, 0.97f, 1.0f, 0.35f);
        glVertex2f(px, py);
    }
    glEnd();
}

void drawBorderShrubBand() {
    // Connected border shrubs from light-green to medium-green tones.
    for (int i = 0; i < 30; i++) {
        float x = -0.99f + i * (1.98f / 29.0f);
        float y = 0.105f + 0.012f * sinf(i * 0.55f);
        float r = 0.040f + 0.008f * ((i % 4) / 3.0f);

        glColor3ub(162, 220, 140);
        drawCircle(x - 0.020f, y, r);
        glColor3ub(128, 192, 108);
        drawCircle(x + 0.006f, y + 0.010f, r * 0.95f);
        glColor3ub(98, 168, 84);
        drawCircle(x + 0.030f, y - 0.004f, r * 0.82f);

        // Connector foliage to ensure an unbroken green hedge from distance.
        glColor4f(0.42f, 0.66f, 0.34f, 0.60f);
        drawEllipse(x + 0.018f, y - 0.010f, 0.038f, 0.015f, 40);
    }
}

void drawGroundFoliage() {
    // Wide spread grass tufts to avoid empty foreground.
    for (int i = 0; i < 120; i++) {
        float x = -0.97f + (1.94f * (i % 60) / 59.0f);
        float y = -0.95f + 0.92f * ((i * 7) % 23) / 22.0f;
        if (y > 0.10f) {
            continue;
        }

        float h = 0.022f + 0.020f * ((i % 5) / 4.0f);
        glColor3ub(132, 196, 104);
        drawLine(x, y, x - 0.010f, y + h, 1.1f);
        glColor3ub(98, 166, 80);
        drawLine(x, y, x + 0.010f, y + h * 0.95f, 1.1f);
        glColor3ub(155, 214, 130);
        drawLine(x, y, x, y + h * 1.05f, 1.0f);
    }

    // Ground vines and low shrubs.
    glColor4f(0.32f, 0.58f, 0.26f, 0.56f);
    for (int i = 0; i < 14; i++) {
        float y = -0.84f + i * 0.060f;
        glBegin(GL_LINE_STRIP);
        for (int k = 0; k <= 110; k++) {
            float t = -0.96f + 1.92f * (k / 110.0f);
            float vy = y + 0.010f * sinf(8.0f * t + i * 0.8f);
            glVertex2f(t, vy);
        }
        glEnd();
    }

    for (int i = 0; i < 20; i++) {
        float x = -0.92f + i * (1.84f / 19.0f);
        float y = -0.88f + 0.03f * sinf(i * 1.1f);
        glColor3ub(144, 204, 116);
        drawCircle(x, y, 0.028f);
        glColor3ub(108, 174, 90);
        drawCircle(x + 0.020f, y + 0.010f, 0.022f);
    }
}

void drawFlutterGrassField() {
    // Sharp, layered blades with a gentle wind-driven flutter.
    float wind = wavePhase * 0.9f;

    glColor4f(0.10f, 0.24f, 0.08f, 0.22f);
    drawEllipse(0.00f, -0.90f, 0.95f, 0.10f, 60);

    // Dark base mass so the field feels rooted instead of floating.
    glColor4f(0.06f, 0.16f, 0.05f, 0.18f);
    drawEllipse(0.00f, -0.97f, 0.96f, 0.05f, 60);

    for (int i = 0; i < 52; i++) {
        float x = -0.96f + i * (1.92f / 51.0f);
        float baseY = -0.95f + 0.06f * sinf(i * 0.73f);
        float cluster = 3.0f + (i % 3);
        float sway = 0.010f * sinf(wind + i * 0.38f);

        for (int j = 0; j < (int)cluster; j++) {
            float offset = (j - 1.0f) * 0.014f;
            float height = 0.055f + 0.038f * ((i + j) % 5) / 4.0f;
            float lean = sway + 0.004f * cosf(wind * 1.3f + i * 0.21f + j * 0.8f);
            float curl = 0.004f * sinf(wind * 1.7f + i * 0.17f + j * 0.45f);

            float bend = lean * (0.65f + 0.10f * j);
            float midX = x + offset + bend * 0.28f;
            float tipX = x + offset + bend + curl;
            float midY = baseY + height * (0.42f + 0.03f * j);
            float tipY = baseY + height;

            glColor3ub(62 + (i % 4) * 7, 132 + (j % 2) * 16, 50 + (i % 3) * 5);
            drawLine(x + offset, baseY,
                     midX, midY, 1.3f);
            drawLine(midX, midY,
                     tipX, tipY, 1.0f);

            glColor3ub(88 + (i % 4) * 7, 176 + (j % 2) * 10, 72 + (i % 3) * 5);
            drawLine(x + offset + 0.002f, baseY,
                     x + offset + bend * 0.18f + curl * 0.5f, baseY + height * 0.24f, 0.8f);

            // Side blades to make the clump look denser and more organic.
            glColor3ub(92 + (i % 3) * 6, 176, 80 + (j % 3) * 4);
            drawLine(x + offset, baseY,
                     x + offset - 0.010f + lean * 0.15f, baseY + height * 0.30f, 0.9f);
            drawLine(x + offset, baseY,
                     x + offset + 0.012f + lean * 0.20f, baseY + height * 0.34f, 0.9f);

            // Extra blade to create layered overlap and sharper edges.
            glColor3ub(48 + (j % 2) * 8, 118 + (i % 3) * 10, 44 + (i % 2) * 5);
            drawLine(x + offset + 0.005f, baseY,
                     x + offset + bend * 0.14f, baseY + height * 0.18f, 0.7f);

            // Tiny pointed tips for sharper grass detail.
            glColor3ub(120, 200, 104);
            drawLine(tipX - 0.004f, tipY,
                     tipX + 0.004f, tipY + 0.010f, 0.7f);

            if ((i + j) % 4 == 0) {
                glColor3ub(148, 214, 120);
                drawCircle(tipX + 0.002f, tipY + 0.004f, 0.0032f);
            }
        }
    }

    // A few taller blades break up the field silhouette.
    for (int i = 0; i < 18; i++) {
        float x = -0.92f + i * (1.84f / 17.0f);
        float baseY = -0.88f + 0.04f * sinf(i * 1.12f);
        float sway = 0.014f * sinf(wind * 1.4f + i * 0.57f);
        float height = 0.10f + 0.03f * ((i % 4) / 3.0f);

        glColor3ub(108, 182, 92);
        drawLine(x, baseY, x + sway * 0.2f, baseY + height * 0.55f, 1.3f);
        drawLine(x + sway * 0.2f, baseY + height * 0.55f,
                 x + sway, baseY + height, 1.1f);

        glColor3ub(138, 214, 118);
        drawLine(x, baseY, x - 0.008f + sway * 0.12f, baseY + height * 0.40f, 0.8f);
    }

    // Foreground reed-like blades give the field a more complex layered rim.
    for (int i = 0; i < 20; i++) {
        float x = -0.93f + i * (1.86f / 19.0f);
        float baseY = -0.99f + 0.012f * sinf(i * 1.21f);
        float lean = 0.018f * sinf(wind * 1.25f + i * 0.47f);
        float height = 0.11f + 0.04f * ((i % 4) / 3.0f);

        glColor3ub(54, 128, 52);
        drawLine(x, baseY, x + lean * 0.30f, baseY + height * 0.55f, 1.0f);
        drawLine(x + lean * 0.30f, baseY + height * 0.55f,
                 x + lean, baseY + height, 0.8f);
        glColor3ub(86, 178, 76);
        drawLine(x - 0.006f, baseY + 0.004f,
                 x + lean * 0.12f, baseY + height * 0.25f, 0.7f);
    }

    // Fine foreground filaments and seed heads to make the field feel less uniform.
    for (int i = 0; i < 36; i++) {
        float x = -0.95f + i * (1.90f / 35.0f);
        float y = -0.93f + 0.03f * sinf(i * 1.37f + wind * 0.5f);
        float lean = 0.012f * sinf(wind + i * 0.61f);

        glColor3ub(78, 154, 64);
        drawLine(x, y, x + lean * 0.25f, y + 0.05f, 0.7f);
        drawLine(x + 0.004f, y + 0.01f, x + lean * 0.55f, y + 0.08f, 0.6f);

        if (i % 6 == 0) {
            glColor3ub(164, 210, 102);
            drawCircle(x + lean * 0.45f, y + 0.08f, 0.0045f);
            drawLine(x + lean * 0.42f, y + 0.08f, x + lean * 0.48f, y + 0.11f, 0.6f);
        }

        if (i % 5 == 0) {
            glColor4f(0.18f, 0.38f, 0.14f, 0.35f);
            drawEllipse(x + 0.01f, y - 0.01f, 0.015f, 0.006f, 24);
        }
    }
}

void drawVegetationColorGrade() {
    // Localized tint over lower frame so greenery shifts by time of day.
    glBegin(GL_QUADS);
    if (isDay) {
        glColor4f(0.93f, 1.00f, 0.90f, 0.10f);
        glVertex2f(-1.0f, -1.0f);
        glVertex2f(1.0f, -1.0f);
        glColor4f(0.98f, 1.00f, 0.95f, 0.03f);
        glVertex2f(1.0f, 0.22f);
        glVertex2f(-1.0f, 0.22f);
    } else {
        glColor4f(0.16f, 0.24f, 0.17f, 0.14f);
        glVertex2f(-1.0f, -1.0f);
        glVertex2f(1.0f, -1.0f);
        glColor4f(0.12f, 0.20f, 0.15f, 0.05f);
        glVertex2f(1.0f, 0.22f);
        glVertex2f(-1.0f, 0.22f);
    }
    glEnd();
}

void drawBottomCornerBushes() {
    // Left bush mass with deeper layering
    glColor4f(0.08f, 0.10f, 0.08f, 0.24f);
    drawEllipse(-0.84f, -0.93f, 0.18f, 0.045f, 40);

    glColor3ub(82, 48, 25);
    drawLine(-0.84f, -0.91f, -0.84f, -0.77f, 2.4f);
    drawLine(-0.90f, -0.91f, -0.90f, -0.80f, 2.0f);
    drawLine(-0.76f, -0.90f, -0.76f, -0.81f, 1.9f);
    drawLine(-0.84f, -0.84f, -0.93f, -0.75f, 1.4f);
    drawLine(-0.84f, -0.84f, -0.75f, -0.74f, 1.4f);
    drawLine(-0.84f, -0.82f, -0.88f, -0.71f, 1.1f);
    drawLine(-0.84f, -0.82f, -0.80f, -0.70f, 1.1f);

    glColor3ub(58, 144, 66);
    drawCircle(-0.92f, -0.80f, 0.10f);
    drawCircle(-0.84f, -0.87f, 0.11f);
    drawCircle(-0.76f, -0.81f, 0.10f);
    drawCircle(-0.86f, -0.74f, 0.08f);
    drawCircle(-0.73f, -0.74f, 0.07f);
    drawCircle(-0.95f, -0.73f, 0.07f);

    glColor3ub(40, 122, 52);
    drawCircle(-0.84f, -0.81f, 0.08f);
    drawCircle(-0.79f, -0.76f, 0.06f);
    drawCircle(-0.89f, -0.86f, 0.06f);
    drawCircle(-0.74f, -0.85f, 0.055f);
    drawCircle(-0.95f, -0.84f, 0.055f);

    glColor3ub(26, 98, 38);
    drawLine(-0.93f, -0.82f, -0.98f, -0.75f, 0.9f);
    drawLine(-0.74f, -0.83f, -0.69f, -0.74f, 0.9f);
    drawLine(-0.86f, -0.74f, -0.90f, -0.67f, 0.8f);
    drawLine(-0.86f, -0.74f, -0.80f, -0.66f, 0.8f);

    glColor4f(0.72f, 0.92f, 0.60f, 0.34f);
    drawCircle(-0.90f, -0.76f, 0.020f);
    drawCircle(-0.80f, -0.73f, 0.018f);
    drawCircle(-0.77f, -0.80f, 0.016f);

    // Left crown breakup and fine foliage texture.
    glColor3ub(34, 112, 46);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.99f, -0.78f);
    glVertex2f(-0.93f, -0.80f);
    glVertex2f(-0.96f, -0.70f);
    glVertex2f(-0.70f, -0.78f);
    glVertex2f(-0.77f, -0.80f);
    glVertex2f(-0.73f, -0.69f);
    glVertex2f(-0.85f, -0.70f);
    glVertex2f(-0.90f, -0.74f);
    glVertex2f(-0.79f, -0.73f);
    glEnd();

    glColor3ub(48, 136, 58);
    for (int i = 0; i < 26; i++) {
        float lx = -0.97f + 0.26f * ((i * 7) % 23) / 22.0f;
        float ly = -0.88f + 0.19f * ((i * 5) % 19) / 18.0f;
        float r = 0.006f + 0.006f * (i % 5) / 4.0f;
        drawCircle(lx, ly, r, 24);
    }

    glColor4f(0.16f, 0.36f, 0.15f, 0.40f);
    for (int i = 0; i < 9; i++) {
        float vx = -0.96f + i * 0.032f;
        float vy = -0.86f + 0.02f * sinf(i * 1.2f);
        drawEllipse(vx, vy, 0.018f, 0.006f, 20);
    }

    // Right bush mass with mirrored complexity
    glColor4f(0.08f, 0.10f, 0.08f, 0.24f);
    drawEllipse(0.84f, -0.93f, 0.18f, 0.045f, 40);

    glColor3ub(82, 48, 25);
    drawLine(0.84f, -0.91f, 0.84f, -0.79f, 2.3f);
    drawLine(0.92f, -0.91f, 0.92f, -0.82f, 2.0f);
    drawLine(0.74f, -0.90f, 0.74f, -0.81f, 1.9f);
    drawLine(0.84f, -0.84f, 0.93f, -0.75f, 1.4f);
    drawLine(0.84f, -0.84f, 0.75f, -0.74f, 1.4f);
    drawLine(0.84f, -0.82f, 0.88f, -0.71f, 1.1f);
    drawLine(0.84f, -0.82f, 0.80f, -0.70f, 1.1f);

    glColor3ub(58, 144, 66);
    drawCircle(0.92f, -0.81f, 0.10f);
    drawCircle(0.84f, -0.87f, 0.11f);
    drawCircle(0.76f, -0.81f, 0.10f);
    drawCircle(0.86f, -0.75f, 0.08f);
    drawCircle(0.73f, -0.75f, 0.07f);
    drawCircle(0.95f, -0.74f, 0.07f);

    glColor3ub(40, 122, 52);
    drawCircle(0.84f, -0.82f, 0.08f);
    drawCircle(0.79f, -0.77f, 0.06f);
    drawCircle(0.89f, -0.87f, 0.06f);
    drawCircle(0.74f, -0.85f, 0.055f);
    drawCircle(0.95f, -0.85f, 0.055f);

    glColor3ub(26, 98, 38);
    drawLine(0.93f, -0.83f, 0.98f, -0.76f, 0.9f);
    drawLine(0.74f, -0.83f, 0.69f, -0.74f, 0.9f);
    drawLine(0.86f, -0.75f, 0.90f, -0.68f, 0.8f);
    drawLine(0.86f, -0.75f, 0.80f, -0.67f, 0.8f);

    glColor4f(0.72f, 0.92f, 0.60f, 0.34f);
    drawCircle(0.90f, -0.77f, 0.020f);
    drawCircle(0.80f, -0.74f, 0.018f);
    drawCircle(0.77f, -0.81f, 0.016f);

    // Right crown breakup and fine foliage texture.
    glColor3ub(34, 112, 46);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.99f, -0.79f);
    glVertex2f(0.93f, -0.81f);
    glVertex2f(0.96f, -0.71f);
    glVertex2f(0.70f, -0.79f);
    glVertex2f(0.77f, -0.81f);
    glVertex2f(0.73f, -0.70f);
    glVertex2f(0.85f, -0.71f);
    glVertex2f(0.90f, -0.75f);
    glVertex2f(0.79f, -0.74f);
    glEnd();

    glColor3ub(48, 136, 58);
    for (int i = 0; i < 26; i++) {
        float lx = 0.71f + 0.26f * ((i * 7) % 23) / 22.0f;
        float ly = -0.89f + 0.19f * ((i * 5) % 19) / 18.0f;
        float r = 0.006f + 0.006f * (i % 5) / 4.0f;
        drawCircle(lx, ly, r, 24);
    }

    glColor4f(0.16f, 0.36f, 0.15f, 0.40f);
    for (int i = 0; i < 9; i++) {
        float vx = 0.96f - i * 0.032f;
        float vy = -0.87f + 0.02f * sinf(i * 1.2f + 0.8f);
        drawEllipse(vx, vy, 0.018f, 0.006f, 20);
    }
}

void drawSceneToneOverlay() {
    // Stronger day/night contrast for realism
    glBegin(GL_QUADS);
    if (isDay) {
        glColor4f(1.0f, 0.98f, 0.92f, 0.02f);
    } else {
        glColor4f(0.08f, 0.12f, 0.24f, 0.08f);
    }
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();
}

void drawAmbientDepth() {
    // Kept intentionally neutral to avoid darkening
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    drawSkyAndCelestialBody();
    drawGround();
    drawFlutterGrassField();
    drawGroundFoliage();
    drawBorderShrubBand();
    drawSmallBackgroundTrees();

    // Cloud at upper-right area, moving left to right
    drawCloud(cloudX, 0.63f, 1.50f);
    drawCloud(cloudX - 0.50f, 0.67f, 0.95f);

    drawRain();

    // Trees matching sketch layout
    drawBigTree(-0.60f, 0.05f, 1.22f);
    drawConiferTree(0.02f, 0.07f, 1.05f);
    drawBigTree(0.62f, 0.02f, 1.08f);

    // Benches: left foreground and center
    drawBenchIron(-0.72f, 0.09f, 1.16f);
    drawBenchIron(-0.12f, 0.12f, 0.65f);

    // Main pond
    drawPond(-0.08f, -0.40f, 1.18f);
    if (!isDay) {
        // Moon reflection at night only
        glColor4f(0.88f, 0.90f, 0.75f, 0.20f);
        drawEllipse(-0.05f, -0.35f, 0.11f, 0.02f);
    }

    // Lamp near pond right side
    drawLampPost(0.33f, -0.20f, 1.50f);

    // Two flying birds moving right to left
    drawBird(birdX, 0.48f, 1.28f);
    drawBird(birdX + 0.18f, 0.43f, 1.18f);

    drawBottomCornerBushes();
    drawVegetationColorGrade();
    drawSceneToneOverlay();
    drawAmbientDepth();

    glutSwapBuffers();
}

void update(int) {
    const float cloudSpeed = 0.0035f;
    cloudX += cloudSpeed;
    if (cloudX > 1.3f) {
        cloudX = -1.3f;
    }

    float birdSpeed = cloudSpeed * 1.7f;
    birdX -= birdSpeed;
    if (birdX < -1.4f) {
        birdX = 1.4f;
    }

    wavePhase += 0.12f;
    if (wavePhase > 1000.0f) {
        wavePhase = 0.0f;
    }
    birdFlapPhase += 0.20f;
    if (birdFlapPhase > 1000.0f) {
        birdFlapPhase = 0.0f;
    }

    if (isRaining) {
        rainPhase += 0.045f;
        if (rainPhase > 2000.0f) {
            rainPhase = 0.0f;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void keyboard(unsigned char key, int, int) {
    if (key == 'd' || key == 'D') {
        isDay = true;
    }
    if (key == 'n' || key == 'N') {
        isDay = false;
    }
    if (key == 'r' || key == 'R') {
        isRaining = !isRaining;
    }
    glutPostRedisplay();
}

void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1100, 700);
    glutCreateWindow("Park Scene - Day/Night with Animation");

    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}

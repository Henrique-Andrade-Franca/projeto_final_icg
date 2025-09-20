#include <GL/glut.h>
#include <iostream>
#include <cmath>
#include <vector>

// Define STB_IMAGE_IMPLEMENTATION antes de incluir stb_image.h para criar a implementação
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


// Variáveis de controle da câmera
float camX = 0.0f, camY = 25.0f, camZ = 100.0f;
float camYaw = -90.0f; // Rotação em torno do eixo Y
float camPitch = -15.0f; // Rotação em torno do eixo X

// Variáveis de controle da animação
float animationSpeed = 1.0f;
float day = 0.0f;

// Estrutura para armazenar informações dos corpos celestes
struct CelestialBody {
    float radius;         // Raio do corpo
    float distance;       // Distância do corpo que orbita
    float orbitSpeed;     // Velocidade de órbita (dias terrestres para uma volta)
    float rotationSpeed;  // Velocidade de rotação no próprio eixo
    GLuint textureID;     // ID da textura
    std::vector<CelestialBody> satellites; // Luas ou outros satélites
};

std::vector<CelestialBody> solarSystem;
GLuint sunTexture, ringTexture;
GLUquadric* quad; // Objeto quadric para desenhar esferas e discos

//================================================================================
// FUNÇÕES DE CALLBACK E OPENGL
//================================================================================

// Função para carregar uma textura (Atividade 5)
GLuint loadTexture(const char* filename) {
    GLuint texture;
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);

    if (data) {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        // Parâmetros de wrapping e filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Carrega os dados da textura (verifica se é RGB ou RGBA)
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        gluBuild2DMipmaps(GL_TEXTURE_2D, format, width, height, format, GL_UNSIGNED_BYTE, data);
        
        stbi_image_free(data);
    } else {
        std::cerr << "Falha ao carregar textura: " << filename << std::endl;
        return 0;
    }
    return texture;
}


// Função de inicialização
void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Habilita o Z-Buffer para oclusão correta (Atividade 3)
    glEnable(GL_DEPTH_TEST);

    // Habilita iluminação (Atividade 4)
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glShadeModel(GL_SMOOTH); // Sombreamento suave (Atividade 4)

    // Habilita texturas 2D (Atividade 5)
    glEnable(GL_TEXTURE_2D);

    // Habilita a mistura de cores (para os anéis de Saturno com transparência)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Cria o objeto quadric para usar nas esferas (Atividade 5)
    quad = gluNewQuadric();
    gluQuadricTexture(quad, GL_TRUE);

    // Carrega todas as texturas
    sunTexture = loadTexture("sun.jpg");
    ringTexture = loadTexture("saturn_ring.png");

    // Inicializa os planetas
    CelestialBody mercury = {0.5f, 10.0f, 88.0f, 58.6f, loadTexture("mercury.jpg")};
    CelestialBody venus = {0.9f, 15.0f, 225.0f, -243.0f, loadTexture("venus.jpg")};
    CelestialBody earth = {1.0f, 20.0f, 365.0f, 1.0f, loadTexture("earth.jpg"), 
        {{0.3f, 2.0f, 27.3f, 27.3f, loadTexture("moon.jpg")}}};
    CelestialBody mars = {0.7f, 28.0f, 687.0f, 1.03f, loadTexture("mars.jpg")};
    CelestialBody jupiter = {4.0f, 45.0f, 4333.0f, 0.41f, loadTexture("jupiter.jpg")};
    CelestialBody saturn = {3.5f, 65.0f, 10759.0f, 0.44f, loadTexture("saturn.jpg")};
    CelestialBody uranus = {2.5f, 80.0f, 30687.0f, -0.72f, loadTexture("uranus.jpg")};
    CelestialBody neptune = {2.3f, 95.0f, 60190.0f, 0.67f, loadTexture("neptune.jpg")};

    solarSystem = {mercury, venus, earth, mars, jupiter, saturn, uranus, neptune};
}

// Função para desenhar a órbita (Atividade 6 - conceito de geração de vértices)
void drawOrbit(float radius) {
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 360; i++) {
        float angle = i * M_PI / 180.0f;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        glVertex3f(x, 0.0f, z);
    }
    glEnd();
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
}

// Função recursiva para desenhar um corpo celeste e seus satélites
void drawBody(const CelestialBody& body) {
    glPushMatrix(); // Salva o estado atual da matriz

    // 1. Rotação da órbita
    float orbitAngle = day * (365.0f / body.orbitSpeed);
    glRotatef(orbitAngle, 0.0f, 1.0f, 0.0f);

    // 2. Translação para a posição na órbita
    glTranslatef(body.distance, 0.0f, 0.0f);

    // Desenha a órbita do planeta (relativa ao Sol)
    if (body.distance > 0) { // Não desenha órbita para o Sol
        glPushMatrix();
        glTranslatef(-body.distance, 0.0, 0.0); // Volta para o centro do sistema
        drawOrbit(body.distance);
        glPopMatrix();
    }

    // 3. Rotação no próprio eixo
    float rotationAngle = day * (1.0f / body.rotationSpeed);
    glRotatef(rotationAngle, 0.0f, 1.0f, 0.0f);

    // Desenha o corpo celeste
    glBindTexture(GL_TEXTURE_2D, body.textureID);
    gluSphere(quad, body.radius, 50, 50);

    // Caso especial para os anéis de Saturno
    if (body.textureID == solarSystem[5].textureID) {
        glDisable(GL_LIGHTING); // Anéis não devem ter sombreamento realista para destacar a textura
        glBindTexture(GL_TEXTURE_2D, ringTexture);
        glRotatef(90, 1.0f, 0.0f, 0.0f); // Inclina os anéis
        gluDisk(quad, body.radius + 0.5f, body.radius + 4.0f, 50, 1);
        glEnable(GL_LIGHTING);
    }

    // Desenha os satélites (luas) recursivamente
    for (const auto& satellite : body.satellites) {
        drawBody(satellite);
    }

    glPopMatrix(); // Restaura a matriz para o estado anterior
}


// Função principal de renderização (display)
void display() {
    // Limpa os buffers de cor e profundidade (Atividade 3)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Configura a câmera (Atividade 2)
    float camLookX = camX + cos(camYaw * M_PI / 180.0f);
    float camLookY = camY + sin(camPitch * M_PI / 180.0f);
    float camLookZ = camZ + sin(camYaw * M_PI / 180.0f);
    gluLookAt(camX, camY, camZ, camLookX, camLookY, camLookZ, 0.0, 1.0, 0.0);

    // Define a posição da luz (Sol) na origem (Atividade 4)
    GLfloat light_position[] = {0.0, 0.0, 0.0, 1.0}; // Luz posicional
    GLfloat white_light[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat ambient_light[] = {0.2, 0.2, 0.2, 1.0};
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);


    // --- Desenho do Sistema Solar (Atividade 2 - Hierarquia) ---

    // Desenha o Sol (sem iluminação, material emissivo)
    glPushMatrix();
    glRotatef(day * (365.0f / 25.38f), 0.0f, 1.0f, 0.0f); // Rotação do Sol
    glDisable(GL_LIGHTING);
    glBindTexture(GL_TEXTURE_2D, sunTexture);
    glColor3f(1.0f, 1.0f, 0.8f); // Cor amarelada
    gluSphere(quad, 5.0f, 50, 50);
    glEnable(GL_LIGHTING);
    glPopMatrix();

    // Desenha os planetas
    for (const auto& planet : solarSystem) {
        drawBody(planet);
    }

    // Troca os buffers para exibir a cena renderizada (Atividade 1)
    glutSwapBuffers();
}

// Função de reshape (chamada quando a janela é redimensionada)
void reshape(int w, int h) {
    if (h == 0) h = 1;
    float ratio = 1.0 * w / h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, w, h);
    // Projeção em perspectiva (Atividade 2)
    gluPerspective(45, ratio, 1, 1000);
    glMatrixMode(GL_MODELVIEW);
}

// Função de timer para animação
void timer(int value) {
    day += animationSpeed; // Incrementa o "dia"
    glutPostRedisplay();   // Solicita que a cena seja redesenhada
    glutTimerFunc(16, timer, 0); // Chama a si mesma a cada ~16ms (~60 FPS)
}

// Função para tratar teclas do teclado (Atividade 1 - Interação)
void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case 'q':
        case 27: // ESC
            gluDeleteQuadric(quad);
            exit(0);
            break;
        case '+':
            animationSpeed *= 1.5f;
            break;
        case '-':
            animationSpeed /= 1.5f;
            break;
        // Controles de câmera WASD
        case 'w': camZ -= 2.0f; break;
        case 's': camZ += 2.0f; break;
        case 'a': camX -= 2.0f; break;
        case 'd': camX += 2.0f; break;
    }
}

// Função para tratar teclas especiais (setas)
void specialKeys(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:    camY += 1.0f; break;
        case GLUT_KEY_DOWN:  camY -= 1.0f; break;
        case GLUT_KEY_LEFT:  camYaw -= 2.0f; break;
        case GLUT_KEY_RIGHT: camYaw += 2.0f; break;
    }
}

// Função principal
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    // Double Buffer, cores RGB e buffer de profundidade (Atividade 1 e 3)
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutCreateWindow("Sistema Solar - Projeto Final");

    init();

    // Registra as funções de callback
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(0, timer, 0);

    // Inicia o loop principal do GLUT
    glutMainLoop();
    return 0;
}

/*
 * =================================================================================================
 * PROJETO FINAL - SISTEMA SOLAR EM OPENGL
 * DISCIPLINA: INTRODUÇÃO À COMPUTAÇÃO GRÁFICA
 * EQUIPE: DEIVID GABRIEL DA SILVA LOPES PROCOPIO
 *         HENRIQUE DE ANDRADE FRANCA
 *         PEDRO IAM PEREIRA LOPES
 *
 * CONCEITOS APLICADOS:
 * - Modelagem Hierárquica: Uso de glPushMatrix e glPopMatrix para criar a relação Sol-Planeta-Lua.
 * - Texturização: Mapeamento de imagens (.jpg, .png) em esferas para realismo visual.
 * - Iluminação: Modelo de iluminação de Phong com uma fonte de luz pontual representando o Sol.
 * - Animação: Controle de tempo para simular a órbita e rotação dos corpos celestes.
 * - Interação: Controle de câmera orbital e velocidade da simulação via teclado.
 * =================================================================================================
 */

#include <GL/glut.h>
#include <iostream> 
#include <cmath>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

// --- SEÇÃO DE VARIÁVEIS GLOBAIS ---

// Variáveis da Câmera Orbital.
float g_CameraAngle = 0.0f;    // Ângulo da câmera em torno da origem (do Sol).
float g_CameraDistance = 100.0f; // Distância da câmera até a origem.

// Variáveis da Animação.
float g_AnimationTime = 0.0f; // Um timer global que avança a cada quadro.
float g_AnimationSpeed = 1.0f;  // Um multiplicador para acelerar ou desacelerar o tempo.

// Organiza todas as propriedades de um corpo celeste.
struct CelestialBody {
    float radius;        // Raio do corpo em unidades OpenGL.
    float distance;      // Distância do corpo que ele orbita.
    float orbitSpeed;    // Período orbital em dias terrestres. Um valor maior significa órbita mais lenta.
    float rotationSpeed; // Período de rotação em seu eixo.
    GLuint textureID;    // ID numérico pro OpenGL atribuir uma textura carregada.
};

// Declaração das variáveis que irão armazenar os dados do sistema.
vector<CelestialBody> g_Planets; // Vetor para guardar todos os planetas.
CelestialBody g_Moon;            // Variável separada para a Lua.
GLuint g_SunTexture, g_RingTexture, g_EarthTexture; // IDs para texturas especiais.
GLUquadric* g_Quad; // Um objeto quadric da GLU, para mapear texturas em esferas.

// --- SEÇÃO DE FUNÇÕES UTILITÁRIAS ---

// Função que carrega uma imagem e a transforma em uma textura OpenGL.
GLuint loadTexture(const char* filename) {
    GLuint texture;
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (data) {
        glGenTextures(1, &texture);        // Gera um ID para a textura.
        glBindTexture(GL_TEXTURE_2D, texture); // Seleciona a textura para configurar.

        // Configura como a textura se repetirá ou será fixada nas bordas.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // Configura o filtro para quando a textura é minificada ou magnificada (melhora a qualidade).
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB; // Checa se a imagem tem transparência (alpha).

        // Envia os dados da imagem para a GPU e gera mipmaps (versões menores da textura para performance).
        gluBuild2DMipmaps(GL_TEXTURE_2D, format, width, height, format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    } else {
        cerr << "Falha ao carregar textura: " << filename << endl;
        return 0;
    }
    return texture;
}

// Função para desenhar uma linha circular que representa a órbita.
void drawOrbit(float radius) {
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_LINE_STRIP);
    // Loop de 0 a 360 graus para criar um círculo.
    for (int i = 0; i <= 360; i++) {
        float angle = i * M_PI / 180.0f; // Converte o grau para radianos.
        // Calcula a posição (x, z) no círculo usando trigonometria. Y é 0 para ser no plano.
        glVertex3f(radius * cos(angle), 0.0f, radius * sin(angle));
    }
    glEnd();
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
}

// --- SEÇÃO DE RENDERIZAÇÃO ---

// Função principal de desenho, chamada a cada quadro pela timer.
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // --- LÓGICA DA CÂMERA ORBITAL ---
    // Calcula a posição da câmera (X, Z) em um círculo ao redor da origem.
    float camX = g_CameraDistance * cos(g_CameraAngle * M_PI / 180.0f);
    float camZ = g_CameraDistance * sin(g_CameraAngle * M_PI / 180.0f);
    // Posiciona e orienta a câmera.
    gluLookAt(camX, 40.0, camZ,   // Posição da Câmera. Y=40 para uma visão de cima.
              0.0, 0.0, 0.0,   // Ponto para onde a câmera olha: o Sol.
              0.0, 1.0, 0.0);  // Vetor up.

    GLfloat light_position[] = {0.0, 0.0, 0.0, 1.0};
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    // --- DESENHO DO SOL ---
    glPushMatrix();
    float sunRotationAngle = g_AnimationTime * (365.0f / 25.38f); // Cálculo da rotação do Sol.
    glRotatef(sunRotationAngle, 0.0f, 1.0f, 0.0f);
    glDisable(GL_LIGHTING);
    glBindTexture(GL_TEXTURE_2D, g_SunTexture);
    gluSphere(g_Quad, 5.0f, 50, 50);
    glEnable(GL_LIGHTING);
    glPopMatrix();

    // --- DESENHO DAS ÓRBITAS ---
    // Desenha todas as linhas de órbita estáticas, centradas no Sol.
    for (const auto& planet : g_Planets) {
        drawOrbit(planet.distance);
    }

    // --- DESENHO DOS PLANETAS ---
    for (const auto& planet : g_Planets) {
        glPushMatrix();

        // --- Cálculos de Animação ---
        // Ângulo da órbita: baseado no tempo e no período orbital do planeta.
        // O fator 365.0 normaliza a velocidade em relação à Terra.
        float orbitAngle = g_AnimationTime * (365.0f / planet.orbitSpeed);
        // Ângulo da rotação própria: baseado no tempo e período de rotação do planeta.
        // O fator 30.0 foi ajustado para uma melhor visualização.
        float rotationAngle = g_AnimationTime * (30.0f / planet.rotationSpeed);

        // --- Aplicação das Transformações ---
        // 1. Rotação do sistema de coordenadas ao redor do Sol. Define a posição na órbita.
        glRotatef(orbitAngle, 0.0f, 1.0f, 0.0f);
        // 2. Translação ao longo do novo eixo X para afastar o planeta do Sol.
        glTranslatef(planet.distance, 0.0f, 0.0f);
        // 3. Rotação do planeta em seu próprio eixo.
        glRotatef(rotationAngle, 0.0f, 1.0f, 0.0f);
        
        // --- Desenho ---
        glBindTexture(GL_TEXTURE_2D, planet.textureID);
        gluSphere(g_Quad, planet.radius, 50, 50);

        // --- Caso Especial: Desenho da Lua da Terra ---
        if (planet.textureID == g_EarthTexture) {
            glPushMatrix();

            // As transformações da Lua são relativas ao sistema de coordenadas da Terra.
            float moonOrbitAngle = g_AnimationTime * (365.0f / g_Moon.orbitSpeed);
            glRotatef(moonOrbitAngle, 0.0f, 1.0f, 0.0f);
            glTranslatef(g_Moon.distance, 0.0f, 0.0f);
    
            glBindTexture(GL_TEXTURE_2D, g_Moon.textureID);
            gluSphere(g_Quad, g_Moon.radius, 30, 30);

            glPopMatrix();
        }

        // --- Caso Especial: Anéis de Saturno ---
        if (planet.textureID == g_Planets[5].textureID) {
            glDisable(GL_LIGHTING);
            glBindTexture(GL_TEXTURE_2D, g_RingTexture);
            glRotatef(90, 1.0f, 0.0f, 0.0f);
            gluDisk(g_Quad, planet.radius + 0.5f, planet.radius + 4.0f, 50, 1); // Desenha um disco vazado.
            glEnable(GL_LIGHTING);
        }

        glPopMatrix();
    }

    // Apresenta o quadro que foi desenhado em segundo plano (double buffering).
    glutSwapBuffers();
}

// --- SEÇÃO DE CONFIGURAÇÃO E CALLBACKS ---

void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glShadeModel(GL_SMOOTH);
    
    GLfloat white_light[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat ambient_light[] = {0.2, 0.2, 0.2, 1.0};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Fórmula padrão para transparência.
    
    // Criação do objeto quadric.
    g_Quad = gluNewQuadric();
    gluQuadricTexture(g_Quad, GL_TRUE); // Habilita a geração de coordenadas de textura para o quadric.

    // Carregamento das texturas.
    g_SunTexture = loadTexture("sun.jpg");
    g_RingTexture = loadTexture("saturn_ring.png");
    g_EarthTexture = loadTexture("earth.jpg"); // Guardamos o ID da Terra para a comparação da Lua.

    // Inicialização dos dados dos planetas e da Lua com valores.
    // {raio, distância, período orbital, período de rotação, textura}
    g_Planets = {
        {0.5f, 10.0f, 88.0f, 58.6f, loadTexture("mercury.jpg")},
        {0.9f, 15.0f, 225.0f, -243.0f, loadTexture("venus.jpg")},
        {1.0f, 20.0f, 365.0f, 1.0f, g_EarthTexture},
        {0.7f, 28.0f, 687.0f, 1.03f, loadTexture("mars.jpg")},
        {4.0f, 45.0f, 4333.0f, 0.41f, loadTexture("jupiter.jpg")},
        {3.5f, 65.0f, 10759.0f, 0.44f, loadTexture("saturn.jpg")},
        {2.5f, 80.0f, 30687.0f, -0.72f, loadTexture("uranus.jpg")},
        {2.3f, 95.0f, 60190.0f, 0.67f, loadTexture("neptune.jpg")}
    };
    g_Moon = {0.3f, 2.5f, 27.3f, 27.3f, loadTexture("moon.jpg")};
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, w, h);
    gluPerspective(45, (float)w / h, 1, 1000);
}

// Função de callback do timer, responsável por animar a cena.
void timer(int value) {
    g_AnimationTime += g_AnimationSpeed; // Avança o relógio da simulação.
    glutPostRedisplay(); // Solicita ao GLUT que redesenhe a tela.
    glutTimerFunc(16, timer, 0); // Pede para ser chamada novamente em ~16ms.
}

void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case 'q': case 27: // 'q' ou ESC para sair.
            gluDeleteQuadric(g_Quad);
            exit(0);
            break;
        case '+': g_AnimationSpeed *= 1.5; break; // Acelera a simulação.
        case '-': g_AnimationSpeed /= 1.5; break; // Desacelera a simulação.
    }
}

void specialKeys(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT:  g_CameraAngle -= 3.0f; break; // Gira a câmera para a esquerda.
        case GLUT_KEY_RIGHT: g_CameraAngle += 3.0f; break; // Gira a câmera para a direita.
        case GLUT_KEY_UP:    g_CameraDistance -= 3.0f; if (g_CameraDistance < 10.0f) g_CameraDistance = 10.0f; break; // Zoom in.
        case GLUT_KEY_DOWN:  g_CameraDistance += 3.0f; break; // Zoom out.
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Sistema Solar");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(0, timer, 0); // Inicia o timer da animação.
    glutMainLoop();
    return 0;
}

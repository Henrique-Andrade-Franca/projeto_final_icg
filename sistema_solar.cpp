/*
 * PROJETO FINAL - SISTEMA SOLAR EM OPENGL
 * DISCIPLINA: INTRODUÇÃO À COMPUTAÇÃO GRÁFICA
 *
 * MOTIVO DA ESCOLHA DE C++ EM VEZ DE C:
 * A linguagem C++ foi escolhida primariamente pela conveniência e segurança no gerenciamento
 * de dados. O uso de `std::vector` simplifica enormemente a criação de listas dinâmicas de
 * corpos celestes (como as luas de um planeta), gerenciando a memória automaticamente e
 * evitando erros comuns em C, como vazamentos de memória (memory leaks) ou acesso indevido
 * a ponteiros. Além disso, a biblioteca de I/O `<iostream>` oferece uma sintaxe mais limpa
 * para depuração em comparação com o `printf` do C.
 */

#include <GL/glut.h>
#include <iostream>
#include <cmath>
#include <vector>

// Define STB_IMAGE_IMPLEMENTATION antes de incluir stb_image.h para criar a implementação
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Adiciona o namespace std para evitar a necessidade de prefixar std:: em todo o código.
using namespace std;

//================================================================================
// BLOCO 1 - ESTRUTURA DE DADOS E GERENCIAMENTO DE ASSETS (INTEGRANTE 1)
//
// Esta seção é responsável pela definição da estrutura de dados que armazena as
// informações do sistema solar e pelas rotinas de carregamento de recursos externos,
// como as texturas dos planetas.
//================================================================================

// Estrutura para armazenar informações dos corpos celestes
struct CelestialBody {
    float radius;         // Raio do corpo
    float distance;       // Distância do corpo que orbita
    float orbitSpeed;     // Velocidade de órbita (dias terrestres para uma volta)
    float rotationSpeed;  // Velocidade de rotação no próprio eixo
    GLuint textureID;     // ID da textura OpenGL
    vector<CelestialBody> satellites; // Vetor de satélites (luas)
};

// Variáveis globais para os dados do sistema e texturas
vector<CelestialBody> solarSystem;
GLuint sunTexture, ringTexture;
GLUquadric* quad; // Objeto quadric para desenhar esferas (reutilizado para todos)

// Função para carregar uma textura de um arquivo de imagem (Atividade 5)
// Retorna o ID da textura gerada pelo OpenGL.
GLuint loadTexture(const char* filename) {
    GLuint texture;
    int width, height, nrChannels;
    // Carrega a imagem usando a biblioteca stb_image.h
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);

    if (data) {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        // Define os parâmetros de como a textura se comporta (wrapping e filtering)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Determina o formato da imagem (RGB ou RGBA com transparência)
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        // Envia os dados da imagem para a GPU e gera mipmaps para melhor qualidade
        gluBuild2DMipmaps(GL_TEXTURE_2D, format, width, height, format, GL_UNSIGNED_BYTE, data);
        
        // Libera a memória da imagem da CPU, pois ela já está na GPU
        stbi_image_free(data);
    } else {
        cerr << "Falha ao carregar textura: " << filename << endl;
        return 0;
    }
    return texture;
}


//================================================================================
// BLOCO 2 - LÓGICA DE RENDERIZAÇÃO E MODELAGEM HIERÁRQUICA (INTEGRANTE 2)
//
// Este é o coração do projeto. Contém a função `display`, que é chamada a cada quadro
// para desenhar a cena, e as funções auxiliares `drawBody` e `drawOrbit`, que
// implementam a lógica de transformações (rotação, translação) e o desenho dos
// objetos usando a pilha de matrizes do OpenGL.
//================================================================================

// Variáveis de controle da câmera
float camX = 0.0f, camY = 25.0f, camZ = 100.0f;
float camYaw = -90.0f; // Rotação da câmera em torno do eixo Y
float camPitch = -15.0f; // Rotação da câmera em torno do eixo X

// Função para desenhar a linha da órbita de um planeta (Atividade 6 - conceito)
void drawOrbit(float radius) {
    // Desabilitamos iluminação e textura para desenhar uma linha simples
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
    // Reabilitamos para os próximos objetos
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
}

// Função recursiva para desenhar um corpo celeste e seus satélites
// Esta função é a chave da modelagem hierárquica (Atividade 2)
void drawBody(const CelestialBody& body, float day) {
    glPushMatrix(); // Salva a matriz de transformação atual (do corpo pai)

    // 1. ROTAÇÃO DA ÓRBITA: Gira o sistema de coordenadas em torno do corpo pai
    float orbitAngle = day * (365.0f / body.orbitSpeed);
    glRotatef(orbitAngle, 0.0f, 1.0f, 0.0f);

    // 2. TRANSLAÇÃO: Afasta o corpo celeste do seu pai
    glTranslatef(body.distance, 0.0f, 0.0f);

    // Desenha a órbita do planeta
    if (body.distance > 0) {
        glPushMatrix();
        glTranslatef(-body.distance, 0.0, 0.0);
        drawOrbit(body.distance);
        glPopMatrix();
    }

    // 3. ROTAÇÃO NO PRÓPRIO EIXO: Gira o corpo em torno de si mesmo
    float rotationAngle = day * (365.0f / body.rotationSpeed); // Usamos 365 para normalizar
    glRotatef(rotationAngle, 0.0f, 1.0f, 0.0f);

    // 4. DESENHO DO OBJETO: Agora, na posição e orientação corretas
    glBindTexture(GL_TEXTURE_2D, body.textureID);
    gluSphere(quad, body.radius, 50, 50);

    // Caso especial para os anéis de Saturno
    if (body.textureID == solarSystem[5].textureID) {
        glDisable(GL_LIGHTING);
        glBindTexture(GL_TEXTURE_2D, ringTexture);
        glRotatef(90, 1.0f, 0.0f, 0.0f); // Inclina os anéis
        gluDisk(quad, body.radius + 0.5f, body.radius + 4.0f, 50, 1);
        glEnable(GL_LIGHTING);
    }

    // 5. RECURSÃO: Chama a mesma função para desenhar os satélites (luas)
    for (const auto& satellite : body.satellites) {
        drawBody(satellite, day);
    }

    glPopMatrix(); // Restaura a matriz, voltando ao sistema de coordenadas do corpo pai
}


// Função principal de renderização (display)
void display() {
    // Variável que controla a passagem do tempo na animação
    static float day = 0.0f;
    day += 0.5f; // Incremento a cada quadro
    
    // Limpa os buffers de cor e profundidade (Atividade 3)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

 
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Configura a câmera usando gluLookAt (Atividade 2)
    // Calcula o ponto para onde a câmera está olhando
    float camLookX = camX + cos(camYaw * M_PI / 180.0f);
    float camLookY = camY + sin(camPitch * M_PI / 180.0f);
    float camLookZ = camZ + sin(camYaw * M_PI / 180.0f);
    gluLookAt(camX, camY, camZ, camLookX, camLookY, camLookZ, 0.0, 1.0, 0.0);

    // Define a posição da luz (Sol) na origem do MUNDO (0,0,0) (Atividade 4)
    // Esta chamada deve ser FEITA APÓS o gluLookAt para que a luz fique no
    // sistema de coordenadas do mundo, e não presa à câmera.
    GLfloat light_position[] = {0.0, 0.0, 0.0, 1.0}; // w=1.0 para luz posicional
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    
    // --- Desenho do Sistema Solar ---

    // Desenha o Sol (não é afetado pela iluminação, ele a emite)
    glPushMatrix();
    glRotatef(day * (365.0f / 25.38f), 0.0f, 1.0f, 0.0f);
    glDisable(GL_LIGHTING); // Desabilita a luz para o Sol não ter sombra
    glBindTexture(GL_TEXTURE_2D, sunTexture);
    gluSphere(quad, 5.0f, 50, 50);
    glEnable(GL_LIGHTING); // Reabilita para os planetas
    glPopMatrix();

    // Desenha todos os planetas do sistema solar
    for (const auto& planet : solarSystem) {
        drawBody(planet, day);
    }

    // Troca os buffers para exibir a cena renderizada (Atividade 1)
    glutSwapBuffers();
}


//================================================================================
// BLOCO 3 - CONFIGURAÇÃO INICIAL, JANELA E INTERAÇÃO (INTEGRANTE 3)
//
// Esta seção contém a função `main` e todas as funções de callback do GLUT.
// É responsável por inicializar a janela, configurar o estado inicial do OpenGL,
// popular os dados do sistema solar e gerenciar a entrada do usuário (teclado)
// e a animação (timer).
//================================================================================

// Função de inicialização geral do OpenGL e dos dados
void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Habilita o Z-Buffer para oclusão correta (Atividade 3)
    glEnable(GL_DEPTH_TEST);

    // Configura e habilita a iluminação (Atividade 4)
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glShadeModel(GL_SMOOTH); // Sombreamento suave
    
    GLfloat white_light[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat ambient_light[] = {0.2, 0.2, 0.2, 1.0};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);

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

    // Inicializa os dados dos planetas e luas
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

// Callback de reshape (chamada quando a janela é redimensionada)
void reshape(int w, int h) {
    if (h == 0) h = 1;
    float ratio = 1.0 * w / h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, w, h);
    // Define a projeção em perspectiva (Atividade 2)
    gluPerspective(45, ratio, 1, 1000);
    glMatrixMode(GL_MODELVIEW);
}

// Callback de timer para controlar a animação
void timer(int value) {
    glutPostRedisplay();   // Solicita que a cena seja redesenhada
    glutTimerFunc(16, timer, 0); // Chama a si mesma a cada ~16ms (~60 FPS)
}

// Callback para tratar teclas comuns do teclado (Atividade 1 - Interação)
void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case 'q':
        case 27: // Tecla ESC
            gluDeleteQuadric(quad); // Libera a memória do objeto quadric
            exit(0);
            break;
        // Controles de câmera WASD
        case 'w': camZ -= 2.0f; break;
        case 's': camZ += 2.0f; break;
        case 'a': camX -= 2.0f; break;
        case 'd': camX += 2.0f; break;
    }
}

// Callback para tratar teclas especiais (setas)
void specialKeys(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:    camY += 2.0f; break;
        case GLUT_KEY_DOWN:  camY -= 2.0f; break;
        case GLUT_KEY_LEFT:  camYaw -= 3.0f; break;
        case GLUT_KEY_RIGHT: camYaw += 3.0f; break;
    }
}

// Função principal que inicializa o GLUT e inicia o loop
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    // Double Buffer, cores RGB e buffer de profundidade (Atividade 1 e 3)
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Sistema Solar");

    init(); // Chama nossa função de inicialização

    // Registra as funções de callback que o GLUT usará
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(0, timer, 0); // Inicia o timer para a animação

    // Inicia o loop principal do GLUT, que processa eventos e chama os callbacks
    glutMainLoop();
    return 0;
}

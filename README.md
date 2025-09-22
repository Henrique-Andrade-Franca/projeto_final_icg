# Projeto Final: Sistema Solar em OpenGL

**Disciplina:** Introdução à Computação Gráfica

**Equipe:**
* DEIVID GABRIEL DA SILVA LOPES PROCOPIO
* HENRIQUE DE ANDRADE FRANCA
* PEDRO IAM PEREIRA LOPES

Este projeto consiste na criação de uma simulação 3D interativa do sistema solar, utilizando C++ e a biblioteca OpenGL (com FreeGLUT).

---

### O que o código faz

O programa renderiza uma cena 3D dinâmica que simula os principais corpos do nosso sistema solar. As funcionalidades implementadas são:

* **Modelagem Hierárquica:** O Sol está no centro do sistema, com os 8 planetas (Mercúrio, Vênus, Terra, Marte, Júpiter, Saturno, Urano e Netuno) orbitando ao seu redor. A Lua da Terra também está presente, orbitando corretamente seu planeta.
* **Movimento Realístico:** Cada planeta possui sua própria velocidade de órbita e de rotação em seu eixo, criando uma animação contínua e representativa.
* **Iluminação e Sombreamento:** O Sol atua como uma fonte de luz pontual na origem da cena. Os planetas são iluminados por esta fonte, exibindo sombreamento suave (`GL_SMOOTH`) que lhes confere volume e profundidade.
* **Texturização:** Cada corpo celeste é mapeado com uma textura de imagem real (`.jpg` ou `.png`), conferindo um aspecto visual mais detalhado e reconhecível. Os anéis de Saturno também possuem uma textura própria com transparência.
* **Interatividade:** O usuário pode controlar uma câmera livre para navegar pela cena, utilizando as teclas do teclado para se mover e rotacionar a visão.
* **Controle de Animação:** É possível aumentar ou diminuir a velocidade da simulação em tempo de execução.

---

### Imagem do Programa

<img width="1213" height="766" alt="image" src="https://github.com/user-attachments/assets/64b960aa-b11f-4efa-b847-3cc72eabe23a" />

---

### Instruções de como Compilar e Executar

#### Dependências

Para compilar e executar o projeto, você precisará dos seguintes componentes:

1.  **Compilador C++:** Um compilador moderno como o `g++`.
2.  **FreeGLUT:** Biblioteca de utilitários para OpenGL.
    * **Linux (Debian/Ubuntu):** `sudo apt-get install freeglut3-dev`
    * **macOS (com Homebrew):** `brew install freeglut`
3.  **Bibliotecas OpenGL:** `GL` e `GLU`, que geralmente são instaladas junto com os drivers da placa de vídeo ou o FreeGLUT.
4.  **Biblioteca `stb_image.h`:** Uma biblioteca de um único arquivo para carregar imagens. **Este arquivo deve estar na mesma pasta do código-fonte.**
5.  **Arquivos de Textura:** Todas as imagens (`.jpg` e `.png`) dos planetas, sol e anéis devem estar localizadas **na mesma pasta onde o executável será gerado.**

#### Compilação

Com todas as dependências instaladas, abra um terminal na pasta do projeto e execute o seguinte comando:

```bash
g++ -o sistema_solar sistema_solar.cpp -lglut -lGLU -lGL -lm
```

Após a compilação bem-sucedida, execute o programa com o comando:

```bash
./sistema_solar
```

#### Controles

  - `W` / `S`: Mover a câmera para frente / trás.
  - `A` / `D`: Mover a câmera para a esquerda / direita.
  - `Setas Cima` / `Baixo`: Mover a câmera para cima / baixo.
  - `Setas Esquerda` / `Direita`: Girar a câmera horizontalmente.
  - `+` / `-`: Aumentar / diminuir a velocidade da animação.
  - `Q` ou `ESC`: Fechar o programa.

-----

### Principais problemas encontrados

1.  **Gerenciamento da Pilha de Matrizes:** O desafio inicial foi garantir o aninhamento correto das chamadas `glPushMatrix()` e `glPopMatrix()` para a modelagem hierárquica. Um `glPopMatrix()` no lugar errado fazia com que Marte orbitasse a Terra, por exemplo. A solução foi desenhar a hierarquia em papel e seguir rigorosamente o padrão de salvar a matriz antes de uma transformação e restaurá-la após o desenho do objeto e seus satélites.

2.  **Aplicação de Texturas em Esferas:** A função padrão `glutSolidSphere()` não gera coordenadas de textura, o que impede o mapeamento de imagens. A solução foi utilizar os objetos "quadrics" da biblioteca GLU (`gluNewQuadric`, `gluQuadricTexture`, `gluSphere`), que permitem a geração automática dessas coordenadas e a correta aplicação das texturas.

3.  **Carregamento de Arquivos:** O erro mais comum foi o de "Falha ao carregar textura". Isso ocorria porque o programa não encontrava os arquivos de imagem. A solução foi garantir que o caminho dos arquivos estivesse correto e que as imagens estivessem sempre na mesma pasta do executável final, simplificando o gerenciamento de assets.

-----

### O que pode ser melhorado (e como melhorar)

  - **Skybox para Fundo Espacial:** Atualmente o fundo é preto. Uma grande melhoria seria a implementação de um *skybox* (um cubo gigante com texturas de estrelas mapeadas em suas faces internas). Isso criaria um ambiente muito mais imersivo. A implementação envolveria carregar 6 texturas e desenhar um cubo em volta de toda a cena, desabilitando o teste de profundidade para o fundo.

  - **Órbitas Elípticas e Inclinadas:** As órbitas no código são circulares e no mesmo plano. Para maior realismo, elas poderiam ser implementadas como elipses (usando `glScale` para achatar o círculo antes da rotação) e com suas respectivas inclinações em relação à eclíptica (adicionando uma `glRotatef` no eixo X ou Z antes da translação).

  - **Sistema de Câmera Avançado:** A câmera atual é livre. Poderia ser criado um sistema para focar em um planeta específico, com a câmera seguindo-o em sua órbita. Isso exigiria atualizar os parâmetros do `gluLookAt()` a cada quadro com a posição do planeta alvo.

  - **Exibição de Informações na Tela:** Implementar texto na tela (usando `glutBitmapCharacter`) para mostrar o nome do planeta mais próximo, a velocidade da simulação ou outras informações, tornando a experiência mais informativa.

-----

### O que cada integrante fez

*(Descreva aqui a contribuição de cada membro da equipe para o projeto)*

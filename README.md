# Projeto Final: Sistema Solar em OpenGL

**Disciplina:** Introdução à Computação Gráfica

**Equipe:**

  * DEIVID GABRIEL DA SILVA LOPES PROCOPIO
  * HENRIQUE DE ANDRADE FRANCA
  * PEDRO IAM PEREIRA LOPES

Este projeto consiste na criação de uma simulação 3D interativa do sistema solar, utilizando C++ e a biblioteca OpenGL (com FreeGLUT), com foco em uma implementação clara e didática dos conceitos aprendidos na disciplina.

-----

### O que o código faz

O programa renderiza uma cena 3D dinâmica que simula os principais corpos do nosso sistema solar. As funcionalidades implementadas são:

  * **Modelagem Hierárquica:** O Sol está no centro do sistema, com os 8 planetas orbitando ao seu redor. A Lua da Terra também está presente, orbitando corretamente seu planeta através de transformações aninhadas (`glPushMatrix`/`glPopMatrix`).
  * **Animação Contínua:** Cada planeta possui sua própria velocidade de órbita e de rotação em seu eixo, criando um movimento contínuo e proporcional.
  * **Iluminação e Sombreamento:** O Sol atua como uma fonte de luz pontual na origem da cena. Os planetas são iluminados por esta fonte, exibindo sombreamento suave (`GL_SMOOTH`) que lhes confere volume e profundidade.
  * **Texturização:** Cada corpo celeste é mapeado com uma textura de imagem (`.jpg` ou `.png`), conferindo um aspecto visual detalhado. Os anéis de Saturno são implementados com uma textura própria com transparência.
  * **Interatividade:** O usuário pode controlar uma **câmera orbital**, girando ao redor do Sol e aplicando zoom para observar a cena de diferentes ângulos.
  * **Controle de Animação:** É possível aumentar ou diminuir a velocidade da simulação em tempo de execução.

-----

### Imagem do Programa

<img width="1214" height="765" alt="image" src="https://github.com/user-attachments/assets/a192acfe-449d-42c8-ac25-8819e6f58033" />

-----

### Instruções de como Compilar e Executar

#### Dependências

Para compilar e executar o projeto, você precisará dos seguintes componentes:

1.  **Compilador C++:** Um compilador moderno como o `g++`.
2.  **FreeGLUT:** Biblioteca de utilitários para OpenGL.
      * **Linux (Debian/Ubuntu):** `sudo apt-get install freeglut3-dev`
      * **macOS (com Homebrew):** `brew install freeglut`
3.  **Bibliotecas OpenGL:** `GL` e `GLU`, geralmente instaladas com os drivers da placa de vídeo ou o FreeGLUT.
4.  **Biblioteca `stb_image.h`:** Uma biblioteca de um único arquivo para carregar imagens. **Este arquivo deve estar na mesma pasta do código-fonte.**
      * Disponível em: [https://github.com/nothings/stb](https://github.com/nothings/stb)
5.  **Arquivos de Textura:** Todas as imagens (`.jpg` e `.png`) devem estar localizadas **na mesma pasta onde o executável será gerado.**
      * Disponível em: [https://www.solarsystemscope.com/textures/](https://www.solarsystemscope.com/textures/) 

#### Compilação

Com todas as dependências instaladas, abra um terminal na pasta do projeto e execute o seguinte comando:

```bash
g++ -o sistema_solar sistema_solar.cpp -lglut -lGLU -lGL -lm
```

*(Nota: ajuste o nome `sistema_solar.cpp` para o nome que você salvou o arquivo)*

#### Execução

Após a compilação bem-sucedida, execute o programa com o comando:

```bash
./sistema_solar
```

#### Controles

  * **Setas Esquerda / Direita:** Gira a câmera ao redor do Sol.
  * **Setas Cima / Baixo:** Aplica zoom (aproxima/afasta a câmera).
  * **`+` / `-`:** Aumenta / diminui a velocidade da animação.
  * **`Q` ou `ESC`:** Fecha o programa.

-----

### Principais problemas encontrados

1.  **Gerenciamento da Pilha de Matrizes:** O conceito de `glPushMatrix()` e `glPopMatrix()` foi fundamental para a hierarquia Sol-Planeta-Lua. O principal desafio foi garantir que, para cada planeta, a matriz do Sol fosse salva e restaurada corretamente, e o mesmo para a Lua em relação à Terra, evitando que um corpo celeste afetasse a posição do outro indevidamente.

2.  **Aplicação de Texturas em Esferas:** A função padrão `glutSolidSphere()` não gera coordenadas de textura. A solução foi utilizar os objetos "quadrics" da biblioteca GLU (`gluNewQuadric`, `gluQuadricTexture`, `gluSphere`), que permitem a geração automática dessas coordenadas e a correta aplicação das texturas nos corpos esféricos.

-----

### O que pode ser melhorado (e como melhorar)

  * **Skybox para Fundo Espacial:** Atualmente o fundo é preto. Uma grande melhoria seria a implementação de um *skybox* (um cubo gigante com texturas de estrelas mapeadas em suas faces internas) para criar um ambiente muito mais imersivo.

  * **Órbitas Elípticas e Inclinadas:** As órbitas no código são circulares e no mesmo plano. Para maior realismo, elas poderiam ser implementadas como elipses (usando `glScale` para achatar o círculo antes da rotação) e com suas respectivas inclinações em relação à eclíptica (adicionando uma `glRotatef` extra).

  * **Exibição de Informações na Tela:** Implementar texto na tela (usando `glutBitmapCharacter`) para mostrar informações, como o nome do planeta em foco ou a velocidade atual da simulação, tornando a experiência mais interativa.

  * **Iluminação Realista:** O modelo de luz poderia ser melhorado ativando a **atenuação**, que faria a luz do Sol diminuir com a distância, tornando os planetas externos mais escuros, e reduzindo a **luz ambiente** para criar sombras mais profundas e realistas, como as do espaço.

-----

### O que cada integrante fez

O projeto foi dividido em três seções principais de implementação, com cada integrante sendo responsável por uma parte fundamental do código, garantindo uma contribuição proporcional de toda a equipe.

* **Henrique:** Responsável pela arquitetura de dados e pelas funções utilitárias de base do projeto. Ele definiu a estrutura de dados `CelestialBody`, que organiza as informações de cada corpo celeste, e implementou as funções essenciais `loadTexture`, para o carregamento e configuração de todas as texturas, e `drawOrbit`, para a renderização das órbitas.

* **Deivid:** Encarregado da implementação da função de renderização principal, o coração visual do projeto. Ele desenvolveu toda a lógica contida na função `display`, que inclui o posicionamento da câmera orbital (`gluLookAt`), a definição da fonte de luz, e a correta aplicação das transformações hierárquicas (`glPushMatrix`/`glPopMatrix`, `glRotatef`, `glTranslatef`) para animar os planetas, a lua e os anéis de Saturno a cada quadro.

* **Pedro:** Focado na inicialização do ambiente OpenGL, na interatividade e no ciclo de vida da aplicação. Ele foi responsável pela função `main`, que cria a janela e registra os callbacks, e pela função `init`, que carrega os dados dos planetas e configura os estados do OpenGL (iluminação, profundidade, etc.). Adicionalmente, implementou a animação através da função `timer` e a interatividade do usuário com o teclado (`keyboard` e `specialKeys`).


// Bibliotecas de sistema
#include <stdlib.h>
#include <stdio.h>
// Bibliotecas gráficas
#include <GL/glut.h>
// Blibliotecas adicionais
#include <math.h>
#include <vector>

// Variaveis de controle
#define CIRCLE_POINTS 25
#define CIRCLE_RADIUS 3.0f
#define TAB_X 5
#define TAB_Y 7

// Legenda: c = cachorro, o = onça, f = posição inválida, n = posição livre
std::vector<std::vector<char>> tabuleiro = {{'c', 'c', 'c', 'n', 'n', 'f', 'n',},
                                            {'c', 'c', 'c', 'n', 'n', 'n', 'n',},
                                            {'c', 'c', 'o', 'n', 'n', 'n', 'n',},
                                            {'c', 'c', 'c', 'n', 'n', 'n', 'n',},
                                            {'c', 'c', 'c', 'n', 'n', 'f', 'n',}};

// Função adaptada do programa demonstrativo 'Worms'
void drawCircle(float x0, float y0, float radius, char player) {
    int i;
    float angle;

    /* a table of offsets for a circle (used in drawCircle) */
    static float circlex[CIRCLE_POINTS];
    static float circley[CIRCLE_POINTS];
    static int   inited = 0;

    if (! inited) {
        for (i = 0; i < CIRCLE_POINTS; i++) {
            angle = 2.0 * M_PI * i / CIRCLE_POINTS;
            circlex[i] = cos(angle);
            circley[i] = sin(angle);
        }
        inited++;
    };

    if(player == 'c')
        glColor3f(0.0f, 1.0f, 0.0f);
    else
        glColor3f(0.0f, 0.0f, 1.0f);

    glBegin(GL_POLYGON);
    for(i = 0; i < CIRCLE_POINTS; i++)
        glVertex2f((radius * circlex[i]) + x0, (radius * circley[i]) + y0);
    glEnd();

  return;
}

/* Fun��es projetadas para tratar as diferentes classes de eventos */
void Atualiza_tamanho(int largura, int altura){
    glViewport(0, 0, largura, altura);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    gluOrtho2D (-25.0f, 45.0f, -30.0f, 30.0f);
}

void desenha_tabuleiro() {
    float x1 = -20.0,
          y1 = -20.0,
          x2 = 20.0,
          y2 = 20.0;

    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

        // Desenhar as linhas do quadrado, losango e x
    glBegin(GL_LINES);
        // Quadrado
        glVertex2f(x1, y1);
        glVertex2f(x1, y2);

        glVertex2f(x1, y2);
        glVertex2f(x2, y2);

        glVertex2f(x2, y2);
        glVertex2f(x2, y1);

        glVertex2f(x2, y1);
        glVertex2f(x1, y1);

        // Losango
        glVertex2f(x1, 0.0);
        glVertex2f(0.0, y2);

        glVertex2f(0.0, y2);
        glVertex2f(x2, 0.0);

        glVertex2f(x2, 0.0);
        glVertex2f(0.0, y1);

        glVertex2f(0.0, y1);
        glVertex2f(x1, 0.0);

        // X
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);

        glVertex2f(x1, y2);
        glVertex2f(x2, y1);

        // Linhas verticais
        glVertex2f(0.0, y1);
        glVertex2f(0.0, y2);

        glVertex2f(x1/2, y1);
        glVertex2f(x1/2, y2);

        glVertex2f(x2/2, y1);
        glVertex2f(x2/2, y2);

        // Linhas horizontais
        glVertex2f(x1, 0.0);
        glVertex2f(x2, 0.0);

        glVertex2f(x1, y1/2);
        glVertex2f(x2, y1/2);

        glVertex2f(x1, y2/2);
        glVertex2f(x2, y2/2);

        // Toca
        glVertex2f(x1, 0.0);
        glVertex2f(2*x2, 0.0);

        glVertex2f(x2, 0.0);
        glVertex2f(2*x2, 0.0);

        glVertex2f(x2, 0.0);
        glVertex2f(2*x2, 20.0);

        glVertex2f(x2, 0.0);
        glVertex2f(2*x2,-20.0);

        glVertex2f(2*x2, 20.0);
        glVertex2f(2*x2,-20.0);

        glVertex2f(x2 + x2/2, 10.0);
        glVertex2f(x2 + x2/2,-10.0);
    glEnd();
}

void desenha_pecas() {
    for(int i = 0; i < TAB_X; i++) {
        for(int j = 0; j < TAB_Y; j++) {
            if(tabuleiro[i][j] != 'f' && tabuleiro[i][j] != 'n')
                drawCircle(-20.0 + j*10.0, 20 - i*10.0, CIRCLE_RADIUS, tabuleiro[i][j]);
        }
    }
}

void Atualiza_desenho(void){
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1.0f, 0.0f, 0.0f);

	desenha_tabuleiro();
    desenha_pecas();

	glFlush();
}

void Teclado( unsigned char tecla, int x, int y){
    switch (tecla){
        case 27 :
        case 'q':
            exit(0);
        break;
    }
}

// Trecho de funções do jogo
bool par(int num) {
    return !(num & 1);
}

bool movimenta(int x1, int y1, int x2, int y2) {
    /* Pecas localizadas em posicoes onde x1 e y1 são ambos pares
        ou impares se movem em todas as direções (pecas totais).
       O restante se movimenta somente na vertical ou horizontal,
        sem diagonal (pecas restritias)
    */
    if( /* Verifica se a peca é restrita */
        (!par(i) && par(j)) || (par(i) && !par(j))
        /* Verifica se o movimento é diagonal */
        && !((x2 == x1 && y2 != y1) || (x2 != x1 && y2 == y1))
      )
        /* Movimento invalido: peça restrita tentou andar diagonalmente */
        return false;
    /* Verifica se a posição está livre */
    if()

}

int main(int argc, char *argv[]){
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowPosition(100,50);
	glutInitWindowSize(500,500);
	glutCreateWindow("Raz�o de Aspecto");
	glutDisplayFunc(Atualiza_desenho);
	glutReshapeFunc(Atualiza_tamanho);
	glutKeyboardFunc (Teclado);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glutMainLoop();
    return 0;
}

// Bibliotecas de sistema
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <windows.h>
#include <ctime>
// Bibliotecas gráficas
#include <GL/glut.h>
// Blibliotecas adicionais
#include <math.h>
#include <vector>
#include <map>
// Biblioteca de thread
#include <thread>

// Variaveis de controle
#define CIRCLE_POINTS 25
#define CIRCLE_RADIUS 3.0f
#define CURSOR_RADIUS 4.5f
#define MOVIMENTOS 9
#define TAB_X 5
#define TAB_Y 7
// Variáveis de
#define MAX  1000
#define MIN -1000

//const int N_THREADS = std::thread::hardware_concurrency();
int N_JOGADAS = 0;

// Armazena os dados do cursor de seleção de peça
typedef struct cursor_data {
    int x;
    int y;
    char player;
    int next_x;
    int next_y;
    bool holding;
    char direction;
} cursor_data;

// Inicializa a variável
cursor_data cursor = {2, 2, 'o', 2, 2, false, '5'};

// Variável de controle dos parametros do jogo
typedef struct {
    char mode;          // pvp, pvm
    char difficulty;    // easy, medium, hard
    bool debug_mode;    // false by default
    char player1;
    char player2;
    int pcs_c;
} game_data ;

game_data gdt = {'m', '1', true, 'c', 'o', 6};

// Legenda: c = cachorro, o = onça, f = posição inválida, n = posição livre
typedef std::vector<std::vector<char>> Tabuleiro;
Tabuleiro tabuleiro = {{'c', 'c', 'c', 'n', 'n', 'f', 'n',},
                       {'c', 'c', 'c', 'n', 'n', 'n', 'f',},
                       {'c', 'c', 'o', 'n', 'n', 'n', 'n',},
                       {'c', 'c', 'c', 'n', 'n', 'n', 'f',},
                       {'c', 'c', 'c', 'n', 'n', 'f', 'n',}};

std::ostream& operator <<(std::ostream &out, const Tabuleiro &tab) {
    for(int i = 0; i < TAB_X; i++) {
        for(int j = 0; j < TAB_Y; j++)
            out << tab[i][j] << " ";
        out << "\n";
    }
    return out;
}

// Estrutura de controle de possiveis movimentos para cada peca
typedef struct {
    int x;
    int y;
} Positions;

typedef struct {
    float x;
    float y;
} FltPositions;

typedef struct {
    Tabuleiro tab;
    int heuristica;
} ResultMinimax;

// Vetor de maps que contem todos os movimentos possiveis
std::map<const char, Positions> movimentos = {
    {'7',{-1, -1}}, {'8',{-1, -1}}, {'9',{-1, -1}},
    {'4',{-1, -1}},                 {'6',{-1, -1}},
    {'1',{-1, -1}}, {'2',{-1, -1}}, {'3',{-1, -1}}
};

// Estrutura temporaria para auxiliar a declaração de objetos
std::map<const char, Positions> temp = movimentos;

int max(int a, int b) {
    return (a > b ? a : b);
}

int min(int a, int b) {
    return (a < b ? a : b);
}

void mover_peca(int x1, int y1, int x2, int y2) {
    tabuleiro[x2][y2] = tabuleiro[x1][y1];
    tabuleiro[x1][y1] = 'n';
    // Para caso o movimento tenha sido para comer uma peça
    if(abs(x2 - x1) == 2 || abs(y2 - y1) == 2) {
        gdt.pcs_c--;
        tabuleiro[x1 + (x2 - x1)/2][y1 + (y2 - y1)/2] = 'n';
    }
}

// Função adaptada do programa demonstrativo 'Worms'
void drawCircle(float x0, float y0, float radius, char player, bool filled, bool cursor_selected) {
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
    // Seta cores diferentes para a onca (azul) e os cachorros (verde)
    if(cursor_selected)
        glColor3f(0.0f, 0.0f, 0.0f);
    else if(!filled) {
        if(cursor.player == 'o')
            glColor3f(0.0f, 0.0f, 1.0f);
        else
            glColor3f(0.0f, 1.0f, 0.0f);
    }
    else if(player == 'c')
        glColor3f(0.0f, 1.0f, 0.0f);
    else
        glColor3f(0.0f, 0.0f, 1.0f);
    // Imprime uma um circulo (true) ou uma circunferencia (false)
    if (filled)
        glBegin(GL_POLYGON);
    else
        glBegin(GL_LINE_LOOP);
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
                drawCircle(-20.0 + j*10.0, 20 - i*10.0, CIRCLE_RADIUS, tabuleiro[i][j], true,
                           false);
        }
    }
}

void desenha_cursor() {
    if(cursor.holding) {
        for(int i = '1'; i < MOVIMENTOS + 49; i++) {
            if(i != '5' && movimentos[i].x != -1)
                drawCircle(-20.0 + movimentos[i].y*10.0, 20 - movimentos[i].x*10.0, CURSOR_RADIUS,
                            tabuleiro[movimentos[i].x][movimentos[i].y], false,
                            cursor.next_x == movimentos[i].x && cursor.next_y == movimentos[i].y ? true : false );
        }
    }
    drawCircle(-20.0 + cursor.y*10.0, 20 - cursor.x*10.0, CURSOR_RADIUS,
                   tabuleiro[cursor.x][cursor.y], false, true);
}

// Trecho de funções do jogo
bool par(int num) {
    return !(num & 1);
}

bool validar_posicao(int x1, int y1, int x2, int y2) {
    /* Pecas localizadas em posicoes onde x1 e y1 são ambos pares
        ou impares se movem em todas as direções (pecas totais).
       O restante se movimenta somente na vertical ou horizontal,
        sem diagonal (pecas restritias)
    */
    // Verifica se o movimento não é para fora do tabuleiro
    if(x2 < 0 || y2 < 0 || x2 >= TAB_X || y2 >= TAB_Y)
        return false;
    // Movimenta corretamente a localização [2][6]
    if(x1 == 2 && y1 == 6) {
        if(x2 < x1)
            x2 -= 1;
        else if(x2 > x1)
            x2 += 1;
    }
    // Impede movimentos para as posicoes irregulares
    if((x1 == 1 || x1 == 3) && ((y1 == 4 && y2 > 4 && x2 != 2) || (y1 == 5 && y2 < 5 && x2 != 2)))
        return false;
    /* Movimentação para posição que não existe */
    if(tabuleiro[x2][y2] == 'f')
        return false;
    /* Movimentação das peças da toca */
    if(y1 > 4 && (x1 != 2 && y1 != 5)) {
        // Peças na parte inferior da toca
        if(x1 == 3 && y1 == 5 && (
            (x2 == 3 && y2 == 4) ||
            (x2 == 4 && y2 == 4) ||
            (x2 == 4 && y2 == 5) ||
            (x2 == 2 && y2 == 6)
           ))
           return false;
        if(x1 == 1 && y1 == 5 && (
            (x2 == 1 && y2 == 4) ||
            (x2 == 0 && y2 == 4) ||
            (x2 == 0 && y2 == 5) ||
            (x2 == 2 && y2 == 6)
           ))
           return false;
        if(x1 == 0 && y1 == 6 && (
            (x2 != 1 && y2 != 5) &&
            (x2 != 2 && y2 != 6)
           ))
           return false;
        if(x1 == 4 && y1 == 6 && (
            (x2 != 3 && y2 != 5) &&
            (x2 != 2 && y2 != 6)
           ))
           return false;
        if(x1 == 2 && y1 == 6 && (
            (x2 != 0 && y2 != 6) &&
            (x2 != 4 && y2 != 6) &&
            (x2 != 2 && y2 != 5)
           ))
            return false;
    }
    else if( /* Verifica se a peca é restrita */
        ((!par(x1) && par(y1)) || (par(x1) && !par(y1)))
        /* Verifica se o movimento é diagonal */
        && (x2 != x1 && y2 != y1)
        ) {
        /* Movimento invalido: peça restrita tentou andar diagonalmente */
        return false;
    }
    /* Verifica se a posição está livre */
    if(tabuleiro[x2][y2] != 'n')
        return false;
    return true;
}

void atualizar_movimento(int i, int j, int x, int y, bool global_position) {
    char keys[3][3] = {'7', '8', '9', '4', '5', '6', '1', '2', '3'};
    if(keys[i][j] != '5') {
        if(global_position)
            movimentos[keys[i][j]] = {x, y};
        else
            temp[keys[i][j]] = {x, y};
    }
}

void limpar_movimentos(bool global_position) {
    // Iterador para atualizar valores do mapa
    for(int i = 0; i < 3; i++)
        for(int j = 0; j < 3; j++)
            if(i != '5')
                atualizar_movimento(i, j, -1, -1, global_position);
}

int obter_movimentos(char player, bool global_position) {
    // Contador de movimentos
    int c = 0;
    limpar_movimentos(global_position); // Restaura o mapa de movimentos
    for(int i = cursor.x - 1; i <= cursor.x + 1; i++) {
        for(int j = cursor.y - 1; j <= cursor.y + 1; j++)
            if(validar_posicao(cursor.x, cursor.y, i, j) || /* Verifica se a movimentação para 'c' é válida */
               (player == 'o' && validar_posicao(i, j, i + (i - cursor.x), j + (j - cursor.y)))) { /* Verifica se a movimentação para 'o' é válida */
                if(player == 'o') {
                    if(tabuleiro[i][j] == 'n') {
                        atualizar_movimento(i - (cursor.x - 1), j - (cursor.y - 1), i, j, global_position);
                        c++;
                    } else if(tabuleiro[i][j] == 'c') {
                        int jump_x = i + (i - cursor.x);
                        int jump_y = j + (j - cursor.y);
                        if(validar_posicao(cursor.x, cursor.y, jump_x, jump_y)
                           && tabuleiro[jump_x][jump_y] == 'n') {
                            atualizar_movimento(i - (cursor.x - 1), j - (cursor.y - 1), jump_x, jump_y, global_position);
                            c++;
                        }
                    }
                } else if(player == 'c') {
                    if(tabuleiro[i][j] == 'n') {
                        atualizar_movimento(i - (cursor.x - 1), j - (cursor.y - 1), i, j, global_position);
                        c++;
                    }
                }
            }
    }
    return c;
}

void selecionar_pos() {
    if(cursor.player != 'o') {
        for(int i = 0; i < TAB_X; i++)
            for(int j = 0; j < TAB_Y; j++)
                if(tabuleiro[i][j] == 'c') {
                    cursor.x = i;
                    cursor.y = j;
                    return;
                }
    } else {
        for(int i = 0; i < TAB_X; i++)
            for(int j = 0; j < TAB_Y; j++)
                if(tabuleiro[i][j] == 'o') {
                    cursor.x = i;
                    cursor.y = j;
                    return;
                }
    }
}

void atualizar_parametros() {
    // Passa para o proximo jogador
    N_JOGADAS++;
    cursor.player = cursor.player == 'o' ? 'c' : 'o';
    cursor.holding = false;
    selecionar_pos();
}

void Atualiza_desenho(void){
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1.0f, 0.0f, 0.0f);

	desenha_tabuleiro();
    desenha_pecas();
    desenha_cursor();

	glFlush();
}

bool mover_cursor(int x1, int y1, int x2, int y2) {
    // Verifica se esta dentro do tabuleiro
    if(x2 < 0 || y2 < 0 || x2 >= TAB_X || y2 >= TAB_Y)
        return false;
    // Movimenta corretamente a localização [2][6]
    if(x1 == 2 && y1 == 6) {
        if(x2 < x1)
            x2 -= 1;
        else if(x2 > x1)
            x2 += 1;
    }
    if(x1 == 0 && y1 == 4 && x2 == 0 && y2 == 5)
        y2 = 6;
    if(x1 == 4 && y1 == 4 && x2 == 4 && y2 == 5)
        y2 = 6;

    cursor.x = x2;
    cursor.y = y2;

    return true;
}

void mover_seletor(int next_x, int next_y) {
    cursor.next_x = next_x;
    cursor.next_y = next_y;
}

// Estrutura da arvore de decisão
class Node{
private:
    Tabuleiro cur_tab;
    int heuristica = 0;
    std::vector<Node> filhos;
public:
    Node(Tabuleiro tab, int x, int y, char player, int depth, int cur_depth = INT_MAX) {
        char foo;
        //heuristica = 0;
        // Corrige o valor base de cur_depth (é uma gambiarra)
        if(cur_depth > depth)
            cur_depth = depth;
        cur_tab = tab;
        std::vector<Positions> pecas_movem = pecasMovimentam(tab, player);
        if (cur_depth == 0) {
            std::cout << "ok" << std::endl;
            calcularHeuristica(player, x, y);
            /*
            std::cout << "Node(" << x << "," << y << ") -> (H: " << heuristica << ", D: "
                        << cur_depth << "]" << std::endl << cur_tab << std::endl;
            */
        } else {
            // Verifica todos os movimentos possiveis para cada cachorro
            for(Positions p : pecas_movem) {
                //std::cout << "(" << p.x << ", " << p.y << ")" << std::endl;
                // Escolhe qual tipo de peca deve ter seus movimentos analisados
                cur_tab = tab; // Reinicia o tabuleiro para o estado atual
                cursor.x = p.x; cursor.y = p.y; // Atualiza os valores do cursor para a função obter_movimentos funcionar
                obter_movimentos(player, false);
                char next_player = player == 'o' ? 'c' : 'o';
                if(gdt.debug_mode) {
                    std::cout << "Movimentos obtidos para (" << p.x << "," << p.y << ", " << cur_tab[p.x][p.y] << ")" << ", com h(x): " << heuristica << std::endl;
                    for(char c = '1'; c <= '9'; c == '4' ? c+=2 : c++)
                        std::cout << "(" << c << "|" << temp[c].x << "," << temp[c].y << ")";
                    std::cout << std::endl;
                    std::cout << cur_tab << std::endl;
                }
                for(int c = '1'; c <= '9'; c++)
                    if(c != '5' && (temp[c].x != -1 || temp[c].y != -1)) {
                        // Atualiza as posicoes das peças
                        std::cout << "O[" << p.x <<"][" << p.y << "] = " << cur_tab[p.x][p.y] <<
                                     " to N[" << temp[c].x << "][" << temp[c].y << "] = " <<
                                     cur_tab[temp[c].x][temp[c].y] << std::endl;
                        cur_tab[p.x][p.y] = 'n';
                        cur_tab[temp[c].x][temp[c].y] = player;
                        filhos.push_back(Node(cur_tab, temp[c].x, temp[c].y, next_player, depth, cur_depth - 1));
                        cur_tab = tab;
                    }
                std::cout << "-----------------------------------------------------------------------------------)" << std::endl;
            }
        }

        if (cursor.player != player)
            heuristica *= -1;
    }

    std::vector<Positions> pecasMovimentam(Tabuleiro tab, char peca) {
        std::vector<Positions> pecas_movem;
        for(int i = 0; i < TAB_X; i++)
            for(int j = 0; j < TAB_Y; j++)
                if(tab[i][j] == peca) {
                    cursor.x = i;
                    cursor.y = j;
                    if(obter_movimentos(peca, false))
                        pecas_movem.push_back({i, j});
                }
        return pecas_movem;
    }

    void calcularHeuristica(char peca, int x, int y) {
        if(peca == 'c') {
            heuristica = 8;
            for(int i = x - 1; i <= x + 1; i++) {
                for(int j = y - 1; j <= y + 1; j++) {
                    /* Verifica se o cachorro tem muitas opções de movimentação
                        e se é ameaçado pela onça
                    */
                    if(validar_posicao(x, y, i, j)) {
                        if(cur_tab[i][j] == 'o' &&
                           validar_posicao(i, j, i + (i-x), j + (j - y)))
                            heuristica -= 8 + (int)pow((14 - gdt.pcs_c), 2);
                    }
                }
            }
        } else {
            for(int i = x - 1; i <= x + 1; i++)
                for(int j = y - 1; j <= y + 1; j++)
                    /* Verifica se o cachorro tem muitas opções de movimentação
                        e se é ameaçado pela onça
                    */
                    if(validar_posicao(x, y, i, j)) {
                        if(cur_tab[i][j] == 'n')
                            heuristica++;
                        else if(validar_posicao(i, j, i + (i-x), j + (j - y)) && cur_tab[i][j] != 'f') {
                            heuristica += (int)pow((14 - gdt.pcs_c), 2);
                        } else {
                            heuristica -= 2;
                        }
                    }
        }
        std::cout << "h(x) para (" << x << "," << y << ") = " << heuristica << std::endl;
    }

public:
    int getHeuristica() { return heuristica; }

    Tabuleiro getCurTab() { return cur_tab; }

    int getChildSize() { return filhos.size(); }

    Node getChild(int index) {
        // Substituir por try/catch
        if(index >= 0 && index < filhos.size())
            return filhos[index];
    }

    void print(Node node, int depth = 0) {
        std::cout << node.getCurTab() << std::endl;
        for(int i = 0; i < node.getChildSize(); i++)
            print(node.getChild(i), depth + 1);
    }
};

ResultMinimax minimax(int cd, Node node, bool maximizing_player, int alpha, int beta) {
    ResultMinimax rmmx;
    static bool first_exc = false;
    static int td;
    char player;
    // Seta o total_depth (td) uma única vez, na 1a execucao
    if(!first_exc) {
        player = cursor.player;
        td = cd;
        first_exc = true;
    } else {
        player = (player == 'o' ? 'c' : 'o');
    }

    if (cd == 0) {
        rmmx = {node.getCurTab(), node.getHeuristica()};
        return rmmx;
    }

    if(maximizing_player) {
        int best = MIN;

        for(int i = 0; i < node.getChildSize(); i++) {
            ResultMinimax val = minimax(cd - 1, node.getChild(i), false, alpha, beta);
            best = max(best, val.heuristica);
            alpha = max(alpha, best);

            rmmx.heuristica = best;

            if(beta <= alpha) {
                rmmx = {node.getChild(i).getCurTab(), node.getChild(i).getHeuristica()};
                break;
            }
        }
        if(cd == td - 1)
        return rmmx;
    } else {
        int best = MAX;

        for(int i = 0; i < node.getChildSize(); i++) {
            ResultMinimax val = minimax(cd - 1, node.getChild(i), false, alpha, beta);
            best = min(best, val.heuristica);
            alpha = min(alpha, best);

            rmmx.heuristica = best;

            if(beta <= alpha) {
                rmmx = {node.getChild(i).getCurTab(), node.getChild(i).getHeuristica()};
                break;
            }
        }
        return rmmx;
    }
}

void maquina_joga() {
    if(gdt.mode == 'm' && cursor.player == gdt.player2) {
        // clock_t b = clock();
        system("cls");
        Node node(tabuleiro, cursor.x, cursor.y, gdt.player2, 2*(gdt.difficulty - 48));
        //node.print(node);
        ResultMinimax rmmx = minimax(3, node, true, MIN, MAX);
        std::cout << "-----------------------------------------"
                  << "\nResultado: " << rmmx.heuristica << std::endl;
        std::cout << rmmx.tab << std::endl;
        // clock_t e = clock();
        tabuleiro = rmmx.tab;
        atualizar_parametros();
    }
}

void Teclado( unsigned char tecla, int x, int y){
    int next_x = -1,
        next_y = -1;
    switch (tecla){
        case 27 :
        case 'q':
            exit(0);
        break;
        case 'w':
            cursor.next_x = cursor.x - 1;
            cursor.next_y = cursor.y;
            break;
        case 'a':
            cursor.next_x = cursor.x;
            cursor.next_y = cursor.y - 1;
            break;
        case 's':
            cursor.next_x = cursor.x + 1;
            cursor.next_y = cursor.y;
            break;
        case 'd':
            cursor.next_x = cursor.x;
            cursor.next_y = cursor.y + 1;
            break;
    }
    if(tecla >= '1' && tecla <= '9' && tecla != '5'
        && validar_posicao(cursor.x, cursor.y, movimentos[tecla].x, movimentos[tecla].y)) {
            cursor.direction = tecla;
            mover_seletor(movimentos[tecla].x, movimentos[tecla].y);
    }
    if(tecla == 'w' || tecla == 'a' || tecla == 's' || tecla == 'd') {
        mover_cursor(cursor.x, cursor.y, cursor.next_x, cursor.next_y);
        if(gdt.debug_mode)
            std::cout << "Mover Cursor: " << mover_cursor(cursor.x, cursor.y, next_x, next_y) << std::endl;
    }
    if(tecla == 'k') {
        mover_peca(cursor.x, cursor.y, cursor.next_x, cursor.next_y);
        atualizar_parametros();
        // Realiza o movimento da maquina, se ela existir
        maquina_joga();
    }
    if(tecla == 'h') {
        if(tabuleiro[cursor.x][cursor.y] == cursor.player)
            cursor.holding = !cursor.holding;
        obter_movimentos(cursor.player, true);
    }
    if(N_JOGADAS == 0 && tecla == 'b' && gdt.player1 == 'c') {
        Sleep(1000);
        int foo = rand() % 3;
        tabuleiro[1 + foo][3] = tabuleiro[cursor.x][cursor.y];
        tabuleiro[cursor.x][cursor.y] = 'n';
        atualizar_parametros();
    }

    glutPostRedisplay();
}

void menu() {
    char menu_input = 'f';
    while(gdt.mode != 'm' && gdt.mode != 'p') {
        std::cout << "Escolha o modo de jogo: " << std::endl
                  << "1. PVM" << std::endl
                  << "2. PVP" << std::endl
                  << "> ";
        std::cin >> gdt.mode;
        gdt.mode = (gdt.mode == '1' ? 'm' : 'p');
        system("cls");
    }

    if(gdt.mode == 'm') {
        while(gdt.difficulty != '1' && gdt.difficulty != '2' && gdt.difficulty != '3') {
            std::cout << "Escolha a dificuldade do jogo: " << std::endl
                      << "1. Easy" << std::endl
                      << "2. Medium" << std::endl
                      << "3. Hard" << std::endl
                      << "> ";
            std::cin >> gdt.difficulty;
            system("cls");
        }
        while(gdt.player1 == 'n' || (gdt.player1 != 'c' && gdt.player1 != 'C' && gdt.player1 == 'o' && gdt.player1 == 'O')) {
            std::cout << "[Player 1] Escreva 'c' para jogar como cachorros e 'o' para oncas: ";
            std::cin >> gdt.player1;
            system("cls");
        }
        gdt.player1 = (gdt.player1 == 'c' || gdt.player1 == 'C') ? 'c' : 'o';
        gdt.player2 = (gdt.player1 == 'c' || gdt.player1 == 'C') ? 'o' : 'c';
    }
    while(menu_input != 'S' && menu_input != 's' && menu_input != 'n' && menu_input != 'N') {
        std::cout << "Ativar modo debug? (S/N)" << std::endl
                  << "> ";
        std::cin >> menu_input;
        system("cls");
    }
    if(menu_input == 'S')
        gdt.debug_mode = true;
}

int main(int argc, char *argv[]){
	menu();
	obter_movimentos(cursor.player, true);
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

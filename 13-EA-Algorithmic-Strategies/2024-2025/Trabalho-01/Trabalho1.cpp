#include <iostream>
#include <vector>
#include <climits>
#include <algorithm>
#include <cstdlib>

using namespace std;

// Estrutura para representar uma posição no grid
struct Pos {
    int row, col;
};

int nRows, nCols, bestSolution;
vector<vector<char>> board;
vector<Pos> posts;     // Guarda os outposts (caracteres '0'-'4')
vector<Pos> empties;   // Células vazias onde podemos colocar torres

// Para cada outpost, armazenamos os índices (na ordem de 'empties')
// das células que são adjacentes (distância 1)
vector<vector<int>> postAdj;

// Vetores de direção para cima, baixo, esquerda e direita
const int dRow[4] = {-1, 1, 0, 0};
const int dCol[4] = {0, 0, -1, 1};

// Função que verifica se (r, c) está dentro do board
bool isInside(int r, int c) {
    return (r >= 0 && r < nRows && c >= 0 && c < nCols);
}

// Verifica se é seguro colocar uma torre na célula (r, c)
// ou seja, não deve existir outra torre na linha de visão
bool safeToPlace(int r, int c, const vector<vector<char>> &b) {
    for (int d = 0; d < 4; d++) {
        int nr = r + dRow[d], nc = c + dCol[d];
        while (isInside(nr, nc)) {
            if (b[nr][nc] == 'T')
                return false; // Já há uma torre na linha de visão
            if (b[nr][nc] == '#' || (b[nr][nc] >= '0' && b[nr][nc] <= '4'))
                break;
            nr += dRow[d];
            nc += dCol[d];
        }
    }
    return true;
}

// Verifica se todas as células vazias estão cobertas por alguma torre
bool checkCoverage(const vector<vector<char>> &b) {
    for (int r = 0; r < nRows; r++) {
        for (int c = 0; c < nCols; c++) {
            if (b[r][c] == '.') {
                bool coberta = false;
                for (int d = 0; d < 4; d++) {
                    int nr = r + dRow[d], nc = c + dCol[d];
                    while (isInside(nr, nc)) {
                        if (b[nr][nc] == 'T') {
                            coberta = true;
                            break;
                        }
                        if (b[nr][nc] == '#' || (b[nr][nc] >= '0' && b[nr][nc] <= '4'))
                            break;
                        nr += dRow[d];
                        nc += dCol[d];
                    }
                    if (coberta) break;
                }
                if (!coberta)
                    return false;
            }
        }
    }
    return true;
}

// Verifica se cada outpost possui exatamente o número de torres adjacentes exigido
bool checkPosts(const vector<vector<char>> &b) {
    for (auto op : posts) {
        int need = b[op.row][op.col] - '0';
        int count = 0;
        for (int d = 0; d < 4; d++) {
            int nr = op.row + dRow[d], nc = op.col + dCol[d];
            if (isInside(nr, nc) && b[nr][nc] == 'T')
                count++;
        }
        if (count != need)
            return false;
    }
    return true;
}

// Função de backtracking
// index: posição atual no vetor 'empties'
// count: número de torres colocadas até o momento
// outCount[i]: número de torres já adjacentes ao outpost i
void backtrack(int index, int count, vector<int> &outCount) {
    // Se já usamos torres em quantidade maior ou igual à melhor solução, aborta
    if (count >= bestSolution)
        return;
    
    // Poda incremental: para cada outpost, verifica se a solução é viável
    for (int i = 0; i < posts.size(); i++) {
        int need = board[posts[i].row][posts[i].col] - '0';
        if (outCount[i] > need)
            return; // Já excedeu o número necessário
        int disponivel = 0;
        // Conta quantas células adjacentes ainda não decididas podem contribuir para este outpost
        for (int idx : postAdj[i]) {
            if (idx >= index)
                disponivel++;
        }
        if (outCount[i] + disponivel < need)
            return; // Mesmo usando todas as células restantes, não atinge o necessário
    }
    
    // Se todas as células foram processadas, verifica se a solução é válida
    if (index == empties.size()) {
        if (checkPosts(board) && checkCoverage(board))
            bestSolution = min(bestSolution, count);
        return;
    }
    
    int r = empties[index].row, c = empties[index].col;
    
    // Opção 1: Não coloca torre nesta célula
    backtrack(index + 1, count, outCount);
    
    // Opção 2: Tenta colocar torre, se for seguro
    if (safeToPlace(r, c, board)) {
        board[r][c] = 'T';
        vector<int> modificados;
        // Atualiza o contador dos outposts adjacentes
        for (int i = 0; i < posts.size(); i++) {
            if (abs(r - posts[i].row) + abs(c - posts[i].col) == 1) {
                outCount[i]++;
                modificados.push_back(i);
            }
        }
        backtrack(index + 1, count + 1, outCount);
        // Volta atrás (backtracking): reverte as alterações
        for (int i : modificados)
            outCount[i]--;
        board[r][c] = '.';
    }
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    int T;
    cin >> T;
    
    while (T--) {
        cin >> nRows >> nCols;
        board.assign(nRows, vector<char>(nCols));
        posts.clear();
        empties.clear();
        bestSolution = INT_MAX;
        
        // Leitura do grid e separação de outposts e células vazias
        for (int i = 0; i < nRows; i++){
            for (int j = 0; j < nCols; j++){
                cin >> board[i][j];
                if (board[i][j] == '.')
                    empties.push_back({i, j});
                else if (board[i][j] >= '0' && board[i][j] <= '4')
                    posts.push_back({i, j});
            }
        }
        
        // Ordena as células vazias para dar prioridade às mais próximas dos outposts
        sort(empties.begin(), empties.end(), [&](const Pos &a, const Pos &b) {
            int minA = INT_MAX, minB = INT_MAX;
            for (auto op : posts) {
                minA = min(minA, abs(a.row - op.row) + abs(a.col - op.col));
                minB = min(minB, abs(b.row - op.row) + abs(b.col - op.col));
            }
            return minA < minB;
        });
        
        // Para cada outpost, regista os índices de 'empties' que são adjacentes
        postAdj.clear();
        postAdj.resize(posts.size());
        for (int i = 0; i < empties.size(); i++){
            int r = empties[i].row, c = empties[i].col;
            for (int j = 0; j < posts.size(); j++){
                if (abs(r - posts[j].row) + abs(c - posts[j].col) == 1)
                    postAdj[j].push_back(i);
            }
        }
        
        vector<int> outCount(posts.size(), 0);
        backtrack(0, 0, outCount);
        
        if (bestSolution == INT_MAX)
            cout << "noxus will rise!\n";
        else
            cout << bestSolution << "\n";
    }
    
    return 0;
}

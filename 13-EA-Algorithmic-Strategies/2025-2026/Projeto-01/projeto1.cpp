#include <bits/stdc++.h>
using namespace std;

int n;
int pecas[20][3]; // cada peça: 3 números por ordem não decrescente

// Codificação de aresta dirigida: x*100+y significa x→y.
// A peça [a,b,c] (a<=b<=c) colocada no sentido horário tem arestas: a→b, b→c, c→a.
// Duas peças partilham um lado físico quando uma tem p→q e a outra q→p.
// Pontuação de um encaixe = p+q.
inline int codAresta(int x, int y) { return x * 100 + y; }
inline int arestaInversa(int cod) { return (cod % 100) * 100 + (cod / 100); }
inline int pontuacaoAresta(int cod) { return (cod / 100) + (cod % 100); }

int arestas[20][3];    // código orientado de cada lado
int invArestas[20][3]; // código inverso de cada lado (para procurar no pool)
int ptsSide[20][3];    // pontuação de cada lado

// disponiveis[cod] = número de slots de aresta abertos na mesa com código cod.
// Códigos vão de 0 a 505 (x*100+y com x,y em [0,5]).
int disponiveis[506];

// totalC[c] = disponiveis[c] + número de lados de peças não colocadas com código c.
// Mantido de forma incremental: só muda para lados matched.
int totalC[506];

bool usada[20];
int melhorPontuacao;

// para cada par de códigos (c, inv_c), o número máximo de encaixes é min(totalC[c], totalC[inv_c]).
inline int computeBound() {
    int b = 0;
    for (int x = 0; x <= 5; x++) { 
        b += 2 * x * (totalC[x * 100 + x] / 2); // par reflexivo x->x
        for (int y = x + 1; y <= 5; y++) {
            int tc = totalC[x * 100 + y], ti = totalC[y * 100 + x];
            b += (x + y) * (tc < ti ? tc : ti);
        }
    }
    return b;
}

void backtrack(int pontuacaoAtual, bool mesaVazia) {
    if (pontuacaoAtual > melhorPontuacao)
        melhorPontuacao = pontuacaoAtual;

    // Poda: se apontuação atual + limite otimista para o que falta colocar melhora melhorPontuacao skip.
    if (pontuacaoAtual + computeBound() <= melhorPontuacao)
        return;

    for (int i = 0; i < n; i++) {
        if (usada[i]) continue;

        if (mesaVazia) {
            // Primeira peça colocada livremente: todos os 3 lados ficam abertos na mesa.
            // totalC não muda: cada lado passa de "peça não colocada" para "pool" — soma igual.
            usada[i] = true;
            for (int s = 0; s < 3; s++) disponiveis[arestas[i][s]]++;
            backtrack(pontuacaoAtual, false);
            for (int s = 0; s < 3; s++) disponiveis[arestas[i][s]]--;
            usada[i] = false;
        } else {
            // Tentar colocar a peça i encaixando num subconjunto dos seus 3 lados.
            // Uma peça pode encaixar em vários trikits simultaneamente quando colocada
            // numa "bolsa" entre peças já na mesa, marcando pontos em todos os lados.
            //
            // Para cada subconjunto mask (1..7):
            //   - Lados no mask: encaixados (consomem slot do pool do inverso)
            //   - Lados fora do mask:  ficam abertos na mesa (adicionados ao pool)
            //
            // Lados com inv codes repetidos no mesmo mask (peças [a,a,a]):
            //   exige contagem suficiente no pool.
            for (int mask = 7; mask >= 1; mask--) { 
                bool ok = true;
                for (int s = 0; s < 3 && ok; s++) {
                    if (!(mask >> s & 1)) continue; // lado s não encaixa, não consome do pool
                    int jaUsados = 0;
                    for (int ss = 0; ss < s; ss++) // conta quantos lados ss < s também estão no mask e têm o mesmo inv code (caso de peças [a,a,a])
                        if ((mask >> ss & 1) && invArestas[i][ss] == invArestas[i][s]) jaUsados++;
                    if (disponiveis[invArestas[i][s]] <= jaUsados) ok = false; // não tem slots suficientes no pool para encaixar esse lado s
                }
                if (!ok) continue; // máscara inválida: tenta próximo subconjunto

                int pontos = 0;
                usada[i] = true;
                for (int s = 0; s < 3; s++) { // atualiza disponiveis e totalC para refletir a colocação da peça i com máscara mask
                    if (mask >> s & 1) {
                        disponiveis[invArestas[i][s]]--;
                        totalC[arestas[i][s]]--;    // lado s sai de "não colocada"
                        totalC[invArestas[i][s]]--; // slot inv_s consumido do pool
                        pontos += ptsSide[i][s];
                    } else {
                        disponiveis[arestas[i][s]]++;
                        // totalC[arestas[i][s]] inalterado: "não colocada" → "pool", soma zero
                    }
                }

                backtrack(pontuacaoAtual + pontos, false);

                for (int s = 0; s < 3; s++) { // undo alterações em disponiveis e totalC após a chamada recursiva
                    if (mask >> s & 1) {
                        disponiveis[invArestas[i][s]]++; 
                        totalC[arestas[i][s]]++;
                        totalC[invArestas[i][s]]++;
                    } else {
                        disponiveis[arestas[i][s]]--;
                    }
                }
                usada[i] = false;
            }
        }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    n = 0;
    while (cin >> pecas[n][0] >> pecas[n][1] >> pecas[n][2]) {
        int a = pecas[n][0], b = pecas[n][1], c = pecas[n][2];
        // Ordena para garantir a<=b<=c (necessário para a codificação horária)
        if (a > b) swap(a, b);
        if (b > c) swap(b, c);
        if (a > b) swap(a, b);
        // Arestas dirigidas no sentido horário: a→b, b→c, c→a
        arestas[n][0] = codAresta(a, b);
        arestas[n][1] = codAresta(b, c);
        arestas[n][2] = codAresta(c, a);
        for (int s = 0; s < 3; s++) {
            invArestas[n][s] = arestaInversa(arestas[n][s]);
            ptsSide[n][s]    = pontuacaoAresta(arestas[n][s]);
        }
        n++;
    }

    // Ordena peças por valor decrescente (a+b+c) para explorar soluções de alta pontuação primeiro,
    {
        int idx[20];
        for (int i = 0; i < n; i++) idx[i] = i;
        sort(idx, idx + n, [](int a, int b){
            return (pecas[a][0]+pecas[a][1]+pecas[a][2]) > (pecas[b][0]+pecas[b][1]+pecas[b][2]);
        });
        int tmpP[20][3], tmpA[20][3], tmpI[20][3], tmpS[20][3];
        for (int i = 0; i < n; i++) {
            for (int j=0;j<3;j++) tmpP[i][j]=pecas[idx[i]][j];
            for (int j=0;j<3;j++) tmpA[i][j]=arestas[idx[i]][j];
            for (int j=0;j<3;j++) tmpI[i][j]=invArestas[idx[i]][j];
            for (int j=0;j<3;j++) tmpS[i][j]=ptsSide[idx[i]][j];
        }
        for (int i = 0; i < n; i++) {
            for (int j=0;j<3;j++) pecas[i][j]=tmpP[i][j];
            for (int j=0;j<3;j++) arestas[i][j]=tmpA[i][j];
            for (int j=0;j<3;j++) invArestas[i][j]=tmpI[i][j];
            for (int j=0;j<3;j++) ptsSide[i][j]=tmpS[i][j];
        }
    }

    // Inicializa totalC com os lados de todas as peças (pool vazio no início)
    memset(totalC, 0, sizeof(totalC));
    for (int i = 0; i < n; i++)
        for (int s = 0; s < 3; s++)
            totalC[arestas[i][s]]++;

   
    // Para cada escolha de primeira peça, colocar sempre a peça com o encaixe mais pontuado.
    melhorPontuacao = 0;
    for (int first = 0; first < n; first++) {
        bool tmpUsada[20] = {};
        int  tmpDisp[506] = {};
        tmpUsada[first] = true;
        for (int s = 0; s < 3; s++) tmpDisp[arestas[first][s]]++;
        int score = 0;
        bool progresso = true;
        while (progresso) {
            progresso = false;
            int melhorTile = -1, melhorPts = -1, melhorMask = 0;
            for (int i = 0; i < n; i++) {
                if (tmpUsada[i]) continue;
                for (int mask = 7; mask >= 1; mask--) { 
                    bool ok = true;
                    int pontos = 0;
                    for (int s = 0; s < 3 && ok; s++) {
                        if (!(mask >> s & 1)) continue; // lado s não encaixa, não consome do pool
                        int jaUsados = 0;
                        for (int ss = 0; ss < s; ss++)
                            if ((mask >> ss & 1) && invArestas[i][ss] == invArestas[i][s]) jaUsados++;
                        if (tmpDisp[invArestas[i][s]] <= jaUsados) ok = false;
                        else pontos += ptsSide[i][s];
                    }
                    if (!ok || pontos == 0) continue; // máscara inválida ou sem pontos: tenta próximo subconjunto
                    if (pontos > melhorPts) { melhorPts = pontos; melhorTile = i; melhorMask = mask; }
                }
            }
            if (melhorTile == -1) break;
            progresso = true;
            score += melhorPts;
            tmpUsada[melhorTile] = true;
            for (int s = 0; s < 3; s++) {
                if (melhorMask >> s & 1) tmpDisp[invArestas[melhorTile][s]]--;
                else tmpDisp[arestas[melhorTile][s]]++;
            }
        }
        if (score > melhorPontuacao) melhorPontuacao = score;
    }

    memset(usada, false, sizeof(usada));
    memset(disponiveis, 0, sizeof(disponiveis));

    backtrack(0, true);

    cout << melhorPontuacao << "\n";
    return 0;
}

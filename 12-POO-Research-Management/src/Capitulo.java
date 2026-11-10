/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package projetopoo;

import java.io.Serializable;

/**
 * Subclasse de Livro que caracteriza um Capitulo de Livro
 * @author Diogo Henriques
 */
public class Capitulo extends Livro implements Serializable {

    private String nomecapitulo;
    private int paginicial, pagfinal;

    /**
     * Construtor da subclasse de Livro, capitulo
     *
     * @param nomecapitulo string com o nome do capitulo
     * @param paginicial valor inteiro da páginal inicial do capitulo
     * @param pagfinal valor inteiro da página final do capitulo
     * @param editora Herdado de Livro
     * @param titulo Herdado de Publicação
     * @param tipo Herdado de Publicação
     * @param resumo Herdado de Publicação
     * @param dim Herdado de Publicação
     * @param ano Herdado de Publicação
     */
    public Capitulo(String nomecapitulo, int paginicial, int pagfinal, String editora, String titulo, String tipo, String resumo, int dim, int ano) {
        super(editora, titulo, tipo, resumo, dim, ano);
        this.nomecapitulo = nomecapitulo;
        this.paginicial = paginicial;
        this.pagfinal = pagfinal;
    }

    @Override
    public String toString() {
        return "Nome do Capítulo: " + nomecapitulo + "; Livro: " + getTitulo() + "; Página Inicial: " + paginicial + "; Página Final: " + pagfinal + "; Editora: " + getEditora() + "; Tipo: " + getTipo() + "; Resumo: " + getResumo()
                + "; Fator de Impacto: " + fatorImpacto(getDim()) + "; Ano: " + getAno();
    }

    @Override
    public char fatorImpacto(int dim) {
        char fi;
        if (dim >= 10000) {
            fi = 'A';

        } else if (dim < 10000 && dim >= 5000) {
            fi = 'B';
        } else {
            fi = 'C';
        }
        return fi;

    }

}

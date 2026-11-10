/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package projetopoo;

import java.io.Serializable;

/**
 * Subclasse de Publicacao que caracteriza um Livro
 *
 * @author Diogo Henriques
 */
public class Livro extends Publicacao implements Serializable {

    private String editora;

    /**
     * Construtor da classe livro
     *
     * @param editora string com o nome da editora do livro
     * @param titulo Herdado de Publicação
     * @param tipo Herdado de Publicação
     * @param resumo Herdado de Publicação
     * @param dim Herdado de Publicação
     * @param ano Herdado de Publicação
     */
    public Livro(String editora, String titulo, String tipo, String resumo, int dim, int ano) {
        super(titulo, tipo, resumo, dim, ano);
        this.editora = editora;
    }

    /**
     * Obtém a editora do Livrp
     *
     * @return String com o nome da editora
     */
    public String getEditora() {
        return editora;
    }

    /**
     * Define a editora do livro
     *
     * @param editora string com o nome da editora a definir
     */
    public void setEditora(String editora) {
        this.editora = editora;
    }

    @Override
    public String toString() {
        return "Título do Livro: " + getTitulo() + "; Editora: " + getEditora() + "; Tipo: " + getTipo() + "; Resumo: " + getResumo()
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

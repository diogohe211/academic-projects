/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package projetopoo;

import java.io.Serializable;

/**
 * Subclasse de Livro que caracteriza um Livro de Conferência
 *
 * @author Diogo Henriques
 */
public class LivroConferencia extends Livro implements Serializable {

    private String nomeconf;
    int numartigos;

    /**
     * Construtor da subclasse de Livro, LivroConferencia
     *
     * @param nomeconf string com o nome da conferência
     * @param numartigos valor inteiro de artigos
     * @param editora Herdado de Livro
     * @param titulo Herdado de Publicação
     * @param tipo Herdado de Publicação
     * @param resumo Herdado de Publicação
     * @param dim Herdado de Publicação
     * @param ano Herdado de Publicação
     */
    public LivroConferencia(String nomeconf, int numartigos, String editora, String titulo, String tipo, String resumo, int dim, int ano) {
        super(editora, titulo, tipo, resumo, dim, ano);
        this.nomeconf = nomeconf;
        this.numartigos = numartigos;
    }

    @Override
    public String toString() {
        return "Nome da Conferência: " + nomeconf + "; Número de artigos: " + numartigos + "; Titulo do Livro da Conferência: " + getTitulo() + "; Editora: " + getEditora() + "; Tipo: " + getTipo() + "; Resumo: " + getResumo() + "; Fator de Impacto: " + fatorImpacto(getDim()) + "; Ano: " + getAno();
    }

}

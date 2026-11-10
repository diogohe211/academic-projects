/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package projetopoo;

import java.io.Serializable;

/**
 * Subclasse de Publicacao que, caracteriza uma revista
 *
 * @author Diogo Henriques
 */
public class Revista extends Publicacao implements Serializable {

    private String nomerevista;
    private Data datarevista;
    private int num;

    /**
     * Construtor da subclasse de Publicacao, Rveista
     *
     * @param nomerevista string com o nome da revista
     * @param datarevista objeto Data com a data de publicação da revista
     * @param num valor inteiro do número da revista
     * @param titulo Herdado de Publicacao
     * @param tipo Herdado de Publicacao
     * @param resumo Herdado de Publicacao
     * @param dim Herdado de Publicacao
     * @param ano Herdado de Publicacao
     */
    public Revista(String nomerevista, Data datarevista, int num, String titulo, String tipo, String resumo, int dim, int ano) {
        super(titulo, tipo, resumo, dim, ano);
        this.nomerevista = nomerevista;
        this.datarevista = datarevista;
        this.num = num;
    }

    @Override
    public String toString() {
        return "Nome da Revista: " + nomerevista + "; Número da revista: " + num + "; Titulo do artigo: " + getTitulo() + "; Data: " + datarevista.toString() + "; Tipo: " + getTipo() + "; Resumo: " + getResumo() + "; Fator de Impacto: " + fatorImpacto(getDim()) + "; Ano: " + getAno();
    }

    @Override
    public char fatorImpacto(int dim) {
        char fi;
        if (dim >= 1000) {
            fi = 'A';

        } else if (dim < 1000 && dim >= 500) {
            fi = 'B';
        } else {
            fi = 'C';
        }
        return fi;
    }

}

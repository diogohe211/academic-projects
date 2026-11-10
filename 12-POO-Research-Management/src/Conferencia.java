/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package projetopoo;

import java.io.Serializable;

/**
 * Subclasse de Publicacao que caracteriza uma Conferência
 *
 * @author Diogo Henriques
 */
public class Conferencia extends Publicacao implements Serializable {

    private Data dataconferencia;
    private String nomeconferencia, local;

    /**
     * Construtor da subclasse de Publicação, Conferência
     *
     * @param dataconferencia objeto Data com a data da conferência
     * @param nomeconferencia string com o nome da conferência
     * @param local string com o local da conferência
     * @param titulo Herdado de Publicacao
     * @param tipo Herdado de Publicacao
     * @param resumo Herdado de Publicacao
     * @param dim Herdado de Publicacao
     * @param ano Herdado de Publicacao
     */
    public Conferencia(Data dataconferencia, String nomeconferencia, String local, String titulo, String tipo, String resumo, int dim, int ano) {
        super(titulo, tipo, resumo, dim, ano);
        this.dataconferencia = dataconferencia;
        this.nomeconferencia = nomeconferencia;
        this.local = local;
    }

    @Override
    public String toString() {

        return "Nome da Conferência: " + nomeconferencia + "; Data: " + dataconferencia + "; Local da Conferência: " + local + "; " + "Titulo do artigo: " + getTitulo() + "; Tipo: " + getTipo() + "; Resumo: " + getResumo() + "; Fator de Impacto: " + fatorImpacto(getDim()) + "; Ano: " + getAno();
    }

    @Override
    public char fatorImpacto(int dim) {
        char fi;
        if (dim >= 500) {
            fi = 'A';

        } else if (dim < 500 && dim >= 200) {
            fi = 'B';
        } else {
            fi = 'C';
        }
        return fi;
    }

}

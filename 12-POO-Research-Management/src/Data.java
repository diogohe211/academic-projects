package projetopoo;

import java.io.Serializable;

/**
 * Classe que caracteriza uma data do tipo dd/mm/aaaa
 *
 * @author Diogo Henriques
 */
public class Data implements Serializable {

    private int dia, mes, ano;

    /**
     * Construtor da classe data
     *
     * @param dia número do dia
     * @param mes número do mês
     * @param ano número do ano
     */
    protected Data(int dia, int mes, int ano) {
        this.dia = dia;
        this.mes = mes;
        this.ano = ano;
    }

    @Override
    public String toString() {
        return dia + "/" + mes + "/" + ano;
    }

//Getters e Setters
    /**
     * Obtém o dia da data
     *
     * @return valor inteiro do dia
     */
    protected int getDia() {
        return dia;
    }

    /**
     * Define o dia da data
     *
     * @param dia valor inteiro do dia
     */
    protected void setDia(int dia) {
        this.dia = dia;
    }

    /**
     * Obtém o mês da data
     *
     * @return valor inteiro do mês
     */
    protected int getMes() {
        return mes;
    }

    /**
     * Define o mês da data
     *
     * @param mes valor inteiro do mês
     */
    protected void setMes(int mes) {
        this.mes = mes;
    }

    /**
     * Obtém o ano da publicação
     *
     * @return valor inteiro da publicação
     */
    protected int getAno() {
        return ano;
    }

    /**
     * Define o ano da publicação
     *
     * @param ano valor inteiro da publicação
     */
    protected void setAno(int ano) {
        this.ano = ano;
    }

}

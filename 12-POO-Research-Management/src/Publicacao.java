/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package projetopoo;

import java.io.Serializable;
import java.util.ArrayList;

/**
 * Classe que caracteriza uma publicação
 *
 * @author Diogo Henriques
 */
public abstract class Publicacao implements Serializable {

    private String titulo, tipo, resumo;
    private int dim, ano;
    private ArrayList<String> keywords;
    private ArrayList<String> investigadores;

    /**
     * Construtor da classe Publicação
     *
     * @param titulo titulo da publicação
     * @param tipo tipo de publicação
     * @param resumo resumo da publicação
     * @param dim dimensão da audiência da publicação
     * @param ano ano de publicação
     */
    public Publicacao(String titulo, String tipo, String resumo, int dim, int ano) {
        this.titulo = titulo;
        this.tipo = tipo;
        this.resumo = resumo;
        this.dim = dim;
        this.ano = ano;
        keywords = new ArrayList<>();
        investigadores = new ArrayList<>();
    }

    @Override
    public abstract String toString();

    /**
     * Obtém o resumo da publicação
     *
     * @return String do resumo
     */
    public String getResumo() {
        return resumo;
    }

    /**
     * Define o resumo de uma publicação
     *
     * @param resumo String do resumo
     */
    public void setResumo(String resumo) {
        this.resumo = resumo;
    }

    /**
     * Calcula o fator de impacto baseado no tipo de publicação e dimensão de
     * audiência
     *
     * @param dim valor inteiro da dimensão da audiência
     * @return caracter 'A','B' ou 'C
     */
    public abstract char fatorImpacto(int dim);

    /**
     * obtém o titulo da publicação
     *
     * @return string com o titulo
     */
    public String getTitulo() {
        return titulo;
    }

    /**
     * define o titulo da publicação
     *
     * @param titulo string com o titulo a definir
     */
    public void setTitulo(String titulo) {
        this.titulo = titulo;
    }

    /**
     * Obtém tipo de uma Publicação
     *
     * @return string contendo o tipo da publicação
     */
    public String getTipo() {
        return tipo;
    }

    /**
     * Define o tipo da publicação
     *
     * @param tipo string com o tipo de publicação
     */
    public void setTipo(String tipo) {
        this.tipo = tipo;
    }

    /**
     * Obtém a dimensão da audiência da publicação
     *
     * @return valor inteiro da audiência
     */
    public int getDim() {
        return dim;
    }

    /**
     * Define a dimensão da audiência da publicação
     *
     * @param dim valor inteiro da dimensão
     */
    public void setDim(int dim) {
        this.dim = dim;
    }

    /**
     * Obtém o ano da publicação
     *
     * @return valor inteiro do ano da publicação
     */
    public int getAno() {
        return ano;
    }

    /**
     * Define o ano da publicação
     *
     * @param ano valor inteiro do ano da publicação
     */
    public void setAno(int ano) {
        this.ano = ano;
    }

    /**
     * Obtém arraylist das keywords da publicação
     *
     * @return arraylist das strings que contém as keywords
     */
    public ArrayList<String> getKeywords() {
        return keywords;
    }

    /**
     * Define as keywords de uma publicação
     *
     * @param keywords arraylist de strings com keywords
     */
    public void setKeywords(ArrayList<String> keywords) {
        this.keywords = keywords;
    }

    /**
     * Obtém arraylist dos investigadores(autores) da publicação
     *
     * @return arraylist das strings que contém os nomes dos autores
     */
    public ArrayList<String> getInvestigadores() {
        return investigadores;
    }

    /**
     * Define os investigadores(autores) da publicação
     *
     * @param investigadores arraylist das strings que contém os nomes dos
     * autores
     */
    public void setInvestigadores(ArrayList<String> investigadores) {
        this.investigadores = investigadores;
    }

}

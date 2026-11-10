package projetopoo;

import java.io.Serializable;
import java.util.ArrayList;

/**
 * Classe que caracteriza um Grupo de Investigação da CISUC
 *
 * @author Diogo Henriques
 */
public class GrupoDeInvestigacao implements Serializable {

    private String nome, acr, responsavel;
    private ArrayList<String> investigadores;
    private ArrayList<Publicacao> publicacoes;

    /**
     * Construtor da classe GrupoDeInvestigacao
     *
     * @param nome nome do grupo
     * @param acr acrónimo do grupo
     * @param responsavel Nome do investigador responsável
     */
    public GrupoDeInvestigacao(String nome, String acr, String responsavel) {
        this.nome = nome;
        this.acr = acr;
        this.responsavel = responsavel;
        investigadores = new ArrayList<>();
        publicacoes = new ArrayList<>();
    }

//Getters e Setters
    protected String getNome() {
        return nome;
    }

    protected void setNome(String nome) {
        this.nome = nome;
    }

    protected String getAcr() {
        return acr;
    }

    protected void setAcr(String acr) {
        this.acr = acr;
    }

    protected String getResponsavel() {
        return responsavel;
    }

    protected void setResponsavel(String responsavel) {
        this.responsavel = responsavel;
    }

    protected ArrayList<Publicacao> getPublicacoes() {
        return publicacoes;
    }

    protected void setPublicacoes(ArrayList<Publicacao> publicacoes) {
        this.publicacoes = publicacoes;
    }

    protected ArrayList<String> getInvestigadores() {
        return investigadores;
    }

    protected void setInvestigadores(ArrayList<String> investigadores) {
        this.investigadores = investigadores;
    }

}

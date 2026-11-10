package projetopoo;

import java.io.Serializable;

/**
 * Classe que caracteriza um Investigador da CISUC
 *
 * @author Diogo Henriques
 */
abstract class Investigador implements Serializable {

    /**
     * Strings do nome, e-mail , grupo a que pertence e o tipo de investigador
     */
    protected String nome, mail, grupo, tipo;

    /**
     * Contrutor da classe investigador
     *
     * @param nome Nome do investigador
     * @param mail e-mail do investigador
     * @param grupo Acrónimo do grupo de investiação a que pertence
     */
    public Investigador(String nome, String mail, String grupo) {
        this.nome = nome;
        this.mail = mail;
        this.grupo = grupo;
    }

    /**
     * Obtém o formato com que o nome do autor parece na publicação
     *
     * @return nome do autor formatado
     */
    protected abstract String obterNomeDePublicacao();

    @Override
    public abstract String toString();

//Getters e Setters
    /**
     * Obtém o nome do investigador
     *
     * @return string com o nome
     */
    public String getNome() {
        return nome;
    }

    /**
     * Define o nome do investigador
     *
     * @param nome string com o nome
     */
    public void setNome(String nome) {
        this.nome = nome;
    }

    /**
     * Obtém o email do investigador
     *
     * @return sting com o email
     */
    public String getMail() {
        return mail;
    }

    /**
     * Define o email do investigador
     *
     * @param mail string com o email
     */
    public void setMail(String mail) {
        this.mail = mail;
    }

    /**
     * Obtém o grupo a qual o investigador pertence
     *
     * @return string com o acrónimo do grupo
     */
    public String getGrupo() {
        return grupo;
    }

    /**
     * Define o grupo a qual o investigador pertence
     *
     * @param grupo string com o acrónimo do grupo
     */
    public void setGrupo(String grupo) {
        this.grupo = grupo;
    }

    /**
     * Obtém o tipo do investigador
     *
     * @return string com o tipo de investigador
     */
    public String getTipo() {
        return tipo;
    }

    /**
     * Define o tipo do investigador
     *
     * @param tipo string como tipo de investigador
     */
    public void setTipo(String tipo) {
        this.tipo = tipo;
    }

}

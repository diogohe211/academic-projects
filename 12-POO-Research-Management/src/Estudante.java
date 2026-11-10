package projetopoo;

import java.io.Serializable;

/**
 * Subclasse de Investigador que representa um Estudante que trabalha num grupo
 * de investigação da CISUC
 *
 * @author Diogo Henriques
 */
public class Estudante extends Investigador implements Serializable {

    private String titulomestrado;
    private String orientador;
    private Data datadeconclusao;

    /**
     * Construtor da subclasse de Investigador, Estudante
     *
     * @param titulomestrado string com o titulo da tese de mestrado do
     * estudante
     * @param orientador string com o nome do professor orientador do estudante
     * @param datadeconclusao objeto Data com a data de conclusão do
     * doutoramento
     * @param nome Herdado de Investigador
     * @param mail Herdado de Investigador
     * @param grupo Herdado de Investigador
     */
    public Estudante(String titulomestrado, String orientador, Data datadeconclusao, String nome, String mail, String grupo) {
        super(nome, mail, grupo);
        this.titulomestrado = titulomestrado;
        this.orientador = orientador;
        this.datadeconclusao = datadeconclusao;
    }

    @Override
    public String obterNomeDePublicacao() {
        String nomepub;
        nome = getNome();
        String[] spltnome = nome.split(" ");
        spltnome[0] = spltnome[0].charAt(0) + ".";
        nomepub = spltnome[0] + " " + spltnome[spltnome.length - 1];
        return nomepub;
    }

    @Override
    public String toString() {
        return "Tipo: " + tipo + "; Nome: " + nome + "; Título da tese de doutoramento: " + titulomestrado + "; Professor orientador: " + orientador + "; Data de conclusão de doutoramento: " + datadeconclusao + "; Nome apresentado nas publicações: " + obterNomeDePublicacao()
                + "; E-mail: " + mail + "; Grupo a que pertence: " + grupo + ";";
    }

    /**
     * Obtém o titulo da tese de mestrado do estudante
     *
     * @return string com o titulo da tese
     */
    public String getTitulomestrado() {
        return titulomestrado;
    }

    /**
     * Define o titulo da tese de mestrado do estudante
     *
     * @param titulomestrado string com o titulo da teste
     */
    public void setTitulomestrado(String titulomestrado) {
        this.titulomestrado = titulomestrado;
    }

    /**
     * Obtém o nome do professor orientador do estudante
     *
     * @return string com o nome do orientador
     */
    public String getOrientador() {
        return orientador;
    }

    /**
     * Define o professor orientador do Estudante
     *
     * @param orientador string com o nome do orientador
     */
    public void setOrientador(String orientador) {
        this.orientador = orientador;
    }

    /**
     * Obtém a data de conclusão do doutoramento do estudante
     *
     * @return objeto Data da data de conclusão do doutoramento
     */
    public Data getDatadeconclusao() {
        return datadeconclusao;
    }

    /**
     * Define a data de conclusão do doutoramento do estudante
     *
     * @param datadeconclusao objeto Data da data de conclusão
     */
    public void setDatadeconclusao(Data datadeconclusao) {
        this.datadeconclusao = datadeconclusao;
    }

}

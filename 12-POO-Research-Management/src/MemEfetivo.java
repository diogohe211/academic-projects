package projetopoo;

import java.io.Serializable;

/**
 * Subclasse de Investigador que representa um Membro Efetivo de um determinado
 * grupo da CISUC
 *
 * @author Diogo Henriques
 */
public class MemEfetivo extends Investigador implements Serializable {

    private String numgabinete, numtelefone;

    /**
     * Contrutor da subclasse de Investigador, Membro Efetivo
     *
     * @param numgabinete número do gabinete do membro efetivo
     * @param numtelefone número de telefone do membro efetivo
     * @param nome herdado da superclasse Investigador
     * @param mail herdado da superclasse Investigador
     * @param grupo herdado da superclasse Investigador
     */
    public MemEfetivo(String numgabinete, String numtelefone, String nome, String mail, String grupo) {
        super(nome, mail, grupo);
        this.numgabinete = numgabinete;
        this.numtelefone = numtelefone;
    }

    @Override
    public String toString() {
        return "Tipo: " + tipo + "; Nome: " + nome + "; E-mail: " + mail + "; Número do gabinete: " + numgabinete + "; Número de telefone: " + numtelefone + "; O grupo a que pertence: " + grupo;
    }

//Getters e Setters
    /**
     * Obtém o número do gabinete do membro efetivo
     * @return string com o número do gabinete
     */
    public String getNumgabinete() {
        return numgabinete;
    }
/**
 * Define o número do gabinete do membro efetivo
 * @param numgabinete string com o número do gabinete
 */
    public void setNumgabinete(String numgabinete) {
        this.numgabinete = numgabinete;
    }
/**
 * Obtém o número de telefone do membro efetivo
 * @return string com o numero de telefone
 */
    public String getNumtelefone() {
        return numtelefone;
    }
/**
 * Define o número de telefone do membro efetivo
 * @param numtelefone string com o número de telefone 
 */
    public void setNumtelefone(String numtelefone) {
        this.numtelefone = numtelefone;
    }

    @Override
    public String getNome() {
        return nome;
    }

    @Override
    public void setNome(String nome) {
        this.nome = nome;
    }

    @Override
    public String getMail() {
        return mail;
    }

    @Override
    public void setMail(String mail) {
        this.mail = mail;
    }

    @Override
    public String getGrupo() {
        return grupo;
    }

    @Override
    public void setGrupo(String grupo) {
        this.grupo = grupo;
    }

    @Override
    public String obterNomeDePublicacao() {
        String nomepub;
        nome = getNome();
        String[] spltnome = nome.split(" ");

        nomepub = "Professor " + spltnome[0] + " " + spltnome[spltnome.length - 1];
        return nomepub;
    }

}

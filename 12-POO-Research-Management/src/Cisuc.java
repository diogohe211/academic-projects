/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package projetopoo;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.Serializable;

import java.time.Year;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Scanner;

/**
 * Classe que caracteriza e engloba todas as funções de CISUC
 * @author Diogo Henriques
 */
public class Cisuc implements Serializable {

    /**
     * ArrayList de grupos de investigação do cisuc
     */
    protected ArrayList<GrupoDeInvestigacao> grupos;
    /**
     * ArrayList de publcações do cisuc
     */
    protected ArrayList<Publicacao> publicacoes;
    /**
     * ArrayList de investigadores do cisuc
     */
    protected ArrayList<Investigador> investigadores;

    /**
     * Contrutor da classe cisuc
     */
    public Cisuc() {
        this.grupos = new ArrayList<>();
        this.publicacoes = new ArrayList<>();
        this.investigadores = new ArrayList<>();
    }

    /**
     * Apresenta as funções do programa ao utilizador. Até o utilizador
     * introduzir 6 na consola este menu reaparece
     */
    public void apresentaMenu() {
        System.out.println("========================================================================================================================");
        System.out.println("=====MENU=====");
        System.out.println("1-Apresentar Indicadores Gerais do CISUC;\n2-Listar as publicações de um grupo de investigação, dos últimos 5 anos, organizadas por ano, por tipo de publicação e por fator de impacto;\n"
                + "3-Listar os membros de um grupo de investigação agrupados por categoria;\n4-Listar as publicações de um investigador agrupadas por ano, tipo de publicação e fator de impacto;\n5-Listar todos os grupos de investigação\n6-Fechar aplicação");

        Scanner sc = new Scanner(System.in);
        int choice = sc.nextInt();

        switch (choice) {
            case 1:
                listaIndicadoresGerais();
                apresentaMenu();
                break;
            case 2:
                System.out.println("Escreva o acrónimo do grupo cujas publicações quer listar: ");
                Scanner sg = new Scanner(System.in);
                String ag = sg.nextLine();
                String a = ag.toUpperCase();

                listaPubsDeGrupo(a);
                apresentaMenu();
                break;
            case 3:
                System.out.println("Escreva o acrónimo do grupo cujos membros quer listar: ");
                Scanner sgm = new Scanner(System.in);
                String agm = sgm.nextLine();
                String am = agm.toUpperCase();
                membrosAgrupados(am);
                apresentaMenu();
                break;
            case 4:
                System.out.println("Escreva o nome do membro cujas publicações quer listar: ");
                Scanner sm = new Scanner(System.in);
                String m = sm.nextLine();
                listaPubsDeInvestigador(m);
                apresentaMenu();
                break;
            case 5:

                listaGrupos();
                apresentaMenu();
                break;
            case 6:
                System.out.println("FIM");
                break;

        }

    }

    /**
     * Lista todos os grupos apresentando para cada : Total de membros;Número de
     * membros de cada categoria; Total de publicações dos ultimos 5 anos;
     * Número de publicações dos ultimos 5 anos agrupados por ano tipo e fator
     * de impacto
     */
    private void listaGrupos() {
        for (GrupoDeInvestigacao gi : getGrupos()) {

            System.out.println("\n===============================================================>\nNome do grupo: " + gi.getNome());
            ArrayList<String> membros = gi.getInvestigadores();
            ArrayList<Investigador> membrosarray = new ArrayList<>();
            for (String m : membros) {
                for (Investigador i : getInvestigadores()) {
                    if (m.equals(i.getNome())) {
                        membrosarray.add(i);
                    }
                }
            }
            int total = 0;
            for (Investigador m : membrosarray) {
                m.toString();
                total += 1;
            }

            System.out.println("Total de Membros:" + total);
            int numefet = 0, numest = 0;
            for (Investigador m : membrosarray) {
                String tipo = m.getTipo();
                if ("Membro Efetivo".equals(tipo)) {
                    numefet += 1;
                } else if ("Estudante".equals(tipo)) {
                    numest += 1;
                }

            }
            System.out.println("Número de Membros Efetivos: " + numefet + "\nNúmero de membros estudantes: " + numest);
            listaPubsDeGrupo(gi.getAcr());
            int y = Year.now().getValue();
            ArrayList<Publicacao> publicacoes = gi.getPublicacoes();
            ArrayList<Publicacao> tocount = new ArrayList<>();
            for (Publicacao p : publicacoes) {
                int aux = 0;
                for (String si : gi.getInvestigadores()) {
                    if (p.getInvestigadores().contains(si) && (y - p.getAno()) <= 5) {
                        aux += 1;
                    }
                }
                if (aux > 0) {
                    tocount.add(p);
                }

            }
            int a = 0, b = 0, c = 0;
            for (Publicacao p : tocount) {

                if (p.fatorImpacto(p.getDim()) == 'A') {
                    a += 1;
                }
                if (p.fatorImpacto(p.getDim()) == 'B') {
                    b += 1;
                }
                if (p.fatorImpacto(p.getDim()) == 'C') {
                    c += 1;
                }
            }
            System.out.println("");
            System.out.println("Número de Publicações, dos ultimos 5 anos, com fator de impacto A: " + a + "\nNúmero de Publicações, dos ultimos 5 anos, com fator de impacto B: " + b + "\nNúmero de Publicações, dos ultimos 5 anos, com fator de impacto C: " + c);

        }
    }

    /**
     * Lista os Indicadores Gerais do Cisuc: Total de membros; Número de membros
     * de cada categoria; Total de publicações dos últimos 5 anos Número de
     * publicações de cada tipo;
     */
    private void listaIndicadoresGerais() {
        System.out.println("===Listagem dos Indicadores Gerais do CISUC===");
        System.out.println("\n==Membros:==");
        listaMembros();
        numMemPorCat();
        System.out.println("\n==Publicações:==");
        listaCincoAnos();
        numPubPorCat();
    }

    /**
     * Listar as publicações de um investigador agrupadas por ano, tipo de
     * publicação e fator de impacto
     *
     * @param investigador Nome do Investigador cujas publicações serão listadas
     */
    public void listaPubsDeInvestigador(String investigador) {
        int aux = 0; //variável auxiliar para o caso do investigador introduzido não constar na lista de investigadores do cisuc

        for (Investigador i : getInvestigadores()) {
            String ni = i.getNome();
            if (ni.equals(investigador)) {

                ArrayList<Publicacao> publicacoes = getPublicacoes();
                ArrayList<Publicacao> toprint = new ArrayList<>();
                for (Publicacao p : publicacoes) {
                    ArrayList<String> autores = p.getInvestigadores();
                    if (autores.contains(ni)) {
                        toprint.add(p);
                    }

                }

                organizaListaPubs(toprint);

                aux += 1;//Se o grupo introduzir foi válido aumenta a variável e já não dá a mensagem de erro
            }

        }

        if (aux == 0) {

            System.out.println("ERRO: Investigador não existente");

        }

    }

    /**
     * Lista os membros de um grupo por categoria
     *
     * @param grupo acrónimo do grupo cujo os membros vão ser listados
     */
    private void membrosAgrupados(String grupo) {
        int aux = 0;
        ArrayList<Investigador> me = new ArrayList<>();
        ArrayList<Investigador> e = new ArrayList<>();
        for (GrupoDeInvestigacao g : getGrupos()) {
            String acronimo = g.getAcr();
            if (acronimo.equals(grupo)) {
                ArrayList<String> membros = g.getInvestigadores();
                for (Investigador gc : getInvestigadores()) {
                    if (membros.contains(gc.getNome())) {
                        String tipo = gc.getTipo();
                        if ("Membro Efetivo".equals(tipo)) {
                            me.add(gc);
                        } else if ("Estudante".equals(tipo)) {
                            e.add(gc);
                        }
                    }
                }
                aux += 1;
            }
        }
        if (aux == 0) {
            System.out.println("ERRO: Grupo não existente");

        } else {
            System.out.println("\nMembros efetivos:");
            for (Investigador i : me) {
                System.out.println(i.toString());
            }
            System.out.println("\nMembros estudantes:");
            for (Investigador i : e) {
                System.out.println(i.toString());
            }
        }
    }

    /**
     * Lista as publicações de um grupo dos ultimos 5 anos por tipo, ano ou
     * fator de impacto
     *
     * @param grupo acrónimo do grupo cujo as publicações vão ser listadas
     */
    private void listaPubsDeGrupo(String grupo) {
        int aux = 0; //variável auxiliar para o caso do grupo introduzido não constar na lista de grupos do cisuc
        int y = Year.now().getValue();
        for (GrupoDeInvestigacao g : getGrupos()) {
            String acronimo = g.getAcr();
            if (acronimo.equals(grupo)) {
                ArrayList<Publicacao> publicacoes = g.getPublicacoes();
                ArrayList<Publicacao> toprint = new ArrayList<>();
                for (Publicacao p : publicacoes) {
                    int anopub = p.getAno();
                    int dif = y - anopub;
                    if (dif <= 5) {
                        toprint.add(p);
                    }
                }

                organizaListaPubs(toprint);
                aux += 1;//Se o grupo introduzir foi válido aumenta a variável e já não dá a mensagem de erro
            }

        }

        if (aux == 0) {
            System.out.println("ERRO: Grupo não existente");

        }

    }

    /**
     * Lista publicações organizadas por ano, tipo e fator de impacto
     *
     * @param toprint ArrayList de publicações para Listar
     */
    private void organizaListaPubs(ArrayList<Publicacao> toprint) {
        System.out.println("\n=================================================================================\nPublicações do grupo dos ultimos 5 anos organizado por ano:");
        ArrayList<Integer> auxanos = new ArrayList<>();//arraylist auxiliar para guardar os anos das publicações
        ArrayList<Integer> auxanosnodupes = new ArrayList<>();//porque o arraylist auxanos contem duplicados este vai conter os anos nã duplicados
        //guardar os anos de todas as publicações
        for (Publicacao prt : toprint) {
            auxanos.add(prt.getAno());
        }
        //por para o segundo array auxiliar os anos não duplicados
        for (int i : auxanos) {
            if (!auxanosnodupes.contains(i)) {
                auxanosnodupes.add(i);
            }
        }
        Collections.sort(auxanosnodupes);
        for (int ano : auxanosnodupes) {
            System.out.println("=>ANO " + ano + ":");
            for (Publicacao p : toprint) {
                if (ano == p.getAno()) {
                    System.out.println(p.toString());
                    printAutoresKeywords(p);
                }
            }
        }

        System.out.println("\n==================================================================================\nPublicações do grupo dos ultimos 5 anos organizado por tipo:");
        System.out.println("\n=>Artigos de Revista:");
        for (Publicacao p : toprint) {
            String tipo = p.getTipo();

            if ("Artigo de Revista".equals(tipo)) {
                System.out.println(p.toString());
                printAutoresKeywords(p);
            }

        }
        System.out.println("\n=>Capitulos de Livro: ");
        for (Publicacao p : toprint) {
            String tipo = p.getTipo();

            if ("Capitulo de Livro".equals(tipo)) {
                System.out.println(p.toString());
                printAutoresKeywords(p);
            }
        }
        System.out.println("\n=>Livros de Conferência: ");
        for (Publicacao p : toprint) {
            String tipo = p.getTipo();

            if ("Livro de Conferência".equals(tipo)) {
                System.out.println(p.toString());
                printAutoresKeywords(p);
            }
        }
        System.out.println("\n=>Livros: ");
        for (Publicacao p : toprint) {
            String tipo = p.getTipo();

            if ("Livro".equals(tipo)) {
                System.out.println(p.toString());
                printAutoresKeywords(p);
            }
        }
        System.out.println("\n=>Artigos de Conferência: ");
        for (Publicacao p : toprint) {
            String tipo = p.getTipo();

            if ("Artigo de Conferência".equals(tipo)) {
                System.out.println(p.toString());
                printAutoresKeywords(p);
            }
        }

        System.out.println("\n==================================================================================\nPublicações do grupo dos ultimos 5 anos organizado por fator de impacto:");
        System.out.println("=>Fator de impacto A: ");
        for (Publicacao p : toprint) {
            char fator = p.fatorImpacto(p.getDim());
            if (fator == 'A') {
                System.out.println(p.toString());
                printAutoresKeywords(p);
            }
        }
        System.out.println("=>Fator de impacto B: ");
        for (Publicacao p : toprint) {
            char fator = p.fatorImpacto(p.getDim());
            if (fator == 'B') {
                System.out.println(p.toString());
                printAutoresKeywords(p);
            }
        }
        System.out.println("=>Fator de impacto C: ");
        for (Publicacao p : toprint) {
            char fator = p.fatorImpacto(p.getDim());
            if (fator == 'C') {
                System.out.println(p.toString());
                printAutoresKeywords(p);
            }
        }
    }

    /**
     * Mostra o número de publicações de cada categoria
     */
    private void numPubPorCat() {
        ArrayList<Publicacao> pubs = getPublicacoes();
        int rev = 0, livr = 0, lconf = 0, aconf = 0, capl = 0;
        for (Publicacao p : pubs) {
            String tipo = p.getTipo();
            if ("Artigo de Revista".equals(tipo)) {
                rev += 1;
            } else if ("Livro de Conferência".equals(tipo)) {
                lconf += 1;
            } else if ("Capitulo de Livro".equals(tipo)) {
                capl += 1;
            } else if ("Livro".equals(tipo)) {
                livr += 1;
            } else if ("Artigo de Conferência".equals(tipo)) {
                aconf += 1;
            }
        }
        System.out.println("===============================================================================================");
        System.out.println("\nNúmero de Livros: " + livr + "\nNúmero de livros de Conferência: " + lconf + "\nNúmero de Capitulos de Livro: " + capl + "\nNúmero de Artigos de Revista: " + rev + "\nNúmero de Artigos de Conferência: " + aconf);
    }

    /**
     * Mostra o número de membros de cada categoria
     */
    private void numMemPorCat() {
        ArrayList<Investigador> membros = getInvestigadores();
        int numefet = 0, numest = 0;

        for (Investigador m : membros) {
            String tipo = m.getTipo();
            if ("Membro Efetivo".equals(tipo)) {
                numefet += 1;
            } else if ("Estudante".equals(tipo)) {
                numest += 1;
            }

        }
        System.out.println("\nNúmero de Membros Efetivos: " + numefet + "\nNúmero de membros estudantes: " + numest);
    }

    /**
     * Lista as publicações dos últimos 5 anos
     */
    private void listaCincoAnos() {
        int y = Year.now().getValue();
        ArrayList<Publicacao> toprint = new ArrayList<>();
        for (Publicacao p : getPublicacoes()) {
            int anopub = p.getAno();
            int dif = y - anopub;
            if (dif <= 5) {
                toprint.add(p);
            }

        }
        System.out.println("");
        for (Publicacao p1 : toprint) {
            System.out.println(p1.toString());

            printAutoresKeywords(p1);
        }
    }

    /**
     * Faz print de todos os membros pertencentes a CISUC
     */
    private void listaMembros() {
        ArrayList<Investigador> membros = getInvestigadores();
        System.out.println("");
        for (Investigador m : membros) {
            System.out.println(m.toString());
        }
    }

    /**
     * Criado para compactar as 3 funções de separação em uma
     *
     * @param fichinvestigadores ficheiro .txt com o conteúdo dos investigadores
     * @param fichpublicacoes ficheiro .txt com o contéudo das publicações
     * @param fichgrupos ficheiro .txt com o conteúdo dos grupos
     */
    public void loadDataFromTxt(String fichinvestigadores, String fichpublicacoes, String fichgrupos) {
        separaGrupos(fichgrupos);
        separaPublicacoes(fichpublicacoes);
        separaInvestigadores(fichinvestigadores);
        putPublicacoesInGrupos();
    }

    /**
     * Lê um ficheiro de texto e separa as informações para criar variáveis do
     * tipo GrupoDeInvestigacao depois faz setGrupos para definir os grupo do
     * objeto CISUC
     *
     * @param nometxt ficheiro do texto com as informações dos grupos
     * @return
     */
    private void separaGrupos(String nometxt) {
        ArrayList<String> linhas = read(nometxt);
        ArrayList<GrupoDeInvestigacao> grupos = new ArrayList<>();
        for (String linha : linhas) {
            String[] lsplit = linha.split("\\|");
            String nome = lsplit[0];
            String acron = lsplit[1];
            String responsavel = lsplit[2];
            String[] membros = lsplit[3].split(",");
            ArrayList<String> almembros = new ArrayList<>();
            for (int i = 0; i < membros.length; i++) {
                almembros.add(membros[i]);
            }
            GrupoDeInvestigacao g = new GrupoDeInvestigacao(nome, acron, responsavel);
            g.setInvestigadores(almembros);
            grupos.add(g);

        }

        setGrupos(grupos);
    }

    /**
     * Lê um txt e cria um ArrayList com as publicações todas
     *
     * @param nometxt
     */
    private void separaPublicacoes(String nometxt) {
        ArrayList<String> linhas = read(nometxt);
        ArrayList<Publicacao> pubs = new ArrayList<>();
        for (String linha : linhas) {
            String[] lsplit = linha.split("\\|");
            if ("Livro de Conferência".equals(lsplit[0])) {
                String nomeconf = lsplit[1];
                int numartigos = Integer.parseInt(lsplit[2]);
                String editora = lsplit[3];
                String nomelivro = lsplit[4];
                String resumo = lsplit[5];
                String tipo = lsplit[0];
                int dim = Integer.parseInt(lsplit[6]);
                int ano = Integer.parseInt(lsplit[7]);
                String la = lsplit[8];
                ArrayList<String> autores = new ArrayList<>();
                String[] arra = la.split(",");
                for (int i = 0; i < arra.length; i++) {
                    autores.add(arra[i]);

                }

                String lk = lsplit[9];
                ArrayList<String> keywords = new ArrayList<>();
                String[] arrk = lk.split(",");
                for (int i = 0; i < arrk.length; i++) {
                    keywords.add(arrk[i]);

                }
                Publicacao publ = new LivroConferencia(nomeconf, numartigos, editora, nomelivro, tipo, resumo, dim, ano);
                publ.setInvestigadores(autores);
                publ.setKeywords(keywords);
                pubs.add(publ);
            }
            if ("Livro".equals(lsplit[0])) {
                String editora = lsplit[1];

                String nomelivro = lsplit[2];
                String resumo = lsplit[3];
                String tipo = lsplit[0];
                int dim = Integer.parseInt(lsplit[4]);
                int ano = Integer.parseInt(lsplit[5]);
                String la = lsplit[6];
                ArrayList<String> autores = new ArrayList<>();
                String[] arra = la.split(",");
                for (int i = 0; i < arra.length; i++) {
                    autores.add(arra[i]);

                }

                String lk = lsplit[7];
                ArrayList<String> keywords = new ArrayList<>();
                String[] arrk = lk.split(",");
                for (int i = 0; i < arrk.length; i++) {
                    keywords.add(arrk[i]);

                }
                Publicacao publ = new Livro(editora, nomelivro, tipo, resumo, dim, ano);
                publ.setInvestigadores(autores);
                publ.setKeywords(keywords);
                pubs.add(publ);

            }
            if ("Capitulo de Livro".equals(lsplit[0])) {
                String capitulo = lsplit[1];
                int paginicial = Integer.parseInt(lsplit[2]);
                int pagfinal = Integer.parseInt(lsplit[3]);
                String editora = lsplit[4];
                String nomelivro = lsplit[5];
                String resumo = lsplit[6];
                String tipo = lsplit[0];
                int dim = Integer.parseInt(lsplit[7]);
                int ano = Integer.parseInt(lsplit[8]);
                String la = lsplit[9];
                ArrayList<String> autores = new ArrayList<>();
                String[] arra = la.split(",");
                for (int i = 0; i < arra.length; i++) {
                    autores.add(arra[i]);

                }

                String lk = lsplit[10];
                ArrayList<String> keywords = new ArrayList<>();
                String[] arrk = lk.split(",");
                for (int i = 0; i < arrk.length; i++) {
                    keywords.add(arrk[i]);

                }
                Publicacao publ = new Capitulo(capitulo, paginicial, pagfinal, editora, nomelivro, tipo, resumo, dim, ano);
                publ.setInvestigadores(autores);
                publ.setKeywords(keywords);
                pubs.add(publ);

            }
            if ("Artigo de Revista".equals(lsplit[0])) {
                String nomerevista = lsplit[1];
                String[] arrdata = lsplit[2].split("\\/");
                Data data = new Data(Integer.parseInt(arrdata[0]), Integer.parseInt(arrdata[1]), Integer.parseInt(arrdata[2]));
                int ano = data.getAno();
                int numrevista = Integer.parseInt(lsplit[3]);
                String nomeartigo = lsplit[4];
                String resumo = lsplit[5];

                String tipo = lsplit[0];
                int dim = Integer.parseInt(lsplit[6]);

                String la = lsplit[7];
                ArrayList<String> autores = new ArrayList<>();
                String[] arra = la.split(",");
                for (int i = 0; i < arra.length; i++) {
                    autores.add(arra[i]);

                }

                String lk = lsplit[8];
                ArrayList<String> keywords = new ArrayList<>();
                String[] arrk = lk.split(",");
                for (int i = 0; i < arrk.length; i++) {
                    keywords.add(arrk[i]);

                }
                Publicacao publ = new Revista(nomerevista, data, numrevista, nomeartigo, tipo, resumo, dim, ano);
                publ.setInvestigadores(autores);
                publ.setKeywords(keywords);
                pubs.add(publ);

            }
            if ("Artigo de Conferência".equals(lsplit[0])) {
                String tipo = lsplit[0];
                String[] arrdata = lsplit[1].split("\\/");
                Data data = new Data(Integer.parseInt(arrdata[0]), Integer.parseInt(arrdata[1]), Integer.parseInt(arrdata[2]));
                int ano = data.getAno();
                String nomeconf = lsplit[2];
                String local = lsplit[3];
                String nomelivroconf = lsplit[4];
                String resumo = lsplit[5];

                int dim = Integer.parseInt(lsplit[6]);

                String la = lsplit[7];
                ArrayList<String> autores = new ArrayList<>();
                String[] arra = la.split(",");
                for (int i = 0; i < arra.length; i++) {
                    autores.add(arra[i]);

                }

                String lk = lsplit[8];
                ArrayList<String> keywords = new ArrayList<>();
                String[] arrk = lk.split(",");
                for (int i = 0; i < arrk.length; i++) {
                    keywords.add(arrk[i]);

                }
                Publicacao publ = new Conferencia(data, nomeconf, local, nomelivroconf, tipo, resumo, dim, ano);
                publ.setInvestigadores(autores);
                publ.setKeywords(keywords);
                pubs.add(publ);

            }

        }

        setPublicacoes(pubs);
    }

    /**
     * Lê um ficheiro de texto e cria uma ArrayList com as linhas do ficheiro
     *
     * @param nometxt
     * @return
     */
    private ArrayList<String> read(String nometxt) {
        ArrayList<String> linhas = new ArrayList<>();
        try {
            FileReader fich = new FileReader(nometxt);
            BufferedReader lerFich = new BufferedReader(fich);
            String linha = "";
            try {
                linha = lerFich.readLine();
                while (linha != null) {
                    linhas.add(linha);
                    linha = lerFich.readLine();
                }
                fich.close();

                return linhas;
            } catch (IOException ex) {
                System.out.println("Erro: Não foi possível ler o ficheiro!");
                return null;
            }
        } catch (FileNotFoundException ex) {
            System.out.println("Erro: Ficheiro não encontrado!");
            return null;
        }
    }

    /**
     * Lê o txt com os dados dos investigadores e cria um array com variáveis do
     * tipo Investigador e depois faz um setInvestigadores para o CISUC
     *
     * @param nometxt Ficheiro de texto com os dados dos investigadores
     */
    private void separaInvestigadores(String nometxt) {
        ArrayList<String> linhas = read(nometxt);
        ArrayList<Investigador> investigadores = new ArrayList<>();
        for (String linha : linhas) {
            String[] lsplit = linha.split("\\|");
            if ("ME".equals(lsplit[0])) {
                //Formato do Membro efetivo no txt: ME|Numgabinete|NumTelefone|Nome|Mail|Grupo
                String numgabi = lsplit[1];
                String numtele = lsplit[2];
                String nome = lsplit[3];
                String mail = lsplit[4];
                String grupo = lsplit[5];

                Investigador i = new MemEfetivo(numgabi, numtele, nome, mail, grupo);
                String tipo = "Membro Efetivo";
                i.setTipo(tipo);

                investigadores.add(i);
            } else if ("E".equals(lsplit[0])) {
                //Formato do estudante no txt: E|Nome do mentor|Data de Conclusão|Nome|Mail|Grupo
                String mestrado = lsplit[1];
                String mentor = lsplit[2];
                String data = lsplit[3];
                String[] datasplit = data.split("/");
                Data dataconclusao = new Data(Integer.parseInt(datasplit[0]), Integer.parseInt(datasplit[1]), Integer.parseInt(datasplit[2]));
                String nome = lsplit[4];
                String mail = lsplit[5];
                String grupo = lsplit[6];
                ;
                Investigador i = new Estudante(mestrado, mentor, dataconclusao, nome, mail, grupo);

                String tipo = "Estudante";
                i.setTipo(tipo);
                investigadores.add(i);
            }

        }

        setInvestigadores(investigadores);
    }

    /**
     * Coloca as publicações nos grupos dos autores
     */
    private void putPublicacoesInGrupos() {
        grupos = getGrupos();

        publicacoes = getPublicacoes();
        for (GrupoDeInvestigacao g : grupos) {
            ArrayList<Publicacao> publicacoesgrupo = g.getPublicacoes();
            ArrayList<String> membros = g.getInvestigadores();
            for (Publicacao p : publicacoes) {
                ArrayList<String> autorespub = p.getInvestigadores();
                int aux = 0;
                for (String a : autorespub) {

                    if (membros.contains(a)) {
                        aux = aux + 1;

                    }

                }
                if (aux > 0) {
                    publicacoesgrupo.add(p);
                }

            }
            g.setPublicacoes(publicacoesgrupo);

        }
        setGrupos(grupos);
    }

    /**
     * Faz print ao nome de publicação de cada autor de uma publicação e às
     * keywords da publicação
     *
     * @param p Publicação cujos autores serão listados
     */
    private void printAutoresKeywords(Publicacao p) {
        ArrayList<String> keywords = p.getKeywords();
        System.out.println("Keywords:" + keywords);
        ArrayList<String> autores = p.getInvestigadores();
        ArrayList<Investigador> arrautores = new ArrayList<>();
        for (String a : autores) {
            for (Investigador i : getInvestigadores()) {
                if (i.getNome().equals(a)) {
                    arrautores.add(i);
                }
            }
        }
        System.out.print("Autores: ");
        for (Investigador i : arrautores) {
            System.out.print(i.obterNomeDePublicacao() + "; ");
        }
        System.out.println("\n");
    }

//Getters e Setters
    /**
     * Obtem array de investigadores
     *
     * @return array
     */
    private ArrayList<Investigador> getInvestigadores() {
        return investigadores;
    }

    /**
     * Define array de investigadores
     *
     * @param investigadores array de investigadores definidor
     */
    private void setInvestigadores(ArrayList<Investigador> investigadores) {
        this.investigadores = investigadores;
    }

    /**
     * Obtém array de grupos de investigação
     *
     * @return array
     */
    private ArrayList<GrupoDeInvestigacao> getGrupos() {
        return grupos;
    }

    /**
     * Define um array de grupos
     *
     * @param grupos array de grupos definidor
     */
    private void setGrupos(ArrayList<GrupoDeInvestigacao> grupos) {
        this.grupos = grupos;
    }

    /**
     * Obtém array de publicações
     *
     * @return array
     */
    private ArrayList<Publicacao> getPublicacoes() {
        return publicacoes;
    }

    /**
     * Define o array de publicações
     *
     * @param publicacoes array de publicação definidor
     */
    private void setPublicacoes(ArrayList<Publicacao> publicacoes) {
        this.publicacoes = publicacoes;
    }
}

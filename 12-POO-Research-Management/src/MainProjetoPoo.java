package projetopoo;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;

/**
 *
 * @author Diogo Henriques
 */
public class MainProjetoPoo {

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        System.out.println("START");
        File f = new File("Cisuc.obj");
        try {
            FileInputStream fis = new FileInputStream(f);
            ObjectInputStream ois = new ObjectInputStream(fis);
            Cisuc cisuc = (Cisuc) ois.readObject();
            
            System.out.println("Objeto Lido");
            cisuc.apresentaMenu();
            ois.close();

        } catch (FileNotFoundException ex) {
            System.out.println("Não existe ficheiro de objetos.\nDados serão recebidos pelos ficheiros de texto");
            Cisuc cisuc = new Cisuc();
            cisuc.loadDataFromTxt("Investigadores.txt", "Publicações.txt", "Grupos de Investigação.txt");

            try {
                FileOutputStream fich = new FileOutputStream("Cisuc.obj");
                ObjectOutputStream obj = new ObjectOutputStream(fich);
                obj.writeObject(cisuc);
                obj.flush();
                System.out.println("Gravação bem sucedida");
            } catch (FileNotFoundException ey) {
                System.out.println("Erro a criar ficheiro .obj.");
            } catch (IOException ey) {
                System.out.println("Erro a escrever para o ficheiro.");
            }
            cisuc.apresentaMenu();
        } catch (IOException ex) {
            System.out.println("Erro a ler ficheiro.");
        } catch (ClassNotFoundException ex) {
            System.out.println("Erro a converter objeto.");
        }

    }

}

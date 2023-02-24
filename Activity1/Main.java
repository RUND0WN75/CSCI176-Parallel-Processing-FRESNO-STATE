import java.sql.SQLOutput;
import java.util.ArrayList;

public class Thread {
}

public class sharedObject {

}

public class Main {
    //Global vector (ArrayList) of size 500 million
    public static Integer global_size = 500_000_000;
    public static ArrayList<Integer> arr1 = new ArrayList<Integer>();

    public static void main(String[] args) {
        //Initializing array to i+1
        for (int i = 0; i < global_size; i++) arr1.add(i+1);

        System.out.println("Hello world!");
    }
}
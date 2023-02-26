import java.sql.SQLOutput;
import java.util.ArrayList;

//Thread class using the extends Thread
public class compThread extends Thread {
    static global_sum gs;
    private int thread_id;
    double local_sum;
    int my_first_index;
    int my_last_index;

    //Constructor
    compThread(int k, global_sum gs) {
        this.gs = gs;
        thread_id = k;
        System.out.println("Thread for compThread: ") + thread_id + (" created");
    }

    //Using run() method
    void run() {
        //First calculate my_first_index and my_last_index
        //Next compute local_sum with a serial operation
        local_sum = thread_id * 100;
        System.out.println("Thread ") + thread_id + (", local_sum: ") + local_sum;
    }
}

//Shared object
public class sharedObject {

}

public class Main {
    //Global vector (ArrayList) of size 500 million
    public static Integer global_size = 500_000_000;
    public static ArrayList<Integer> arr1 = new ArrayList<Integer>();

    public static void main(String[] args) {
        //Initializing array to i+1
        for (int i = 0; i < global_size; i++) arr1.add(i+1)

        System.out.println("Hello world!");
    }
}
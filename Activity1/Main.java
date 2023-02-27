import java.sql.SQLOutput;
import java.util.ArrayList;

//Main class, same as file name
public class Main {
    //Global vector (ArrayList) of size 500 million
    static Integer global_size = 500_000_000;
    static ArrayList<Integer> arr1 = new ArrayList<Integer>();

    //FIXME
    static g_sum gs = new g_sum();

    static void main(String[] args) {
        //Initializing array to i+1
        for (int i = 0; i < global_size; i++) arr1.add(i+1);

        //Creating 4 computational threads
        for (int k = 0; k < 4; k++) {
            compThread t = new compThread(k,gs);
            t.start();
        }

        //For general testing purposes
        System.out.println("Hello world!");
    }
}

//Global sum class
class globalSum {
    private static double gsum = 0;

    //Implementing shared-variable (data) communication using 'synchronized'
    public synchronized void adding(double local_sum) {
        gsum += local_sum; //Updating shared variable

        //If at the last thread, display the first global_sum
        System.out.println("global_sum: " + gsum);
    }
}

//Thread class using the extends Thread
class compThread extends Thread {
    static g_sum gs;
    private int thread_id;
    double local_sum;
    int my_first_index;
    int my_last_index;

    //Constructor
    compThread(int k, g_sum gs) {
        this.gs = gs;
        thread_id = k;
        System.out.println("Thread for compThread: " + thread_id + " created");
    }

    //Using run() method
    public void run() {
        //First calculate my_first_index and my_last_index
        //Next compute local_sum with a serial operation
        local_sum = thread_id * 100;
        System.out.println("Thread " + thread_id + ", local_sum: " + local_sum);

        gs.adding(local_sum);
    }
}
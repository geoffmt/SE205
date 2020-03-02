public class BoundedBufferMain {

    public static void main (String[] args) throws InterruptedException{
        BoundedBuffer buffer;

        // Check the arguments of the command line
        if (args.length != 1){
            System.out.println ("PROGRAM FILENAME");
            System.exit(1);
        }
        Utils.init(args[0]);

        // Create a buffer
        if (Utils.sem_impl == 0)
            buffer = new NatBoundedBuffer(Utils.bufferSize);
        else
            buffer = new SemBoundedBuffer(Utils.bufferSize);


        Thread tasks[] = new Thread[(int)(Utils.nProducers+Utils.nConsumers)];
        // Create producers and then consumers
        for (int i = 0; i<(int)Utils.nConsumers; i++){
          Consumer newConsumer = new Consumer(i,buffer);
          newConsumer.start();
          tasks[i] = newConsumer;
        }
        for (int i=(int)Utils.nConsumers; i<(int)(Utils.nProducers+Utils.nConsumers); i++){
          Producer newProducer = new Producer(i, buffer);
          newProducer.start();
          tasks[i] = newProducer;
        }


        //On attend que les producers et les consumers se terminent
        for (int i=0; i<Utils.nProducers+Utils.nConsumers; i++){
          tasks[i].join();
        }
    }
}

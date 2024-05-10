
#include <assert.h>
#include "headers.h"
#define WINDOW_WIDTH 10
#define WIDTH 30
#define HEIGHT 20
#define BORDER_WIDTH 100
#define SPACING 5
#define VERTICAL_BOX_SPACING 90
#define PADDING 0
#define SCHEDULER_KEY 65
#define SCHEDULER_LOCK 2
#define SCHEDULER_INFO_LOCK 1
#define HPF 1
#define STRN 2
#define RR 3

int msgq_id, msgq_id2, send_val, algo, quantum, shmid;
char *entered_text = NULL;
void clearResources(int);
int SchedulerId;

// messge struct for sending the data to the scheduler
struct msgbuff
{
    long mtype;
    int algotype;
    int quantum;
    struct Process process;
    int NumOfProcesses;
};
//=================================================================================================================
//======================================================= GUI =====================================================
//=================================================================================================================

// Function to render text file onto a Cairo surface
void renderTextFile(const char *filename, GdkPixbuf **pixbuf)
{
    // Open the text file
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        g_print("Error: Cannot open file.\n");
        return;
    }

    // Calculate the maximum line length to determine the surface width
    int maxLineLength = 0;
    char line[1024];
    int numOfLines = 2; // extra space at the end
    while (fgets(line, sizeof(line), file) != NULL)
    {
        int lineLength = strlen(line);
        if (line[lineLength - 1] == '\n')
        {
            lineLength--; // Exclude newline character from length
        }
        if (lineLength > maxLineLength)
        {
            maxLineLength = lineLength;
        }
        numOfLines++;
    }

    // Close the file
    fclose(file);

    // Calculate the surface width and height based on the maximum line length and number of lines
    int surfaceWidth = (maxLineLength + 2) * 8; // Assuming average character width of 12 pixels
    int surfaceHeight = 20 * numOfLines;        // Assuming each line has a height of 20 pixels

    // Create a new Cairo surface with the calculated dimensions
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, surfaceWidth, surfaceHeight);
    cairo_t *cr = cairo_create(surface);

    // Set background color to white
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    // Set font size and color
    cairo_set_font_size(cr, 12);
    cairo_set_source_rgb(cr, 0, 0, 0);

    // Open the text file again
    file = fopen(filename, "r");
    if (file == NULL)
    {
        g_print("Error: Cannot open file.\n");
        return;
    }

    // Render each line of the text file onto the Cairo surface
    double y = 20; // Starting y-coordinate
    while (fgets(line, sizeof(line), file) != NULL)
    {
        cairo_move_to(cr, 20, y);
        cairo_show_text(cr, line);
        y += 20; // Increase y-coordinate for the next line
    }

    // Finish rendering
    cairo_destroy(cr);

    // Convert Cairo surface to GdkPixbuf
    *pixbuf = gdk_pixbuf_get_from_surface(surface, 0, 0, surfaceWidth, surfaceHeight);

    // Clean up
    cairo_surface_destroy(surface);
    fclose(file);
}

void savePixbufToFile(GdkPixbuf *pixbuf, const char *filename)
{
    // Save the GdkPixbuf to a file
    gdk_pixbuf_save(pixbuf, filename, "png", NULL, NULL);
}

// Callback function to handle submit button clicks for quantum value
void submit_quantum_clicked(GtkWidget *widget, gpointer data)
{
    // Get the text entered in the text entry field
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(data));
    // Convert the text to an integer and store it in the quantum variable
    quantum = atoi(text);

    // Close the current window
    GtkWidget *window = gtk_widget_get_toplevel(widget);
    gtk_widget_destroy(window);
}

void button_get_text_clicked(GtkWidget *widget, gpointer data)
{
    // Get the text from the text entry field
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(data));
    g_print("Text file name entered: %s\n", text);

    // Store the text in the global variable
    entered_text = g_strdup(text);

    // Close the current window
    GtkWidget *window = gtk_widget_get_toplevel(widget);
    gtk_widget_destroy(window);
}

// Callback function to handle button clicks
void button_clicked(GtkWidget *widget, gpointer data)
{
    // Get the label of the clicked button
    const gchar *button_label = gtk_button_get_label(GTK_BUTTON(widget));
    g_print("%s clicked\n", button_label);

    // Close the current window
    GtkWidget *window = GTK_WIDGET(data);
    gtk_widget_destroy(window);

    // Determine the value of algo based on the clicked button
    if (strcmp(button_label, "HPF") == 0)
    {
        algo = HPF;
    }
    else if (strcmp(button_label, "SRTN") == 0)
    {
        algo = STRN;
    }
    else if (strcmp(button_label, "RR") == 0)
    {
        algo = RR;

        // Create a new window for entering the quantum value
        GtkWidget *new_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(new_window), "Enter Quantum");
        gtk_container_set_border_width(GTK_CONTAINER(new_window), WINDOW_WIDTH);
        g_signal_connect(new_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

        // Create a vertical box container
        GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, SPACING);
        gtk_container_add(GTK_CONTAINER(new_window), box);

        // Create a label for the text entry field
        GtkWidget *label = gtk_label_new("Quantum:");
        gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, PADDING);

        // Create a text entry field
        GtkWidget *text_entry = gtk_entry_new();
        gtk_box_pack_start(GTK_BOX(box), text_entry, TRUE, TRUE, PADDING);

        // Create a button to submit the quantum value
        GtkWidget *button_submit = gtk_button_new_with_label("Submit");
        g_signal_connect(button_submit, "clicked", G_CALLBACK(submit_quantum_clicked), text_entry);
        gtk_box_pack_start(GTK_BOX(box), button_submit, TRUE, TRUE, PADDING);

        // Show all widgets
        gtk_widget_show_all(new_window);

        // Start the GTK main loop
        gtk_main();
    }

    // Create a new window for text entry
    GtkWidget *new_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(new_window), "Text Entry");
    gtk_container_set_border_width(GTK_CONTAINER(new_window), WINDOW_WIDTH);
    g_signal_connect(new_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create a vertical box container
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, SPACING);
    gtk_container_add(GTK_CONTAINER(new_window), box);

    // Create a text entry field
    GtkWidget *text_entry = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(box), text_entry);

    // Create a button to retrieve text
    GtkWidget *button_get_text = gtk_button_new_with_label("Get Text");
    g_signal_connect(button_get_text, "clicked", G_CALLBACK(button_get_text_clicked), text_entry);
    gtk_container_add(GTK_CONTAINER(box), button_get_text);

    // Show all widgets
    gtk_widget_show_all(new_window);

    // Start the GTK main loop
    gtk_main();
}

//======================================================================================================
//=============================================== MAIN =================================================
//======================================================================================================
int main(int argc, char *argv[])
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *button_hpf;
    GtkWidget *button_srtn;
    GtkWidget *button_rr;

    gtk_init(&argc, &argv);

    // Create the main window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Scheduling Algorithms");
    gtk_container_set_border_width(GTK_CONTAINER(window), BORDER_WIDTH);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create a vertical box container
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, VERTICAL_BOX_SPACING);
    gtk_container_add(GTK_CONTAINER(window), box);

    // Create the "HPF" button
    button_hpf = gtk_button_new_with_label("HPF");
    gtk_widget_set_size_request(button_hpf, WIDTH, HEIGHT);
    g_signal_connect(button_hpf, "clicked", G_CALLBACK(button_clicked), window);
    gtk_box_pack_start(GTK_BOX(box), button_hpf, TRUE, TRUE, PADDING);

    // Create the "SRTN" button
    button_srtn = gtk_button_new_with_label("SRTN");
    gtk_widget_set_size_request(button_srtn, WIDTH, HEIGHT);
    g_signal_connect(button_srtn, "clicked", G_CALLBACK(button_clicked), window);
    gtk_box_pack_start(GTK_BOX(box), button_srtn, TRUE, TRUE, PADDING);

    // Create the "RR" button
    button_rr = gtk_button_new_with_label("RR");
    gtk_widget_set_size_request(button_rr, WIDTH, HEIGHT);
    g_signal_connect(button_rr, "clicked", G_CALLBACK(button_clicked), window);
    gtk_box_pack_start(GTK_BOX(box), button_rr, TRUE, TRUE, PADDING);

    // Show all widgets
    gtk_widget_show_all(window);

    // Start the GTK main loop
    gtk_main();

    if (entered_text != NULL)
    {

        printf("Final text file name entered is: %s and Algorithm chosen is: %d\n", entered_text, algo);
    }
    else
    {
        g_print("No text entered.\n");
    }

    if (algo == RR)
    {
        printf("RR quantum entered is: %d\n", quantum);
    }

    // initialize the processes queue
    struct Queue *ProcessQueue = createQueue();

    signal(SIGINT, clearResources);

    //============= READ FILE ===================//
    strcat(entered_text, ".txt");
    FILE *inputfile = fopen(entered_text, "r");
    if (inputfile == NULL)
    {
        perror("Error opening file");
        return 1;
    }
    char buffer[1024]; // Buffer to read lines
    int Processno = 0;
    int id, at, rt, pr, ms;
    // Read file and create processes and enqueue in ProcessQueue
    while (fgets(buffer, sizeof(buffer), inputfile) != NULL)
    {
        // Check if the first character of the line is #
        if (buffer[0] != '#')
        {
            Processno++;
            sscanf(buffer, "%d    %d  %d  %d    %d", &id, &at, &rt, &pr, &ms);
            struct Process *newp = Create_Process(id, at, rt, pr, ms);
            enqueue(ProcessQueue, newp);
        }
    }
    //===========================================================//

    // Initiate and create the scheduler and clock processes.
    int pid = fork();
    if (pid == 0)
    {
        execl("./scheduler.out", "scheduler.out", NULL);
    }
    else
    {        
        SchedulerId = pid;
    }

    // Create message Queue
    key_t key_id, key_id2;

    // receive shared memory for clock
    shmid = shmget(SHKEY, 4, IPC_CREAT | 0644);

    key_id = ftok("keyfile", SCHEDULER_KEY);
    msgq_id = msgget(key_id, 0666 | IPC_CREAT);

    if (msgq_id == -1)
    {
        perror("Error in create");
        exit(-1);
    }

    // sending the algorithm chosen type
    struct msgbuff message;
    message.mtype = SCHEDULER_INFO_LOCK;
    message.algotype = algo;
    message.quantum = quantum;
    message.NumOfProcesses = Processno;
    
    //send the initial scheduling data
    send_val = msgsnd(msgq_id, &message, sizeof(message) - sizeof(long), !IPC_NOWAIT);
    if (send_val == -1)
        perror("Error in send");

    int pid2 = fork();
        if (pid2 == 0)
        {
            system("./clk.out");
        }
        initClk();

    int x = getClk();
    int prev_c = -1;
    
    
    while (1)
    {
        int c = getClk();
        if (c != prev_c)
        {
            int wPid, status;
            printf("Clock = %d\n", c);

            // send processes to scheduler as long as there is processes arrived
            while (isEmpty(ProcessQueue) != 1)
            {
                struct Process *p = peek(ProcessQueue);
                if (c >= p->arrivalTime)
                {
                    // sending processes
                    printf("sending process id %d at time %d\n",p->id,c);
                    message.process = *p;
                    message.mtype = SCHEDULER_LOCK;
                    
                    send_val = msgsnd(msgq_id, &message, sizeof(message) - sizeof(long), !IPC_NOWAIT);
                    
                    if (send_val == -1)
                        perror("Error in send");

                    p = dequeue(ProcessQueue);
                }
                else
                {
                    break;
                }
            }

            if (isEmpty(ProcessQueue) == 1)
            {
                int p = waitpid(SchedulerId, &status, 0); // check that Scheduler has finished(it worked !!)
                if (SchedulerId == p)
                {
                    break;
                }
                if (status == 1)
                {
                    break;
                }
            }
        }
        prev_c = c;
    }

    printf("Finished scheduling the processes\n");

    // Create a new GtkWindow
    GtkWidget *sc1Window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(sc1Window), "Scheduler.perf");
    gtk_window_set_default_size(GTK_WINDOW(sc1Window), 400, 300);
    g_signal_connect(sc1Window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create a new GtkImage widget to display the text file
    GtkWidget *image1 = gtk_image_new();

    // Render the text file onto a GdkPixbuf
    GdkPixbuf *pixbuf1;
    renderTextFile("scheduler.perf", &pixbuf1);

    // Set the GdkPixbuf as the image data
    gtk_image_set_from_pixbuf(GTK_IMAGE(image1), pixbuf1);

    // Add the GtkImage widget to the GtkWindow
    gtk_container_add(GTK_CONTAINER(sc1Window), image1);

    // Show all widgets
    gtk_widget_show_all(sc1Window);

    // Save the GdkPixbuf to a file
    savePixbufToFile(pixbuf1, "scheduler_perf.jpg");

    // Clean up
    g_object_unref(pixbuf1);

    GtkWidget *sc2Window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(sc2Window), "Scheduler.log");
    gtk_window_set_default_size(GTK_WINDOW(sc2Window), 600, 600);
    g_signal_connect(sc2Window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create a new GtkImage widget to display the text file
    GtkWidget *image2 = gtk_image_new();

    // Render the text file onto a GdkPixbuf
    GdkPixbuf *pixbuf2;
    renderTextFile("scheduler.log", &pixbuf2);

    // Set the GdkPixbuf as the image data
    gtk_image_set_from_pixbuf(GTK_IMAGE(image2), pixbuf2);

    // Add the GtkImage widget to the GtkWindow
    gtk_container_add(GTK_CONTAINER(sc2Window), image2);

    // Show all widgets
    gtk_widget_show_all(sc2Window);

    // Save the GdkPixbuf to a file
    savePixbufToFile(pixbuf2, "scheduler_log.jpg");

    // Clean up
    g_object_unref(pixbuf2);



    //////////////////////////
     GtkWidget *sc3Window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(sc3Window), "Memory.log");
    gtk_window_set_default_size(GTK_WINDOW(sc3Window), 600, 600);
    g_signal_connect(sc3Window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create a new GtkImage widget to display the text file
    GtkWidget *image3 = gtk_image_new();

    // Render the text file onto a GdkPixbuf
    GdkPixbuf *pixbuf3;
    renderTextFile("memory.log", &pixbuf3);

    // Set the GdkPixbuf as the image data
    gtk_image_set_from_pixbuf(GTK_IMAGE(image3), pixbuf3);

    // Add the GtkImage widget to the GtkWindow
    gtk_container_add(GTK_CONTAINER(sc3Window), image3);

    // Show all widgets
    gtk_widget_show_all(sc3Window);

    // Save the GdkPixbuf to a file
    savePixbufToFile(pixbuf3, "memory_log.jpg");

    // Clean up
    g_object_unref(pixbuf3);

    // clear shared memory and destroy clk
    shmctl(shmid, IPC_RMID, NULL);
    destroyClk(true);
}

// clear all shared memory and message queues
void clearResources(int signum)
{
    printf("cleared resources\n");
    killpg(getpgrp(), SIGKILL);
    msgctl(msgq_id, IPC_RMID, NULL);
    msgctl(msgq_id2, IPC_RMID, NULL);
}
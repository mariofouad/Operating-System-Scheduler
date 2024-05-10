#include <gtk/gtk.h>
#include <cairo.h>

void renderTextFile(const char *filename, GdkPixbuf **pixbuf) {
    // Open the text file
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        g_print("Error: Cannot open file.\n");
        return;
    }

    // Create a new Cairo surface to render the text
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 400, 300);
    cairo_t *cr = cairo_create(surface);

    // Set background color to white
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    // Set font size and color
    cairo_set_font_size(cr, 12);
    cairo_set_source_rgb(cr, 0, 0, 0);

    // Read and render each line of the text file
    double y = 20; // Starting y-coordinate
    char line[1024];
    while (fgets(line, sizeof(line), file) != NULL) {
        cairo_move_to(cr, 20, y);
        cairo_show_text(cr, line);
        y += 20; // Increase y-coordinate for the next line
    }

    // Finish rendering
    cairo_destroy(cr);

    // Convert Cairo surface to GdkPixbuf
    *pixbuf = gdk_pixbuf_get_from_surface(surface, 0, 0, 400, 300);

    // Clean up
    cairo_surface_destroy(surface);
    fclose(file);
}

void savePixbufToFile(GdkPixbuf *pixbuf, const char *filename) {
    // Save the GdkPixbuf to a file
    gdk_pixbuf_save(pixbuf, filename, "png", NULL, NULL);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Create a new GtkWindow
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Text File Viewer");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create a new GtkImage widget to display the text file
    GtkWidget *image = gtk_image_new();

    // Render the text file onto a GdkPixbuf
    GdkPixbuf *pixbuf;
    renderTextFile("scheduler.perf", &pixbuf);

    // Set the GdkPixbuf as the image data
    gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);

    // Add the GtkImage widget to the GtkWindow
    gtk_container_add(GTK_CONTAINER(window), image);

    // Show all widgets
    gtk_widget_show_all(window);

    // Save the GdkPixbuf to a file
    savePixbufToFile(pixbuf, "scheduler_perf.jpg");

    // Start the GTK main loop
    gtk_main();

    // Clean up
    g_object_unref(pixbuf);

    return 0;
}

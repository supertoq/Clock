/* Clock Copyright (c) 2026 supertoq
 * LICENSE: BSD 3-Clause "Modified BSD License"
 * https://opensource.org/license/BSD-3-clause
 *
 * gcc $(pkg-config --cflags gtk4 libadwaita-1) -o clock clock.c $(pkg-config --libs gtk4 libadwaita-1)
 *
 * clock.c
 * Uhrzeit mit dedizierten Thread für die Zeitmessung unabhängig von der GUI
 * 2026-01-22
 *
 * Please note:
 * The Use of this code and execution of the applications is at your own risk, I accept no liability!
 */
// 
#define APP_VERSION    "0.1.9"//_0
#define APP_ID         "io.github.supertoq.clock"
#define APP_NAME       "Clock"
#define APP_DOMAINNAME "supertoq-clock"
/* Fenster in Breite u. Höche (min 370, 200) */
#define WIN_WIDTH      400
#define WIN_HEIGHT     200
#define MY_FONT        "Adwaita Mono" // Monospace, DejaVu Sans, Source Code Pro, Open Sans, Adwaita Mono

#include <glib.h>
#include <gtk/gtk.h>
#include <adwaita.h>
#include <time.h>           // für time()
#include <locale.h>         // für setlocale(LC_ALL, "")
#include <glib/gi18n.h>     // für " _(); "

//#include "icon-gresource.h"

/* Voraussetzung zum kompilieren - libadwaite 1.8 */
#ifndef ADW_VERSION_1_8
#error "NOT building against libadwaita 1.8 headers"
#endif

/* ----- Globale Strukturen ------------------------------------------ */
typedef struct {               // ToastOverlay
    AdwToastOverlay *toast_overlay;
} ToastManager;
static ToastManager toast_manager = { NULL };

typedef struct {                    // AppData-Struktur
    AdwNavigationView *nav_view;    // das NavView der App
    GtkWidget         *drawing;     // Widget wird beim Skalieren neu gezeichnet, in draw_callback()
    GtkWidget         *btn_timer;   // Schaltfläche für Timer
    guint             timer_id;     // Timer ID
    char              time_str[16]; // Array für Uhrzeit als String mit 16 Bytes
    double        cached_font_size; // für zukünftigen Umbau
    int           last_w, last_h;   // -"-
} AppData;



/* ----- Callback: About-Dialog öffnen ------------------------------ */
static void show_about(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{ (void)action; (void)parameter;

    AdwApplication *app = ADW_APPLICATION(user_data);
    /* About‑Dialog anlegen */
    AdwAboutDialog *about = ADW_ABOUT_DIALOG(adw_about_dialog_new());
    adw_about_dialog_set_application_name(about, APP_NAME);
    adw_about_dialog_set_version(about, APP_VERSION);
    adw_about_dialog_set_developer_name(about, "supertoq");
    adw_about_dialog_set_website(about, "https://github.com/supertoq/Clock");
    //adw_about_dialog_set_comments(about, " ... ");

    /* Lizenz – BSD3  */
    adw_about_dialog_set_license_type(about, GTK_LICENSE_BSD);
    adw_about_dialog_set_license(about,
        "Copyright © 2026, supertoq\n\n"
        "This program comes WITHOUT ANY WARRANTY.\n"
        "Follow the link to view the license details: "
        "<a href=\"https://opensource.org/license/BSD-3-clause\"><b>Modified BSD License</b></a>\n"
        "\n"
        "Application Icons by SVG Repo. \n"
        "<a href=\"https://www.svgrepo.com\">www.svgrepo.com</a> \n"
        "Thanks to SVG Repo for sharing their free icons, "
        "we appreciate your generosity and respect your work.\n"
        "The icons are licensed under the \n"
        "<a href=\"https://www.svgrepo.com/page/licensing/#CC%20Attribution\">"
        "Creative Commons Attribution License.</a> \n"
        "Colours, shapes, and sizes of the symbols (icons) have been slightly modified from the original, "
        "some symbols have been combined with each other.\n"
        );

    /* Dialog-Icon aus g_resource */
    adw_about_dialog_set_application_icon(about, APP_ID);   //IconName

    /* Dialog innerhalb (modal) des Haupt-Fensters anzeigen */
    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(
                                   gtk_application_get_active_window(GTK_APPLICATION(app)) )));
    adw_dialog_present(ADW_DIALOG(about), GTK_WIDGET(parent));

} // Ende About-Dialog

static void settings_page(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{ (void)action; (void)parameter;

    AdwNavigationView *settings_nav = ADW_NAVIGATION_VIEW(user_data);

    /* ----- ToolbarView für Settings-Seite ----- */
    AdwToolbarView *settings_toolbar = ADW_TOOLBAR_VIEW(adw_toolbar_view_new());

    /* ----- Headerbar erzeugen ----- */
    AdwHeaderBar *settings_header = ADW_HEADER_BAR(adw_header_bar_new());
    GtkWidget *settings_label = gtk_label_new(_("Einstellungen"));
    gtk_widget_add_css_class(settings_label, "heading");
    adw_header_bar_set_title_widget(settings_header, settings_label);

    /* ----- Headerbar einfügen ----- */
    adw_toolbar_view_add_top_bar(settings_toolbar, GTK_WIDGET(settings_header));

    /* ----- Haupt-BOX der Settings-Seite ----- */
    GtkWidget *settings_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top(settings_box,    12);   // Rand unterhalb Toolbar
    gtk_widget_set_margin_bottom(settings_box, 12);   // unterer Rand unteh. der Buttons
    gtk_widget_set_margin_start(settings_box,  12);   // links
    gtk_widget_set_margin_end(settings_box,    12);   // rechts


// ... !!

    /* ----- ScrolledWindow erstellen und in die settingsBOX einfügen ----- */
    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), settings_box);

    /* ----- ToolbarView Inhalt in ScrolledWindow einsetzen ----- */
    adw_toolbar_view_set_content(settings_toolbar, scrolled_window);

    /* ----- NavigationPage anlegen ----- */
    AdwNavigationPage *settings_page = 
                      adw_navigation_page_new(GTK_WIDGET(settings_toolbar), _("Einstellungen"));

    /* ----- Größe nur zum Ausgleichen der Textlänge bei "Große Schrift" ----- */
//    gtk_widget_set_size_request(GTK_WIDGET(settings_page), WIN_WIDTH, WIN_HEIGHT);

    /* ----- Page der Settings_nav hinzufügen ----- */
    adw_navigation_view_push(settings_nav, settings_page);
}// Ende Einstellungen-Fenster



/* ----- Zeit-Callback (Zustandsaktualisierung) ----------------------- */
static gboolean time_ticker(gpointer user_data)
{
    /* AppData aus dem Zeiger holen */
    AppData *app_data = user_data;

    /* Widget auf Gültigkeit prüfen */
    if (!GTK_IS_WIDGET(app_data->drawing))
    return G_SOURCE_REMOVE;

    /* Aktuelle Zeit holen */
    time_t now = time(NULL); // Systemzeit
    struct tm local_time;    // Struktur für Zeit
    localtime_r(&now, &local_time); // (r = reentrant = nicht global, jeder Aufruf in eigenen Speicher); 
                                    // (& = in orig. Variable hineinschreiben)

    /* in String umwandeltn */
    strftime(app_data->time_str, sizeof(app_data->time_str), "%H:%M:%S", &local_time);

    /* Setze den Text im Label */
    //gtk_label_set_text(GTK_LABEL(app_data->time_label), app_data->time_str);

    /* Markieren des Widgets als „dirty“ -  für Neuzeichnen */
    gtk_widget_queue_draw(app_data->drawing);

    return G_SOURCE_CONTINUE; // weiterlaufen
}

/* ----- Berechnung der Schriftgröße für einen skalierbaren Raum ----- */
static double fit_font_size(cairo_t *ct, const char *label_text, int width, int height)
{   // cairo_t = Zeichenkontext-Typ (ausgerüsteter Zeichnungsroboter)

    double size = height; // Ausgangswert = Höhe

    cairo_set_font_size(ct, size); // Schriftgröße im Cairo-Kontext setzen

    cairo_text_extents_t ext; // = Struktur mit exakten geometrischen Daten
    cairo_text_extents(ct, label_text, &ext); // Cairo misst Text relativ zur gesetzten Schriftgröße

    /* Skalierungsfaktoren berechnen */
    double scale_w = width  / ext.width;  // max. passende vertik. Skalierung 
    double scale_h = height / ext.height; // max. passende horizont. Skalierung 
                                          // Beispiel: Text = 300 px, Fensterbreite ist 200 px
                                          // scale_w = 200 / 300 = 0.66

    /* MIN() = kleinsten von beiden w und h Werten verwenden hernehmen */ 
    double min_scale = MIN(scale_w, scale_h);
    /* Größe berechnen */
    double result = size * min_scale * 0.85; // x 0.85 Sicherheitsabstand
    return result; // Grö0en-Wert zurückmelden
}

/* ----- Zeichenfunktion (wird von GTK selbst aufgerufen) ------------- */
static void draw_callback(GtkDrawingArea *widget_area, cairo_t *ct, int width, int height, gpointer user_data)
{  // cairo_t *cr = Zeichenkontext-Typ; user_date = AppData*

    AppData *app = user_data;

    /* Hintergrund */
//    cairo_set_source_rgb(ct, 1.0, 1.0, 1.0);
//    cairo_paint(ct);

    /* Vordergrund, Textfarbe aus Theme ermitteln */
    GdkRGBA color;
    gtk_widget_get_color(GTK_WIDGET(widget_area), &color);
    cairo_set_source_rgba(ct, color.red, color.green, color.blue, color.alpha);

    /* Schrift vorbereiten */
    cairo_select_font_face(
             ct, MY_FONT,               // Schriftart !! 
             CAIRO_FONT_SLANT_NORMAL,   // normal / italic / oblique
             CAIRO_FONT_WEIGHT_NORMAL   // normal / bold
    );

    /* Schriftgröße berechnen (Zahlen sind hier Platzhalter für Breite) */
    double font_size = fit_font_size(ct, "08:08:08", width, height);
    cairo_set_font_size(ct, font_size);
    //cairo_set_font_size(ct, 40); // testen !!

    /* Text-Ausmaße messen */
    cairo_text_extents_t ext; // = Struktur mit exakten geometrischen Daten
    cairo_text_extents(ct, app->time_str, &ext);
    
    /* Textzentrierung */
    double x = (width  - ext.width)  / 2 - ext.x_bearing;
    double y = (height - ext.height) / 2 - ext.y_bearing;

    /* Text positionieren */
    cairo_move_to(ct, x, y);
    //cairo_move_to(ct, 10, 50); // testen !!

    /* Text zeichnen */
    cairo_show_text(ct, app->time_str);
    //cairo_show_text(ct, "12:34:56"); // testen !!
}

static void timer_page(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{ (void)action; (void)parameter;

    AppData *app_data = (AppData *)user_data;

    /* Gültigkeit des NavView prüfen, zum testen ! */
    g_assert(ADW_IS_NAVIGATION_VIEW(app_data->nav_view));
//    g_print("nav_view: %p\n", app_data->nav_view);
//    g_print("is valid: %d\n", ADW_IS_NAVIGATION_VIEW(app_data->nav_view));

     // Hinweis: AdwNavigationView = app_data->nav_view;

    /* ----- ToolbarView für Timer-Seite ----- */
    AdwToolbarView *timer_toolbar = ADW_TOOLBAR_VIEW(adw_toolbar_view_new());

    /* ----- Headerbar erzeugen ----- */
    AdwHeaderBar *timer_header = ADW_HEADER_BAR(adw_header_bar_new());
    GtkWidget *timer_label = gtk_label_new(_("Timer"));
    gtk_widget_add_css_class(timer_label, "heading");
    adw_header_bar_set_title_widget(timer_header, timer_label);

    /* ----- Headerbar einfügen ----- */
    adw_toolbar_view_add_top_bar(timer_toolbar, GTK_WIDGET(timer_header));

    /* ----- Haupt-BOX der Timer-Seite ----- */
    GtkWidget *timer_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
    gtk_widget_set_margin_top   (GTK_WIDGET(timer_box),    1);   // Rand unterhalb Toolbar
    gtk_widget_set_margin_bottom(GTK_WIDGET(timer_box),   12);   // unterer Rand unteh. der Buttons
    gtk_widget_set_margin_start (GTK_WIDGET(timer_box),   12);   // links
    gtk_widget_set_margin_end   (GTK_WIDGET(timer_box),   12);   // rechts

// ... !!

    /* ----- ScrolledWindow erstellen und in die Timer-BOX einfügen ----- */
    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), timer_box);

    /* ----- ToolbarView Inhalt in ScrolledWindow einsetzen ----- */
    adw_toolbar_view_set_content(timer_toolbar, scrolled_window);

    /* ----- NavigationPage anlegen ----- */
    AdwNavigationPage *timer_page = 
                      adw_navigation_page_new(GTK_WIDGET(timer_toolbar), _("Timer"));

    /* ----- Größe nur zum Ausgleichen der Textlänge bei "Große Schrift" ----- */
//    gtk_widget_set_size_request(GTK_WIDGET(timer_page), WIN_WIDTH, WIN_HEIGHT);

    /* ----- Page der Settings_nav hinzufügen ----- */
    adw_navigation_view_push(app_data->nav_view, timer_page);
}// Ende Timer-Fenster



/* Callback-Funktion für den Button */
static void on_timer_button_clicked(GtkButton *button, gpointer user_data)
{ (void)button;

    AppData *app_data = (AppData *)user_data;
    timer_page(NULL, NULL, app_data);
}

static void on_shutdown(GApplication *app, gpointer user_data)
{
    AppData *app_data = user_data;

    /* Auf vorhandenen Timer prüfen und beenden */
    if (app_data->timer_id) {
        g_source_remove(app_data->timer_id);
        app_data->timer_id = 0;
    }
    /* Aufräumen */
    g_free(app_data);
}

/* ------------------------------------------------------------------- */
/* Aktivierungshandler                                                 */
/* ------------------------------------------------------------------- */
static void on_activate(GApplication *app, gpointer user_data)
{

    /* Daten für Anwendungsstruktur */
    AppData *app_data = (AppData *)user_data;

    /* ----- Adwaita-Fenster ----------------------------------------- */
    AdwApplicationWindow *adw_win = ADW_APPLICATION_WINDOW(adw_application_window_new(GTK_APPLICATION(app)));
    gtk_window_set_title(GTK_WINDOW(adw_win), APP_NAME);
    gtk_window_set_default_size(GTK_WINDOW(adw_win), WIN_WIDTH, WIN_HEIGHT);
    gtk_window_set_resizable(GTK_WINDOW(adw_win), TRUE);

    /* --- NavigationView (in Struktur) ------------------------------ */
    app_data->nav_view = ADW_NAVIGATION_VIEW(adw_navigation_view_new());

    /* Überprüfung: */
    g_assert(app_data->nav_view != NULL); 

    /* --- ToolbarView ----------------------------------------------- */
    AdwToolbarView *toolbar_view = ADW_TOOLBAR_VIEW(adw_toolbar_view_new());

    /* --- HeaderBar ------------------------------------------------- */
    AdwHeaderBar *headerbar = ADW_HEADER_BAR(adw_header_bar_new());
    GtkWidget *title_label = gtk_label_new(NULL);
    gtk_widget_add_css_class(title_label, "heading");
    adw_header_bar_set_title_widget(headerbar, title_label);
    adw_toolbar_view_add_top_bar(toolbar_view, GTK_WIDGET(headerbar)); // Headerbar in ToolbarView

    /* --- Hauptseite in NavigationView ------------------------------ */
    AdwNavigationPage *main_page = adw_navigation_page_new(GTK_WIDGET(toolbar_view), APP_NAME);
    adw_navigation_view_push(app_data->nav_view, main_page);

    /* --- ToastOverlay ---------------------------------------------- */
    toast_manager.toast_overlay = ADW_TOAST_OVERLAY(adw_toast_overlay_new()); // ToastManager in Strukt.
    adw_toast_overlay_set_child(toast_manager.toast_overlay, GTK_WIDGET(app_data->nav_view));
    /* --- ToastOverlay einfügen  (ToastOverlay-Struktur) ------------ */
    adw_application_window_set_content(adw_win, GTK_WIDGET(toast_manager.toast_overlay));

     //  Widget Hierarchie:
     //      ApplicationWindow
     //      └---ToastOverlay
     //          └---NavigationView
     //              └---NavigationPage
     //                  └---ToolbarView
     //                      │
     //                      ├---HeaderBar
     //                      │   └---Titel
     //                      ├---Seiteninhalt
     //                      .

    /* --- Hamburger-Button innerhalb der Headerbar ------------------ */
    GtkMenuButton *menu_btn = GTK_MENU_BUTTON(gtk_menu_button_new());
    gtk_menu_button_set_icon_name(menu_btn, "open-menu-symbolic");
    adw_header_bar_pack_end(headerbar, GTK_WIDGET(menu_btn)); // Link in Headerbar

    /* --- Popover-Menu im Hamburger --------------------------------- */
    GMenu *menu = g_menu_new();
    g_menu_append(menu, _("Einstellungen         "), "app.show-settings");
    g_menu_append(menu, _("Infos zu Clock        "), "app.show-about");
    GtkPopoverMenu *menu_popover = GTK_POPOVER_MENU(
               gtk_popover_menu_new_from_model(G_MENU_MODEL(menu)));
    gtk_menu_button_set_popover(menu_btn, GTK_WIDGET(menu_popover));

    /* --- ACTION für Einstellungen u. About-Dialog ------------------ */
    const GActionEntry about_entry[] = 
    {
        { "show-about", // Bezeichnung    - einmalig er GActionMap
            show_about, // Activate       - Funktion wenn ausgelöst
                  NULL, // Parameter_type - welcher Datentyp erwartet werden soll
                  NULL, // State          - Zustand und Zustand-typ(b,s,i,d)
                  NULL, // Change_state   - für Callback wenn sich state ändert
                  { 0 } // Padding        - Erweiterungsfeld
        }
    }; 
    const GActionEntry settings_entry[] = 
    {
        { "show-settings", settings_page, NULL, NULL, NULL, { 0 } }
    };
    /* Registrierung der im Array about_entry definierten Aktionen */ 
    g_action_map_add_action_entries(G_ACTION_MAP(app), 
                                          about_entry, G_N_ELEMENTS(   about_entry), app);
    g_action_map_add_action_entries(G_ACTION_MAP(app), 
                                       settings_entry, G_N_ELEMENTS(settings_entry), app_data->nav_view);

// ...

    /* --- Haupt-BOX ------------------------------------------------- */
    GtkBox *main_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 1));
    gtk_widget_set_margin_top   (GTK_WIDGET(main_box),    1);   // Rand unterhalb Toolbar
    gtk_widget_set_margin_bottom(GTK_WIDGET(main_box),   24);   // unterer Rand unteh. der Buttons
    gtk_widget_set_margin_start (GTK_WIDGET(main_box),   20);   // links
    gtk_widget_set_margin_end   (GTK_WIDGET(main_box),   20);   // rechts
    gtk_widget_set_hexpand      (GTK_WIDGET(main_box), TRUE);
    gtk_widget_set_vexpand      (GTK_WIDGET(main_box), TRUE);

    /* --- Haupt-Box zur ToolbarView hinzufügen ------------------- */
    adw_toolbar_view_set_content(toolbar_view, GTK_WIDGET(main_box));

    /* --- DrawingWidget erstellen ------------- */
    app_data->drawing = gtk_drawing_area_new(); // Zeichenfläche für Cairo_t
    gtk_widget_set_hexpand(app_data->drawing, TRUE);
    gtk_widget_set_vexpand(app_data->drawing, TRUE);
    /* zum Neuzeichnen des Widgets soll GTK folgendes Callback aufrufen */
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(app_data->drawing), draw_callback, app_data, NULL);
    gtk_box_append(main_box, app_data->drawing);

    /* --- Schaltfläche "Timer" in Headerbar ------------------------- */
    app_data->btn_timer = gtk_button_new_with_label(_("Timer"));
    gtk_widget_add_css_class(app_data->btn_timer, "opaque"); // undurchsichtig
//    gtk_widget_add_css_class(app_data->btn_timer, "suggested-action");  // Theme-akzent
    gtk_widget_set_size_request(app_data->btn_timer, 100, 22);  // Breite 100, Höhe 50px
    adw_header_bar_pack_start(headerbar, app_data->btn_timer);
    g_signal_connect(app_data->btn_timer, "clicked", G_CALLBACK(on_timer_button_clicked), app_data);

// !! weitere Elemente hier ...

    /* --- Hauptfenster im Application-Objekt ablegen (!) --------- */
    g_object_set_data(G_OBJECT(app), "main-window", GTK_WINDOW(adw_win));

    /* --- Hauptfenster desktop-konform anzeigen ------------------ */
    gtk_window_present(GTK_WINDOW(adw_win));


    /* ----- Timer starten ---------------------------------------- */
    app_data->timer_id = g_timeout_add(500, time_ticker, app_data); // TimerID vergeben
}

/* ------------------------------------------------------------------- */
/* Anwendungshauptteil                                                 */
/* ------------------------------------------------------------------- */
int main(int argc, char **argv)
{

    g_autoptr(AdwApplication) app =
                        adw_application_new(APP_ID, G_APPLICATION_DEFAULT_FLAGS);

    /* Localiziation-Setup */
    const gchar *locale_path = NULL;
    setlocale(LC_ALL, "");
    textdomain(APP_DOMAINNAME);
    bind_textdomain_codeset(APP_DOMAINNAME, "UTF-8");
    locale_path = "/usr/share/locale";
    bindtextdomain(APP_DOMAINNAME, locale_path);

    /* Resource-Bundle registrieren */
//    g_resources_register(resources_get_resource());

    /* Anwendungs-Struktur init. */
//    AppData app_data;                       // als Stack-Allokation, testen !!
    AppData *app_data = g_new0(AppData, 1); // als Heap-Allokation

    g_signal_connect(app, "activate", G_CALLBACK(on_activate), app_data); // Strukturdaten übergeben
    g_signal_connect(app, "shutdown", G_CALLBACK(on_shutdown), app_data);

    /* Anwendung starten und auf Ereignis warten */
    return g_application_run(G_APPLICATION(app), argc, argv);

}
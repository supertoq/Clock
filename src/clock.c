/* Clock Copyright (c) 2026 supertoq
 * LICENSE: BSD 3-Clause "Modified BSD License"
 * https://opensource.org/license/BSD-3-clause
 *
 * gcc $(pkg-config --cflags gtk4 libadwaita-1) -o clock clock.c $(pkg-config --libs gtk4 libadwaita-1)
 *
 * main.c
 * Uhrzeit mit dedizierten Thread für die Zeitmessung unabhängig von der GUI
 * 2026-01-21
 *
 * Please note:
 * The Use of this code and execution of the applications is at your own risk, I accept no liability!
 */
// 
#define APP_VERSION    "0.1.0"//_0
#define APP_ID         "io.github.supertoq.clock"
#define APP_NAME       "Clock"
#define APP_DOMAINNAME "supertoq-clock"
/* Fenster in Breite u. Höche (370, 200) */
#define WIN_WIDTH      420
#define WIN_HEIGHT     210


#include <glib.h>
#include <gtk/gtk.h>
#include <adwaita.h>
#include <time.h>           // für time()
#include <unistd.h>         // POSIX-Header
#include <locale.h>         // für setlocale(LC_ALL, "")
#include <glib/gi18n.h>     // für " _(); "

//#include "icon-gresource.h"

/* Voraussetzung zum kompilieren - libadwaite 1.8 */
#ifndef ADW_VERSION_1_8
#error "NOT building against libadwaita 1.8 headers"
#endif

/* ----- Globale Strukturen ------------------------------------------ */
typedef struct {               // Für ToastOverlay
    AdwToastOverlay *toast_overlay;
} ToastManager;
static ToastManager toast_manager = { NULL };

typedef struct { 
    char time_str[32];       // Für die Zeit
    GtkWidget *time_label;   // Label
} AppData;

/* ----- Zeit-Callback ----------------------------------------------- */
static gboolean time_ticker(gpointer user_data)
{
    AppData *app_data = user_data;

    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);

    // Formatiere die Zeit
    strftime(app_data->time_str, sizeof(app_data->time_str), "%H:%M:%S", local_time);

    // Setze den Text im Label
    gtk_label_set_text(GTK_LABEL(app_data->time_label), app_data->time_str);

    return G_SOURCE_CONTINUE;
}
/* ------------------------------------------------------------------- */
/* Aktivierungshandler                                                 */
/* ------------------------------------------------------------------- */
static void on_activate(GApplication *app, gpointer user_data)
{
    /* ----- CSS-Provider für zusätzliche Anpassungen --------------- */
    // orange=#db9c4a , lightred=#ff8484 , grey=#c0bfbc
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider,
                                                         ".time_label {"
                                                    "  font-size: 64px;"
                                                                     "}"
                                                                     );
    gtk_style_context_add_provider_for_display(gdk_display_get_default(),
    GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

    AppData *app_data = (AppData *)user_data;

    /* ----- Adwaita-Fenster ----------------------------------------- */
    AdwApplicationWindow *adw_win = ADW_APPLICATION_WINDOW(adw_application_window_new(GTK_APPLICATION(app)));
    gtk_window_set_title(GTK_WINDOW(adw_win), APP_NAME);
    gtk_window_set_default_size(GTK_WINDOW(adw_win), WIN_WIDTH, WIN_HEIGHT);
    gtk_window_set_resizable(GTK_WINDOW(adw_win), TRUE);

    /* --- NavigationView -------------------------------------------- */
    AdwNavigationView *nav_view = ADW_NAVIGATION_VIEW(adw_navigation_view_new());

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
    adw_navigation_view_push(nav_view, main_page);

    /* --- ToastOverlay ---------------------------------------------- */
    toast_manager.toast_overlay = ADW_TOAST_OVERLAY(adw_toast_overlay_new()); // ToastManager in Strukt.
    adw_toast_overlay_set_child(toast_manager.toast_overlay, GTK_WIDGET(nav_view));
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
    adw_header_bar_pack_start(headerbar, GTK_WIDGET(menu_btn)); // Link in Headerbar

    /* --- Popover-Menu im Hamburger --------------------------------- */
    GMenu *menu = g_menu_new();
//    g_menu_append(menu, _("Einstellungen         "), "app.show-settings");
//    g_menu_append(menu, _("Infos zu OLED Saver   "), "app.show-about");
//    GtkPopoverMenu *menu_popover = GTK_POPOVER_MENU(
//               gtk_popover_menu_new_from_model(G_MENU_MODEL(menu)));
//    gtk_menu_button_set_popover(menu_btn, GTK_WIDGET(menu_popover));

    /* --- ACTION für Einstellungen u. About-Dialog ------------------ */
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

    /* --- Label erstellen ------------- */
    app_data->time_label = gtk_label_new(NULL);
    gtk_widget_set_hexpand(app_data->time_label, TRUE);
    gtk_widget_set_vexpand(app_data->time_label, TRUE);
    gtk_widget_add_css_class(app_data->time_label, "time_label");
    gtk_box_append(main_box, app_data->time_label);

// !! weitere Elemente hier ...

    /* --- Hauptfenster im Application-Objekt ablegen (!) --------- */
    g_object_set_data(G_OBJECT(app), "main-window", GTK_WINDOW(adw_win));

    /* --- Hauptfenster desktop-konform anzeigen ------------------ */
    gtk_window_present(GTK_WINDOW(adw_win));

    /* ----- Timer starten ---------------------------------------- */
    g_timeout_add(1000, time_ticker, user_data);
}

/* ------------------------------------------------------------------- */
/* Anwendungshauptteil                                                 */
/* ------------------------------------------------------------------- */
int main(int argc, char **argv)
{
    /* Anwendungs-Struktur init. */
    AppData app_data;
    g_autoptr(AdwApplication) app =
                        adw_application_new(APP_ID, G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), &app_data);

    /* Localiziation-Setup */
    const gchar *locale_path = NULL;
    setlocale(LC_ALL, "");
    textdomain(APP_DOMAINNAME);
    bind_textdomain_codeset(APP_DOMAINNAME, "UTF-8");
    locale_path = "/usr/share/locale";
    bindtextdomain(APP_DOMAINNAME, locale_path);

    /* Resource-Bundle registrieren */
//    g_resources_register(resources_get_resource());

    /* Anwendung starten und auf Ereignis warten */
    return g_application_run(G_APPLICATION(app), argc, argv);

}
/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *		Serghei Amelian <serghei@amelian.ro>
 *		Ivan Korytov <toreonify@outlook.com>
 *
 * Copyright (C) 2021 Serghei Amelian
 * Copyright (C) 2023 Ivan Korytov
 */

#include "systray.h"

/* Forward Declarations */
GType e_systray_get_type (void);
G_DEFINE_DYNAMIC_TYPE (ESystray, e_systray, E_TYPE_EXTENSION)

static gboolean
on_window_state_event (GtkWidget *widget, GdkEventWindowState *event, ESystrayPrivate* priv)
{
    GdkWindow *gdk_window = gtk_widget_get_window(GTK_WIDGET(priv->window));
    GdkWindowState state = gdk_window_get_state(gdk_window);
    priv->window_active = !((state & GDK_WINDOW_STATE_WITHDRAWN) || (state & GDK_WINDOW_STATE_ICONIFIED) || !(state & GDK_WINDOW_STATE_FOCUSED));

    if (!priv->window_active)
    {
        gtk_menu_item_set_label(priv->visibility_item, _D("Show"));
    }
    else
    {
        gtk_menu_item_set_label(priv->visibility_item, _D("Minimize"));
    }
}

static gboolean
on_window_close_alert(GtkWindow *window, GdkEvent *event, GtkMenuItem * item)
{
    if (event->type == GDK_DELETE)
    {
        gtk_widget_hide((GtkWidget*) window);

        return TRUE;
    }

    return FALSE;
}

static gboolean
on_activate(GtkMenuItem *item, ESystrayPrivate* priv)
{
    if (priv->window_active)
    {
        gtk_widget_hide((GtkWidget*) priv->window);
    }
    else
    {
#ifdef GDK_WINDOWING_WAYLAND
        GdkWindow *gdk_window = gtk_widget_get_window(GTK_WIDGET(priv->window));
        GdkWindowState state = gdk_window_get_state(gdk_window);

        if (GDK_IS_WAYLAND_DISPLAY(gtk_widget_get_display(GTK_WIDGET(priv->window)))
            && !(state & GDK_WINDOW_STATE_WITHDRAWN))
        {
            gtk_widget_hide((GtkWidget*) priv->window);
        }
#endif
        gtk_window_deiconify(priv->window);
        gtk_window_present_with_time(priv->window, g_get_monotonic_time () / 1000L);
    }

    return TRUE;
}

gboolean
get_total_unread_messages(GtkTreeModel *model,
                                GtkTreePath  *path,
                                GtkTreeIter  *iter,
                                gpointer user_data)
{
    guint unread = 0;
    guint flags = 0;
    gboolean is_draft = FALSE;
    guint* total_unread_messages = (guint*) user_data;

    // is_draft available only for normal accounts to test if it's a Drafts folder
    // It's unread count equals to all drafts in folder
    gtk_tree_model_get(model, iter, COL_BOOL_IS_DRAFT, &is_draft, -1);
    // For EWS acoounts, we can check flags
    gtk_tree_model_get(model, iter, COL_UINT_FLAGS, &flags, -1);

    if (is_draft || (flags & CAMEL_FOLDER_TYPE_MASK) == CAMEL_FOLDER_TYPE_DRAFTS)
    {
        return FALSE;
    }

    gtk_tree_model_get(model, iter, COL_UINT_UNREAD, &unread, -1);

    if (unread > 0)
        *total_unread_messages += unread;

    return FALSE;
}

void
on_unread_updated(MailFolderCache *folder_cache,
                       CamelStore *store,
                       const gchar *folder_name,
                       gint unread_messages,
                       ESystrayPrivate* priv)
{
    int total_unread_messages = 0;
    gtk_tree_model_foreach(priv->model, get_total_unread_messages, &total_unread_messages);

    if (total_unread_messages > 0)
    {
        gchar *num = g_strdup_printf(_DN("%d unread message", "%d unread messages", total_unread_messages), total_unread_messages);

        status_notifier_item_set_from_icon_name (priv->sn, STATUS_NOTIFIER_ICON, "mail-unread-new");
        status_notifier_item_set_tooltip_body (priv->sn, num);

        g_free(num);
    }
    else
    {
        status_notifier_item_set_from_icon_name (priv->sn, STATUS_NOTIFIER_ICON, "mail-unread");
        status_notifier_item_set_tooltip_body (priv->sn, _D("No unread messages"));
    }
}

static gboolean
menu_quit_click(GtkWidget *window, gpointer user_data)
{
    EShell *shell = e_shell_get_default();

    e_shell_quit(shell, E_SHELL_QUIT_ACTION);
}

static gboolean
sn_menu (StatusNotifierItem *sn, gint x, gint y, ESystrayPrivate* priv)
{
    if (!priv->menu)
    {
        g_object_ref_sink (priv->menu);
    }

    /* All of that because gtk_menu_popup is deprecated */
    /* and GdkEvent is not passed by StatusNotifier */
    GdkEvent* event = gdk_event_new(GDK_BUTTON_PRESS);
    GdkWindow *gdk_window = gtk_widget_get_window(GTK_WIDGET(priv->window));
    GdkSeat* seat = gdk_display_get_default_seat(gdk_display_get_default());

    ((GdkEventButton*)event)->window = gdk_window;
    ((GdkEventButton*)event)->device = gdk_seat_get_pointer(seat);

    gtk_menu_popup_at_pointer (priv->menu, event);

    return TRUE;
}

static gboolean
sn_activate (ESystrayPrivate* priv)
{
    return on_activate(priv->visibility_item, priv);
}


/* ESystray methods */
static void
e_systray_init (ESystray *extension)
{
}

static void
e_systray_constructed (GObject *object)
{
    ESystray* extension = NULL;

    /* Chain up to parent's method. */
    G_OBJECT_CLASS (e_systray_parent_class)->constructed (object);

    extension = (ESystray*) E_EXTENSION (object);
    extension->priv = malloc(sizeof(ESystrayPrivate));

    /* This retrieves the EShell instance we're extending. */
    EExtensible *extensible = e_extension_get_extensible (E_EXTENSION(extension));
    EShellWindow *shell_window = E_SHELL_WINDOW(extensible);
    GtkWindow *window = GTK_WINDOW(extensible);

    extension->priv->window = window;
    g_signal_connect(window, "window-state-event", G_CALLBACK(on_window_state_event), extension->priv);

    /* Create StatusNotifier */
    extension->priv->sn = g_object_new (STATUS_NOTIFIER_TYPE_ITEM,
        "id",               "evolution-systray",
        "category",         STATUS_NOTIFIER_CATEGORY_APPLICATION_STATUS,
        "status",           STATUS_NOTIFIER_STATUS_ACTIVE,
        "title",            "Evolution",
        "item-is-menu",     false,
        NULL);

    status_notifier_item_set_tooltip_title (extension->priv->sn, "Evolution");
    status_notifier_item_set_tooltip_body (extension->priv->sn, _D("No unread messages"));

    /* Create context menu */
    GtkMenu* menu = (GtkMenu*) gtk_menu_new();

    GtkWidget *item = gtk_menu_item_new_with_label (_D("Minimize"));
    extension->priv->visibility_item = (GtkMenuItem*) item;

    g_signal_connect(item, "activate", G_CALLBACK(on_activate), extension->priv);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    gtk_widget_show(item);

    g_signal_connect_after(G_OBJECT(extensible), "event", G_CALLBACK(on_window_close_alert), item);

    item = gtk_separator_menu_item_new ();
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    gtk_widget_show(item);

    item = gtk_menu_item_new_with_label (_D("Quit"));
    g_signal_connect (item, "activate", G_CALLBACK (menu_quit_click), extension->priv->sn);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    gtk_widget_show(item);

    /* Set context menu for tray icon */
    extension->priv->menu = menu;
    bool dbusmenu_status = status_notifier_item_set_context_menu (extension->priv->sn, (GObject *) extension->priv->menu);

    if (dbusmenu_status) {
        status_notifier_item_set_item_is_menu (extension->priv->sn, TRUE);
    } else {
        g_signal_connect (extension->priv->sn, "context-menu", (GCallback) sn_menu, extension->priv);
    }

    // TODO: error handler if creation of tray icon fails
    // g_signal_connect (extension->priv->sn, "registration-failed", (GCallback) sn_reg_failed, NULL);

    /* Enable tray icon */
    g_signal_connect_swapped (extension->priv->sn, "activate", (GCallback) sn_activate, extension->priv);
    status_notifier_item_register (extension->priv->sn);

    /* Connect callback to Evolution for monitoring unread messages count */
    EShell *shell = e_shell_window_get_shell(shell_window);

    EShellView *shell_view = e_shell_window_get_shell_view(shell_window, "mail");
    EShellSidebar *shell_sidebar = e_shell_view_get_shell_sidebar (shell_view);

    EMFolderTree *folder_tree;
    g_object_get (shell_sidebar, "folder-tree", &folder_tree, NULL);

    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(folder_tree));
    EMailSession *email_session = em_folder_tree_model_get_session(EM_FOLDER_TREE_MODEL(model));
    MailFolderCache *mail_folder_cache = e_mail_session_get_folder_cache(email_session);

    extension->priv->model = model;

    g_signal_connect(G_OBJECT(mail_folder_cache), "folder-unread-updated", G_CALLBACK(on_unread_updated), extension->priv);
}

static void
e_systray_finalize (GObject *object)
{
    ESystray* extension = (ESystray*) object;
    g_object_unref (extension->priv->sn);
    free(extension->priv);

    /* Chain up to parent's finalize() method. */
    G_OBJECT_CLASS (e_systray_parent_class)->finalize (object);
}

/* ESystrayClass methods */
static void
e_systray_class_init (ESystrayClass *class)
{
    GObjectClass *object_class;
    EExtensionClass *extension_class;

    object_class = G_OBJECT_CLASS (class);
    object_class->constructed = e_systray_constructed;
    object_class->finalize = e_systray_finalize;

    /* Specify the GType of the class we're extending.
     * The class must implement the EExtensible interface. */
    extension_class = E_EXTENSION_CLASS (class);
    extension_class->extensible_type = E_TYPE_SHELL_WINDOW;
}

static void
e_systray_class_finalize (ESystrayClass *class)
{
}

/* Module Entry Points */
G_MODULE_EXPORT void
e_module_load (GTypeModule *type_module)
{
    e_systray_register_type (type_module);
}

G_MODULE_EXPORT void
e_module_unload (GTypeModule *type_module)
{
}

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

#ifndef _SYSTRAY_H_
#define _SYSTRAY_H_

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <glib/gi18n.h>
#include <locale.h>
#include <statusnotifier.h>

#include <shell/e-shell.h>
#include <shell/e-shell-view.h>
#include <shell/e-shell-window.h>
#include <mail/em-folder-tree.h>

#include <libebackend/libebackend.h>

#include <camel/camel.h>

#define _D(VA...) dgettext("evolution-systray", VA)
#define _DN(VA...) dngettext("evolution-systray", VA)

typedef struct _ESystray ESystray;
typedef struct _ESystrayClass ESystrayClass;
typedef struct _ESystrayPrivate ESystrayPrivate;

struct _ESystray {
        EExtension parent;

        ESystrayPrivate* priv;
};

struct _ESystrayClass {
        EExtensionClass parent_class;
};

struct _ESystrayPrivate {
        GtkWindow* window;

        StatusNotifierItem* sn;
        GtkMenu* menu;
        GtkMenuItem* visibility_item;

        GtkTreeModel *model;
};

/* Module Entry Points */
void e_module_load (GTypeModule *type_module);
void e_module_unload (GTypeModule *type_module);

#endif

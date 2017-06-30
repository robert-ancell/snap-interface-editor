/*
 * Copyright (C) 2017 Canonical Ltd.
 * Author: Robert Ancell <robert.ancell@canonical.com>
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version. See http://www.gnu.org/copyleft/gpl.html the full text of the
 * license.
 */

#include <gtk/gtk.h>
#include <snapd-glib/snapd-glib.h>

#include "snap-combo-box.h"
#include "snap-interface-combo-box.h"
#include "snap-interface-switch.h"

static SnapdClient *client = NULL;
static GPtrArray *slots = NULL;
static GPtrArray *plugs = NULL;

static GtkWidget *grid = NULL;

static SnapdSnap *
find_snap (GPtrArray *snaps, const gchar *name)
{
    guint i;

    for (i = 0; i < snaps->len; i++) {
        SnapdSnap *snap = g_ptr_array_index (snaps, i);
        if (g_strcmp0 (snapd_snap_get_name (snap), name) == 0)
            return snap;
    }

    return NULL;
}

static gint
snap_index (GPtrArray *snaps, SnapdSnap *snap)
{
    gint i;

    for (i = 0; i < snaps->len; i++) {
        SnapdSnap *s = g_ptr_array_index (snaps, i);
        if (s == snap)
            return i;
    }

    return -1;
}

static gint
compare_snap_names (SnapdSnap **a, SnapdSnap **b)
{
    return g_strcmp0 (snapd_snap_get_name (*a), snapd_snap_get_name (*b));
}

static GPtrArray *
get_configurable_snaps (GPtrArray *snaps, GPtrArray *plugs)
{
    GPtrArray *configurable_snaps;
    guint i;

    configurable_snaps = g_ptr_array_new_with_free_func (g_free);
    for (i = 0; i < plugs->len; i++) {
        SnapdPlug *plug = g_ptr_array_index (plugs, i);
        SnapdSnap *snap;

        snap = find_snap (snaps, snapd_plug_get_snap (plug));
        if (snap == NULL)
            continue;
        if (snap_index (configurable_snaps, snap) == -1)
            g_ptr_array_add (configurable_snaps, g_object_ref (snap));
    }
    g_ptr_array_sort (snaps, (GCompareFunc) compare_snap_names);

    return configurable_snaps;
}

static SnapdSlot *
get_core_slot (GPtrArray *slots, SnapdPlug *plug)
{
    guint i;
    SnapdSlot *core_slot = NULL;

    for (i = 0; i < slots->len; i++) {
        SnapdSlot *slot = g_ptr_array_index (slots, i);

        if (g_strcmp0 (snapd_plug_get_interface (plug), snapd_slot_get_interface (slot)) != 0)
            continue;

        if (core_slot != NULL)
            return NULL;
        if (g_strcmp0 (snapd_slot_get_snap (slot), "core") != 0)
            return NULL;

        core_slot = slot;
    }

    return core_slot;
}

static gboolean
is_connected (SnapdPlug *plug, SnapdSlot *slot)
{
    GPtrArray *connections;
    guint i;

    connections = snapd_plug_get_connections (plug);
    for (i = 0; i < connections->len; i++) {
        SnapdConnection *c = g_ptr_array_index (connections, i);
        if (g_strcmp0 (snapd_connection_get_snap (c), snapd_slot_get_snap (slot)) == 0 &&
            g_strcmp0 (snapd_slot_get_name (slot), snapd_connection_get_name (c)) == 0)
            return TRUE;
    }

    return FALSE;
}

static void
snap_changed_cb (GtkWidget *combo)
{
    guint i, row = 0;
    SnapdSnap *snap;

    snap = snap_combo_box_get_snap (SNAP_COMBO_BOX (combo));

    gtk_container_foreach (GTK_CONTAINER (grid), (GtkCallback) gtk_widget_destroy, NULL);

    for (i = 0; i < plugs->len; i++) {
        SnapdPlug *plug = g_ptr_array_index (plugs, i);
        SnapdSlot *core_slot;
        const gchar *plug_label;
        GtkWidget *label;

        if (g_strcmp0 (snapd_plug_get_snap (plug), snapd_snap_get_name (snap)) != 0)
            continue;

        plug_label = snapd_plug_get_label (plug);
        if (plug_label == NULL || g_strcmp0 (plug_label, "") == 0)
            plug_label = snapd_plug_get_name (plug);
        label = gtk_label_new (plug_label);
        gtk_label_set_xalign (GTK_LABEL (label), 1.0);
        gtk_widget_set_hexpand (label, TRUE);
        gtk_widget_show (label);
        gtk_grid_attach (GTK_GRID (grid), label, 0, row, 1, 1);

        core_slot = get_core_slot (slots, plug);
        if (core_slot != NULL) {
            SnapInterfaceSwitch *sw;
            GPtrArray *connections;

            connections = snapd_plug_get_connections (plug);
            sw = snap_interface_switch_new (plug, core_slot, connections->len > 0);
            gtk_widget_show (GTK_WIDGET (sw));
            gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (sw), 1, row, 1, 1);
        }
        else {
            SnapInterfaceComboBox *combo;
            guint j;
            g_autoptr(GPtrArray) available_slots = NULL;
            SnapdSlot *connected_slot = NULL;

            available_slots = g_ptr_array_new_with_free_func (g_object_unref);
            for (j = 0; j < slots->len; j++) {
                SnapdSlot *slot = g_ptr_array_index (slots, j);

                if (g_strcmp0 (snapd_plug_get_interface (plug), snapd_slot_get_interface (slot)) != 0)
                    continue;

                g_ptr_array_add (available_slots, g_object_ref (slot));
                if (is_connected (plug, slot))
                    connected_slot = slot;
            }

            combo = snap_interface_combo_box_new (plug, available_slots);
            snap_interface_combo_box_set_connected_slot (combo, connected_slot);
            gtk_widget_show (GTK_WIDGET (combo));
            gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (combo), 1, row, 1, 1);
        }

        row++;
    }
}

int
main (int argc, char **argv)
{
    GtkWidget *window, *box;
    SnapComboBox *combo;
    g_autoptr(GPtrArray) snaps = NULL;
    g_autoptr(GPtrArray) configurable_snaps = NULL;
    SnapdSnap *selected_snap = NULL;
    g_autoptr(GError) error = NULL;

    gtk_init (&argc, &argv);

    client = snapd_client_new ();
    if (!snapd_client_connect_sync (client, NULL, &error)) {
        g_warning ("Failed to connect to snapd: %s\n", error->message);
        return 1;
    }

    if (!snapd_client_get_interfaces_sync (client, &plugs, &slots, NULL, &error)) {
        g_warning ("Failed to get interfaces: %s\n", error->message);
        return 1;
    }
    snaps = snapd_client_list_sync (client, NULL, &error);
    if (snaps == NULL) {
        g_warning ("Failed to get installed snaps: %s\n", error->message);
        return 1;
    }

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_container_set_border_width (GTK_CONTAINER (window), 20);
    gtk_widget_show (window);

    box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_show (box);
    gtk_container_add (GTK_CONTAINER (window), box);

    configurable_snaps = get_configurable_snaps (snaps, plugs);
    combo = snap_combo_box_new (configurable_snaps);
    g_signal_connect (combo, "changed", G_CALLBACK (snap_changed_cb), NULL);
    gtk_widget_show (GTK_WIDGET (combo));
    gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (combo), FALSE, TRUE, 0);

    grid = gtk_grid_new ();
    gtk_grid_set_row_spacing (GTK_GRID (grid), 10);
    gtk_grid_set_column_spacing (GTK_GRID (grid), 10);
    gtk_widget_show (grid);
    gtk_box_pack_start (GTK_BOX (box), grid, TRUE, TRUE, 0);

    if (argc > 1)
        selected_snap = find_snap (snaps, argv[1]);
    else if (configurable_snaps->len > 0)
        selected_snap = g_ptr_array_index (configurable_snaps, 0);
    snap_combo_box_set_snap (combo, selected_snap);

    gtk_main ();
    return 0;
}

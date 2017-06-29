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

#include "snap-interface-combo-box.h"

struct _SnapInterfaceComboBox
{
    GtkComboBox  parent_instance;

    SnapdPlug   *plug;
    GPtrArray   *slots;
    SnapdSlot   *connected_slot;
};

G_DEFINE_TYPE (SnapInterfaceComboBox, snap_interface_combo_box, GTK_TYPE_COMBO_BOX)

static void
snap_interface_combo_box_dispose (GObject *object)
{
    SnapInterfaceComboBox *combo = SNAP_INTERFACE_COMBO_BOX (object);

    g_clear_object (&combo->plug);
    g_clear_pointer (&combo->slots, g_ptr_array_unref);
    g_clear_object (&combo->connected_slot);

    G_OBJECT_CLASS (snap_interface_combo_box_parent_class)->dispose (object);
}

static void
snap_interface_combo_box_class_init (SnapInterfaceComboBoxClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = snap_interface_combo_box_dispose;
}

static void
snap_interface_combo_box_init (SnapInterfaceComboBox *combo)
{
}

SnapInterfaceComboBox *
snap_interface_combo_box_new (SnapdPlug *plug, GPtrArray *slots, SnapdSlot *connected_slot)
{
    SnapInterfaceComboBox *combo;
    GtkListStore *store;
    GtkCellRenderer *renderer;
    guint i;
    GtkTreeIter iter;

    combo = g_object_new (SNAP_TYPE_INTERFACE_COMBO_BOX, NULL);
    combo->plug = g_object_ref (plug);
    combo->slots = g_ptr_array_ref (slots);
    combo->connected_slot = g_object_ref (connected_slot);

    store = gtk_list_store_new (2, G_TYPE_STRING, SNAPD_TYPE_SLOT);
    gtk_combo_box_set_model (GTK_COMBO_BOX (combo), GTK_TREE_MODEL (store));

    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), renderer, TRUE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combo), renderer, "text", 0);

    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter, 0, "(disconnected)", 1, NULL, -1);
    if (connected_slot == NULL)
        gtk_combo_box_set_active_iter (GTK_COMBO_BOX (combo), &iter);
    for (i = 0; i < slots->len; i++) {
        SnapdSlot *slot = g_ptr_array_index (slots, i);
        const gchar *slot_label;
        g_autofree gchar *label = NULL;

        slot_label = snapd_slot_get_label (slot);
        if (slot_label == NULL || g_strcmp0 (slot_label, "") == 0)
            slot_label = snapd_slot_get_name (slot);
        label = g_strdup_printf ("%s:%s", snapd_slot_get_snap (slot), slot_label);

        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter, 0, label, 1, slot, -1);
        if (connected_slot == slot)
            gtk_combo_box_set_active_iter (GTK_COMBO_BOX (combo), &iter);
    }

    return combo;
}

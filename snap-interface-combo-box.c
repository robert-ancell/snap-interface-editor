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

void
snap_interface_combo_box_set_connected_slot (SnapInterfaceComboBox *combo, SnapdSlot *connected_slot)
{
    GtkTreeModel *model;
    GtkTreeIter iter;

    g_return_if_fail (SNAP_IS_INTERFACE_COMBO_BOX (combo));
    g_return_if_fail (connected_slot != NULL && SNAPD_IS_SLOT (connected_slot));  

    g_clear_object (&combo->connected_slot);
    combo->connected_slot = g_object_ref (connected_slot);

    model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo));
    if (gtk_tree_model_get_iter_first (model, &iter)) {
        do {
            SnapdSlot *slot;
            gtk_tree_model_get (gtk_combo_box_get_model (GTK_COMBO_BOX (combo)), &iter, 1, &slot, -1);
            if (slot == connected_slot) {
                gtk_combo_box_set_active_iter (GTK_COMBO_BOX (combo), &iter);
                break;
            }
        } while (gtk_tree_model_iter_next (model, &iter));
    }
}

SnapdSlot *
snap_interface_combo_box_get_connected_slot (SnapInterfaceComboBox *combo)
{
    g_return_val_if_fail (SNAP_IS_INTERFACE_COMBO_BOX (combo), NULL);
    return combo->connected_slot;
}

static void
changed_cb (SnapInterfaceComboBox *combo)
{
    GtkTreeIter iter;
    SnapdSlot *slot = NULL;

    if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo), &iter))
        gtk_tree_model_get (gtk_combo_box_get_model (GTK_COMBO_BOX (combo)), &iter, 1, &slot, -1);

    if (slot == combo->connected_slot)
        return;

    if (slot != NULL) {
        g_printerr ("connect %s:%s - %s:%s\n",
                    snapd_plug_get_snap (combo->plug), snapd_plug_get_name (combo->plug),
                    snapd_slot_get_snap (slot), snapd_slot_get_name (slot));
        g_clear_object (&combo->connected_slot);
        combo->connected_slot = g_object_ref (slot);
    }
    else {
        g_printerr ("disconnect %s:%s - %s:%s\n",
                    snapd_plug_get_snap (combo->plug), snapd_plug_get_name (combo->plug),
                    snapd_slot_get_snap (combo->connected_slot), snapd_slot_get_name (combo->connected_slot));
        g_clear_object (&combo->connected_slot);
    }
}

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
snap_interface_combo_box_new (SnapdPlug *plug, GPtrArray *slots)
{
    SnapInterfaceComboBox *combo;
    GtkListStore *store;
    GtkCellRenderer *renderer;
    guint i;
    GtkTreeIter iter;

    combo = g_object_new (SNAP_TYPE_INTERFACE_COMBO_BOX, NULL);
    combo->plug = g_object_ref (plug);
    combo->slots = g_ptr_array_ref (slots);

    store = gtk_list_store_new (2, G_TYPE_STRING, SNAPD_TYPE_SLOT);
    gtk_combo_box_set_model (GTK_COMBO_BOX (combo), GTK_TREE_MODEL (store));

    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), renderer, TRUE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combo), renderer, "text", 0);

    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter, 0, "(disconnected)", 1, NULL, -1);
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
    }

    g_signal_connect (combo, "changed", G_CALLBACK (changed_cb), NULL);

    return combo;
}

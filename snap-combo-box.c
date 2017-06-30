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

#include "snap-combo-box.h"

struct _SnapComboBox
{
    GtkComboBox  parent_instance;

    GPtrArray   *snaps;
};

G_DEFINE_TYPE (SnapComboBox, snap_combo_box, GTK_TYPE_COMBO_BOX)

void
snap_combo_box_set_snap (SnapComboBox *combo, SnapdSnap *snap)
{
    GtkTreeModel *model;
    GtkTreeIter iter;

    g_return_if_fail (SNAP_IS_COMBO_BOX (combo));

    model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo));
    if (gtk_tree_model_get_iter_first (model, &iter)) {
        do {
            SnapdSnap *s;
            gtk_tree_model_get (gtk_combo_box_get_model (GTK_COMBO_BOX (combo)), &iter, 1, &s, -1);
            if (s == snap) {
                gtk_combo_box_set_active_iter (GTK_COMBO_BOX (combo), &iter);
                return;
            }
        } while (gtk_tree_model_iter_next (model, &iter));
    }
}

SnapdSnap *
snap_combo_box_get_snap (SnapComboBox *combo)
{
    GtkTreeIter iter;
    SnapdSnap *snap;

    g_return_val_if_fail (SNAP_IS_COMBO_BOX (combo), NULL);

    if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo), &iter))
        return NULL;

    gtk_tree_model_get (gtk_combo_box_get_model (GTK_COMBO_BOX (combo)), &iter, 1, &snap, -1);
    return snap;
}

static void
snap_combo_box_dispose (GObject *object)
{
    SnapComboBox *combo = SNAP_COMBO_BOX (object);

    g_clear_pointer (&combo->snaps, g_ptr_array_unref);

    G_OBJECT_CLASS (snap_combo_box_parent_class)->dispose (object);
}

static void
snap_combo_box_class_init (SnapComboBoxClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = snap_combo_box_dispose;
}

static void
snap_combo_box_init (SnapComboBox *combo)
{
}

SnapComboBox *
snap_combo_box_new (GPtrArray *snaps)
{
    SnapComboBox *combo;
    GtkListStore *store;
    GtkCellRenderer *renderer;
    guint i;
    GtkTreeIter iter;

    combo = g_object_new (SNAP_TYPE_COMBO_BOX, NULL);
    combo->snaps = g_ptr_array_ref (snaps);

    store = gtk_list_store_new (2, G_TYPE_STRING, SNAPD_TYPE_SNAP);
    gtk_combo_box_set_model (GTK_COMBO_BOX (combo), GTK_TREE_MODEL (store));

    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), renderer, TRUE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combo), renderer, "text", 0);

    for (i = 0; i < snaps->len; i++) {
        SnapdSnap *snap = g_ptr_array_index (snaps, i);

        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter, 0, g_strdup (snapd_snap_get_name (snap)), 1, snap, -1);
    }

    return combo;
}

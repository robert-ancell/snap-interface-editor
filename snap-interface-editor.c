#include <gtk/gtk.h>
#include <snapd-glib/snapd-glib.h>

static SnapdClient *client = NULL;
static GPtrArray *slots = NULL;
static GPtrArray *plugs = NULL;

static GtkWidget *grid = NULL;

static gint
snap_index (GPtrArray *snaps, const gchar *value)
{
    gint i;

    for (i = 0; i < snaps->len; i++) {
        const gchar *snap = g_ptr_array_index (snaps, i);
        if (g_strcmp0 (snap, value) == 0)
            return i;
    }

    return -1;
}

static GPtrArray *
get_snaps (GPtrArray *plugs, GPtrArray *slots)
{
    GPtrArray *snaps;
    guint i;

    snaps = g_ptr_array_new_with_free_func (g_free);
    for (i = 0; i < slots->len; i++) {
        SnapdSlot *slot = g_ptr_array_index (slots, i);
        if (snap_index (snaps, snapd_slot_get_snap (slot)) == -1)
            g_ptr_array_add (snaps, g_strdup (snapd_slot_get_snap (slot)));
    }
    for (i = 0; i < plugs->len; i++) {
        SnapdPlug *plug = g_ptr_array_index (plugs, i);
        if (snap_index (snaps, snapd_plug_get_snap (plug)) == -1)
            g_ptr_array_add (snaps, g_strdup (snapd_plug_get_snap (plug)));
    }
    g_ptr_array_sort (snaps, (GCompareFunc) g_strcmp0);

    return snaps;
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

static void
snap_changed_cb (GtkWidget *combo)
{
    guint i, row = 0;
    const gchar *snap;

    snap = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (combo));

    gtk_container_foreach (GTK_CONTAINER (grid), (GtkCallback) gtk_widget_destroy, NULL);

    for (i = 0; i < plugs->len; i++) {
        SnapdPlug *plug = g_ptr_array_index (plugs, i);
        SnapdSlot *core_slot;
        const gchar *plug_label;
        GtkWidget *label;
        guint j;

        if (g_strcmp0 (snapd_plug_get_snap (plug), snap) != 0)
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
            GtkWidget *s;
            GPtrArray *connections;

            s = gtk_switch_new ();
            connections = snapd_plug_get_connections (plug);
            gtk_switch_set_active (GTK_SWITCH (s), connections->len);
            gtk_widget_show (s);
            gtk_grid_attach (GTK_GRID (grid), s, 1, row, 1, 1);
        }
        else {
            GtkWidget *combo;

            combo = gtk_combo_box_text_new ();
            gtk_widget_show (combo);
            gtk_grid_attach (GTK_GRID (grid), combo, 1, row, 1, 1);

            for (j = 0; j < slots->len; j++) {
                SnapdSlot *slot = g_ptr_array_index (slots, j);
                const gchar *slot_label;
                g_autofree gchar *label = NULL;

                if (g_strcmp0 (snapd_plug_get_interface (plug), snapd_slot_get_interface (slot)) != 0)
                    continue;

                slot_label = snapd_plug_get_label (plug);
                if (slot_label == NULL || g_strcmp0 (slot_label, "") == 0)
                    slot_label = snapd_plug_get_name (plug);
                label = g_strdup_printf ("%s:%s", snapd_slot_get_snap (slot), slot_label);
                gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo), label);
            }
            gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo), "(disconnected)");
        }

        row++;
    }
}

static void
set_active (GtkWidget *combo, const gchar *value)
{
    GtkTreeModel *model;
    GtkTreeIter iter;

    model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo));
    if (!gtk_tree_model_get_iter_first (model, &iter))
        return;
    do
    {
        const gchar *v;
        gtk_tree_model_get (model, &iter, 0, &v, -1);
        if (g_strcmp0 (v, value) == 0) {
            gtk_combo_box_set_active_iter (GTK_COMBO_BOX (combo), &iter);
            return;
        }
    } while (gtk_tree_model_iter_next (model, &iter));
}

int
main (int argc, char **argv)
{
    GtkWidget *window, *box, *combo;
    g_autoptr(GPtrArray) snaps = NULL;
    guint i;
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

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_container_set_border_width (GTK_CONTAINER (window), 20);
    gtk_widget_show (window);

    box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_show (box);
    gtk_container_add (GTK_CONTAINER (window), box);

    combo = gtk_combo_box_text_new ();
    snaps = get_snaps (plugs, slots);
    for (i = 0; i < snaps->len; i++) {
        const gchar *snap = g_ptr_array_index (snaps, i);
        gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo), snap);
    }
    g_signal_connect (combo, "changed", G_CALLBACK (snap_changed_cb), NULL);
    gtk_widget_show (combo);
    gtk_box_pack_start (GTK_BOX (box), combo, FALSE, TRUE, 0);

    grid = gtk_grid_new ();
    gtk_grid_set_row_spacing (GTK_GRID (grid), 10);
    gtk_grid_set_column_spacing (GTK_GRID (grid), 10);  
    gtk_widget_show (grid);
    gtk_box_pack_start (GTK_BOX (box), grid, TRUE, TRUE, 0);

    if (argc > 1)
        set_active (combo, argv[1]);
    else
        gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);

    gtk_main ();
    return 0;
}

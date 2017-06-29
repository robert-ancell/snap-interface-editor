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

#include "snap-interface-switch.h"

struct _SnapInterfaceSwitch
{
    GtkSwitch  parent_instance;

    SnapdPlug *plug;
    SnapdSlot *slot;
};

G_DEFINE_TYPE (SnapInterfaceSwitch, snap_interface_switch, GTK_TYPE_SWITCH)

static void
active_changed_cb (SnapInterfaceSwitch *sw)
{
    if (gtk_switch_get_active (GTK_SWITCH (sw)))
        g_printerr ("connect %s:%s - %s:%s\n",
                    snapd_plug_get_snap (sw->plug), snapd_plug_get_name (sw->plug),
                    snapd_slot_get_snap (sw->slot), snapd_slot_get_name (sw->slot));
    else
        g_printerr ("disconnect %s:%s - %s:%s\n",
                    snapd_plug_get_snap (sw->plug), snapd_plug_get_name (sw->plug),
                    snapd_slot_get_snap (sw->slot), snapd_slot_get_name (sw->slot));
}

static void
snap_interface_switch_dispose (GObject *object)
{
    SnapInterfaceSwitch *sw = SNAP_INTERFACE_SWITCH (object);

    g_clear_object (&sw->plug);
    g_clear_object (&sw->slot);

    G_OBJECT_CLASS (snap_interface_switch_parent_class)->dispose (object);
}

static void
snap_interface_switch_class_init (SnapInterfaceSwitchClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = snap_interface_switch_dispose;
}

static void
snap_interface_switch_init (SnapInterfaceSwitch *sw)
{
}

SnapInterfaceSwitch *
snap_interface_switch_new (SnapdPlug *plug, SnapdSlot *slot, gboolean connected)
{
    SnapInterfaceSwitch *sw;

    sw = g_object_new (SNAP_TYPE_INTERFACE_SWITCH, NULL);
    sw->plug = g_object_ref (plug);
    sw->slot = g_object_ref (slot);
    gtk_switch_set_active (GTK_SWITCH (sw), connected);
    g_signal_connect (sw, "notify::active", G_CALLBACK (active_changed_cb), NULL);

    return sw;
}

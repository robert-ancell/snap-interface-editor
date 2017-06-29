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

#ifndef __INTERFACE_SWITCH_H
#define __INTERFACE_SWITCH_H

#include <gtk/gtk.h>
#include <snapd-glib/snapd-glib.h>

G_BEGIN_DECLS

#define SNAP_TYPE_INTERFACE_SWITCH (snap_interface_switch_get_type ())

G_DECLARE_FINAL_TYPE (SnapInterfaceSwitch, snap_interface_switch, SNAP, INTERFACE_SWITCH, GtkSwitch)

SnapInterfaceSwitch *snap_interface_switch_new (SnapdPlug *plug, SnapdSlot *slot, gboolean connected);

G_END_DECLS

#endif /* __INTERFACE_SWITCH_H */

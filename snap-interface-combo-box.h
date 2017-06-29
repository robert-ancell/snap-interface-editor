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

#ifndef __SNAP_INTERFACE_COMBO_BOX_H
#define __SNAP_INTERFACE_COMBO_BOX_H

#include <gtk/gtk.h>
#include <snapd-glib/snapd-glib.h>

G_BEGIN_DECLS

#define SNAP_TYPE_INTERFACE_COMBO_BOX (snap_interface_combo_box_get_type ())

G_DECLARE_FINAL_TYPE (SnapInterfaceComboBox, snap_interface_combo_box, SNAP, INTERFACE_COMBO_BOX, GtkComboBox)

SnapInterfaceComboBox *snap_interface_combo_box_new (SnapdPlug *plug, GPtrArray *slots, SnapdSlot *connected_slot);

G_END_DECLS

#endif /* __SNAP_INTERFACE_COMBO_BOX_H */

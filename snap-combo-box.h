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

#ifndef __SNAP_COMBO_BOX_H
#define __SNAP_COMBO_BOX_H

#include <gtk/gtk.h>
#include <snapd-glib/snapd-glib.h>

G_BEGIN_DECLS

#define SNAP_TYPE_COMBO_BOX (snap_combo_box_get_type ())

G_DECLARE_FINAL_TYPE (SnapComboBox, snap_combo_box, SNAP, COMBO_BOX, GtkComboBox)

SnapComboBox *snap_combo_box_new      (GPtrArray    *snaps);

void          snap_combo_box_set_snap (SnapComboBox *combo,
                                       SnapdSnap    *snap);

SnapdSnap    *snap_combo_box_get_snap (SnapComboBox *combo);


G_END_DECLS

#endif /* __SNAP_COMBO_BOX_H */

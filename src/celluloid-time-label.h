/*
 * Copyright (c) 2020-2021 gnome-mpv
 *
 * This file is part of Celluloid.
 *
 * Celluloid is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Celluloid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Celluloid.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TIME_LABEL_H
#define TIME_LABEL_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CELLULOID_TYPE_TIME_LABEL (celluloid_time_label_get_type())

G_DECLARE_FINAL_TYPE(CelluloidTimeLabel, celluloid_time_label, CELLULOID, TIME_LABEL, GtkBox)

GtkWidget *
celluloid_time_label_new(void);

G_END_DECLS

#endif

/*
 * Copyright (c) 2015-2016 gnome-mpv
 *
 * This file is part of GNOME MPV.
 *
 * GNOME MPV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GNOME MPV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNOME MPV.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib/gi18n.h>

#include "gmpv_actionctl.h"
#include "gmpv_playlist_widget.h"
#include "gmpv_def.h"
#include "gmpv_mpv_obj.h"
#include "gmpv_playlist.h"
#include "gmpv_open_loc_dialog.h"
#include "gmpv_pref_dialog.h"
#include "gmpv_shortcuts_window.h"
#include "gmpv_common.h"
#include "authors.h"

static void open_handler(	GSimpleAction *action,
				GVariant *param,
				gpointer data );
static void open_loc_handler(	GSimpleAction *action,
				GVariant *param,
				gpointer data );
static void loop_handler(	GSimpleAction *action,
				GVariant *value,
				gpointer data );
static void playlist_toggle_handler(	GSimpleAction *action,
					GVariant *param,
					gpointer data );
static void playlist_save_handler(	GSimpleAction *action,
					GVariant *param,
					gpointer data );
static void pref_handler(	GSimpleAction *action,
				GVariant *param,
				gpointer data );
static void quit_handler(	GSimpleAction *action,
				GVariant *param,
				gpointer data );
static void track_select_handler(	GSimpleAction *action,
					GVariant *value,
					gpointer data );
static void load_track_handler(	GSimpleAction *action,
				GVariant *param,
				gpointer data );
static void fullscreen_handler(	GSimpleAction *action,
				GVariant *param,
				gpointer data );
static void video_size_handler(	GSimpleAction *action,
				GVariant *param,
				gpointer data );
static void about_handler(	GSimpleAction *action,
				GVariant *param,
				gpointer data );

static void open_handler(	GSimpleAction *action,
				GVariant *param,
				gpointer data )
{
	GmpvApplication *app = data;
	GmpvMainWindow *wnd = NULL;
	GSettings *main_config = NULL;
	GSettings *win_config = NULL;
	GtkFileChooser *file_chooser = NULL;
	GtkWidget *open_dialog = NULL;
	gboolean last_folder_enable = FALSE;
	gboolean append = FALSE;

	g_variant_get(param, "b", &append);

	wnd = gmpv_application_get_main_window(app);
	open_dialog =	gtk_file_chooser_dialog_new
			(	append?_("Add File to Playlist"):_("Open File"),
				GTK_WINDOW(wnd),
				GTK_FILE_CHOOSER_ACTION_OPEN,
				_("_Cancel"), GTK_RESPONSE_CANCEL,
				_("_Open"), GTK_RESPONSE_ACCEPT,
				NULL );
	main_config = g_settings_new(CONFIG_ROOT);
	file_chooser = GTK_FILE_CHOOSER(open_dialog);
	last_folder_enable =	g_settings_get_boolean
				(main_config, "last-folder-enable");

	if(last_folder_enable)
	{
		gchar *last_folder_uri;

		win_config = g_settings_new(CONFIG_WIN_STATE);
		last_folder_uri =	g_settings_get_string
					(win_config, "last-folder-uri");

		if(last_folder_uri && strlen(last_folder_uri) > 0)
		{
			gtk_file_chooser_set_current_folder_uri
				(file_chooser, last_folder_uri);
		}

		g_free(last_folder_uri);
	}

	gtk_file_chooser_set_select_multiple(file_chooser, TRUE);

	if(gtk_dialog_run(GTK_DIALOG(open_dialog)) == GTK_RESPONSE_ACCEPT)
	{
		GSList *uri_slist = gtk_file_chooser_get_filenames(file_chooser);
		GSList *uri = uri_slist;
		gsize uri_list_size =	sizeof(gchar **)*
					(g_slist_length(uri_slist)+1);
		const gchar **uri_list = g_malloc(uri_list_size);
		gint i;

		for(i = 0; uri; i++)
		{
			uri_list[i] = uri->data;
			uri = g_slist_next(uri);
		}

		uri_list[i] = NULL;

		if(uri_slist)
		{
			GmpvMpvObj *mpv = gmpv_application_get_mpv_obj(app);

			gmpv_mpv_obj_load_list(mpv, uri_list, append, TRUE);
		}

		if(last_folder_enable)
		{
			gchar *last_folder_uri
				= gtk_file_chooser_get_current_folder_uri
					(file_chooser);

			g_settings_set_string(	win_config,
						"last-folder-uri",
						last_folder_uri?:"" );

			g_free(last_folder_uri);
		}

		g_free(uri_list);
		g_slist_free_full(uri_slist, g_free);
	}

	gtk_widget_destroy(open_dialog);
	g_clear_object(&main_config);
	g_clear_object(&win_config);
}

static void open_loc_handler(	GSimpleAction *action,
				GVariant *param,
				gpointer data )
{
	GmpvApplication *app = data;
	GmpvMainWindow *wnd = gmpv_application_get_main_window(app);
	GtkWidget *dlg = gmpv_open_loc_dialog_new(GTK_WINDOW(wnd));

	if(gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT)
	{
		const gchar *loc_str;
		GmpvMpvObj *mpv = gmpv_application_get_mpv_obj(app);

		loc_str =	gmpv_open_loc_dialog_get_string
				(GMPV_OPEN_LOC_DIALOG(dlg));

		gmpv_mpv_obj_set_property_flag(mpv, "pause", FALSE);
		gmpv_mpv_obj_load(mpv, loc_str, FALSE, TRUE);
	}

	gtk_widget_destroy(dlg);
}

static void loop_handler(	GSimpleAction *action,
				GVariant *value,
				gpointer data )
{
	GmpvApplication *app = data;
	GmpvMpvObj *mpv = gmpv_application_get_mpv_obj(app);
	gboolean loop = g_variant_get_boolean(value);

	g_simple_action_set_state(action, value);

	mpv_check_error(gmpv_mpv_obj_set_property_string(	mpv,
							"loop",
							loop?"inf":"no" ));
}

static void show_shortcuts_handler(	GSimpleAction *action,
					GVariant *param,
					gpointer data )
{
	GmpvApplication *app = data;
	GmpvMainWindow *mwnd = gmpv_application_get_main_window(app);
	GtkWidget *wnd = gmpv_shortcuts_window_new(GTK_WINDOW(mwnd));

	gtk_widget_show_all(wnd);
}

static void playlist_toggle_handler(	GSimpleAction *action,
					GVariant *param,
					gpointer data )
{
	GmpvApplication *app = data;
	GmpvMainWindow *wnd = gmpv_application_get_main_window(app);
	gboolean visible = gmpv_main_window_get_playlist_visible(wnd);

	gmpv_main_window_set_playlist_visible(wnd, !visible);
}

static void playlist_save_handler(	GSimpleAction *action,
					GVariant *param,
					gpointer data )
{
	GmpvApplication *app = data;
	GmpvMpvObj *mpv;
	GmpvMainWindow *wnd;
	GmpvPlaylist *playlist;
	GtkTreeModel *model;
	GFile *dest_file;
	GOutputStream *dest_stream;
	GtkFileChooser *file_chooser;
	GtkWidget *save_dialog;
	GError *error;
	GtkTreeIter iter;
	gboolean rc;

	mpv = gmpv_application_get_mpv_obj(app);
	wnd = gmpv_application_get_main_window(app);
	playlist = gmpv_mpv_obj_get_playlist(mpv);
	model = GTK_TREE_MODEL(gmpv_playlist_get_store(playlist));
	dest_file = NULL;
	dest_stream = NULL;
	save_dialog =	gtk_file_chooser_dialog_new
			(	_("Save Playlist"),
				GTK_WINDOW(wnd),
				GTK_FILE_CHOOSER_ACTION_SAVE,
				_("_Cancel"), GTK_RESPONSE_CANCEL,
				_("_Save"), GTK_RESPONSE_ACCEPT,
				NULL );
	file_chooser = GTK_FILE_CHOOSER(save_dialog);
	error = NULL;
	rc = FALSE;

	gtk_file_chooser_set_do_overwrite_confirmation(file_chooser, TRUE);
	gtk_file_chooser_set_current_name(file_chooser, "playlist.m3u");

	if(gtk_dialog_run(GTK_DIALOG(save_dialog)) == GTK_RESPONSE_ACCEPT)
	{
		/* There should be only one file selected. */
		dest_file = gtk_file_chooser_get_file(file_chooser);
	}

	gtk_widget_destroy(save_dialog);

	if(dest_file)
	{
		GFileOutputStream *dest_file_stream;

		dest_file_stream = g_file_replace(	dest_file,
							NULL,
							FALSE,
							G_FILE_CREATE_NONE,
							NULL,
							&error );

		dest_stream = G_OUTPUT_STREAM(dest_file_stream);
		rc = gtk_tree_model_get_iter_first(model, &iter);
		rc &= !!dest_stream;
	}

	while(rc)
	{
		gchar *uri;
		gsize written;

		gtk_tree_model_get
			(model, &iter, PLAYLIST_URI_COLUMN, &uri, -1);

		rc &= g_output_stream_printf(	dest_stream,
						&written,
						NULL,
						&error,
						"%s\n",
						uri );
		rc &= gtk_tree_model_iter_next(model, &iter);
	}

	if(dest_stream)
	{
		g_output_stream_close(dest_stream, NULL, &error);
	}

	if(dest_file)
	{
		g_object_unref(dest_file);
	}

	if(error)
	{
		show_error_dialog(app, NULL, error->message);

		g_error_free(error);
	}
}

static void playlist_remove_selected(	GSimpleAction *action,
					GVariant *param,
					gpointer data )
{
	GmpvMainWindow *wnd =	gmpv_application_get_main_window
				(GMPV_APPLICATION(data));

	if(gmpv_main_window_get_playlist_visible(wnd))
	{
		GmpvPlaylistWidget *playlist;

		playlist = gmpv_main_window_get_playlist(wnd);

		gmpv_playlist_widget_remove_selected(playlist);
	}
}

static void pref_handler(	GSimpleAction *action,
				GVariant *param,
				gpointer data )
{
	GmpvApplication *app = data;
	GmpvMainWindow *wnd = gmpv_application_get_main_window(app);
	GSettings *settings = g_settings_new(CONFIG_ROOT);
	GmpvPrefDialog *pref_dialog;
	gboolean csd_enable_old;

	csd_enable_old = g_settings_get_boolean(settings, "csd-enable");
	pref_dialog = GMPV_PREF_DIALOG(gmpv_pref_dialog_new(GTK_WINDOW(wnd)));

	if(gtk_dialog_run(GTK_DIALOG(pref_dialog)) == GTK_RESPONSE_ACCEPT)
	{
		GmpvMpvObj *mpv;
		gboolean csd_enable;
		gboolean dark_theme_enable;

		mpv = gmpv_application_get_mpv_obj(app);
		csd_enable = g_settings_get_boolean(settings, "csd-enable");
		dark_theme_enable =	g_settings_get_boolean
					(settings, "dark-theme-enable");

		if(csd_enable_old != csd_enable)
		{
			GtkWidget *dialog;

			dialog =	gtk_message_dialog_new
					(	GTK_WINDOW(wnd),
						GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_INFO,
						GTK_BUTTONS_OK,
						_("Enabling or disabling "
						"client-side decorations "
						"requires restarting %s to "
						"take effect." ),
						g_get_application_name() );

			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
		}

		g_object_set(	gtk_settings_get_default(),
				"gtk-application-prefer-dark-theme",
				dark_theme_enable,
				NULL );

		gmpv_mpv_obj_reset(mpv);
		gtk_widget_queue_draw(GTK_WIDGET(wnd));
	}

	gtk_widget_destroy(GTK_WIDGET(pref_dialog));
}

static void quit_handler(	GSimpleAction *action,
				GVariant *param,
				gpointer data )
{
	quit(data);
}

static void track_select_handler(	GSimpleAction *action,
					GVariant *value,
					gpointer data )
{
	GmpvApplication *app = data;
	GmpvMpvObj *mpv;
	gint64 id;
	gchar *name;
	const gchar *mpv_prop;

	g_object_get(action, "name", &name, NULL);
	g_variant_get(value, "x", &id);
	g_simple_action_set_state(action, value);

	if(g_strcmp0(name, "audio_select") == 0)
	{
		mpv_prop = "aid";
	}
	else if(g_strcmp0(name, "video_select") == 0)
	{
		mpv_prop = "vid";
	}
	else if(g_strcmp0(name, "sub_select") == 0)
	{
		mpv_prop = "sid";
	}
	else
	{
		g_assert_not_reached();
	}

	mpv = gmpv_application_get_mpv_obj(app);

	if(id >= 0)
	{
		gmpv_mpv_obj_set_property(mpv, mpv_prop, MPV_FORMAT_INT64, &id);
	}
	else
	{
		gmpv_mpv_obj_set_property_string(mpv, mpv_prop, "no");
	}
}

static void load_track_handler(	GSimpleAction *action,
				GVariant *param,
				gpointer data )
{
	GmpvApplication *app = data;
	GmpvMainWindow *wnd;
	GtkFileChooser *file_chooser;
	GtkWidget *open_dialog;
	const gchar *cmd_name;

	g_variant_get(param, "s", &cmd_name);

	wnd = gmpv_application_get_main_window(app);
	open_dialog =	gtk_file_chooser_dialog_new
			(	_("Load External…"),
				GTK_WINDOW(wnd),
				GTK_FILE_CHOOSER_ACTION_OPEN,
				_("_Cancel"), GTK_RESPONSE_CANCEL,
				_("_Open"), GTK_RESPONSE_ACCEPT,
				NULL );
	file_chooser = GTK_FILE_CHOOSER(open_dialog);

	gtk_file_chooser_set_select_multiple(file_chooser, TRUE);

	if(gtk_dialog_run(GTK_DIALOG(open_dialog)) == GTK_RESPONSE_ACCEPT)
	{
		const gchar *cmd[] = {cmd_name, NULL, NULL};
		GmpvMpvObj *mpv = gmpv_application_get_mpv_obj(app);
		GSList *uri_list = gtk_file_chooser_get_filenames(file_chooser);
		GSList *uri = uri_list;

		while(uri)
		{
			cmd[1] = uri->data;

			gmpv_mpv_obj_command(mpv, cmd);

			uri = g_slist_next(uri);
		}

		g_slist_free_full(uri_list, g_free);
	}

	gtk_widget_destroy(open_dialog);
}

static void fullscreen_handler(	GSimpleAction *action,
				GVariant *param,
				gpointer data )
{
	GmpvMainWindow *wnd =	gmpv_application_get_main_window
				(GMPV_APPLICATION(data));
	gchar *name;

	g_object_get(action, "name", &name, NULL);

	if(g_strcmp0(name, "fullscreen_toggle") == 0)
	{
		gmpv_main_window_toggle_fullscreen(wnd);
	}
	else if(g_strcmp0(name, "fullscreen_enter") == 0)
	{
		gmpv_main_window_set_fullscreen(wnd, TRUE);
	}
	else if(g_strcmp0(name, "fullscreen_leave") == 0)
	{
		gmpv_main_window_set_fullscreen(wnd, FALSE);
	}

	g_free(name);
}

static void video_size_handler(	GSimpleAction *action,
				GVariant *param,
				gpointer data )
{
	gdouble value = g_variant_get_double (param);

	resize_window_to_fit((GmpvApplication *)data, value);
}

static void about_handler(	GSimpleAction *action,
				GVariant *param,
				gpointer data )
{
	GmpvApplication *app = data;
	GmpvMainWindow *wnd = gmpv_application_get_main_window(app);
	const gchar *authors[] = AUTHORS;

	gtk_show_about_dialog(	GTK_WINDOW(wnd),
				"logo-icon-name",
				ICON_NAME,
				"version",
				VERSION,
				"comments",
				_("A GTK frontend for MPV"),
				"website",
				"https://github.com/gnome-mpv/gnome-mpv",
				"license-type",
				GTK_LICENSE_GPL_3_0,
				"copyright",
				"\u00A9 2014-2016 The GNOME MPV authors",
				"authors",
				authors,
				"translator-credits",
				_("translator-credits"),
				NULL );
}

void gmpv_actionctl_map_actions(GmpvApplication *app)
{
	const GActionEntry entries[]
	= {	{.name = "open",
		.activate = open_handler,
		.parameter_type = "b"},
		{.name = "quit",
		.activate = quit_handler},
		{.name = "about",
		.activate = about_handler},
		{.name = "pref",
		.activate = pref_handler},
		{.name = "openloc",
		.activate = open_loc_handler},
		{.name = "loop",
		.state = "false",
		.change_state = loop_handler},
		{.name = "show_shortcuts",
		.activate = show_shortcuts_handler},
		{.name = "playlist_toggle",
		.activate = playlist_toggle_handler},
		{.name = "playlist_save",
		.activate = playlist_save_handler},
		{.name = "playlist_remove_selected",
		.activate = playlist_remove_selected},
		{.name = "audio_select",
		.change_state = track_select_handler,
		.state = "@x 1",
		.parameter_type = "x"},
		{.name = "video_select",
		.change_state = track_select_handler,
		.state = "@x 1",
		.parameter_type = "x"},
		{.name = "sub_select",
		.change_state = track_select_handler,
		.state = "@x 1",
		.parameter_type = "x"},
		{.name = "load_track",
		.activate = load_track_handler,
		.parameter_type = "s"},
		{.name = "fullscreen_toggle",
		.activate = fullscreen_handler},
		{.name = "fullscreen_enter",
		.activate = fullscreen_handler},
		{.name = "fullscreen_leave",
		.activate = fullscreen_handler},
		{.name = "video_size",
		.activate = video_size_handler,
		.parameter_type = "d"} };

	g_action_map_add_action_entries(	G_ACTION_MAP(app),
						entries,
						G_N_ELEMENTS(entries),
						app );
}
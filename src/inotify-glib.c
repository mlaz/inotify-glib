/*
 * inotify-lib.c - backend for really simple inotify example
 *
 * Robert Love <rml@novell.com>
 */

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <glib.h>
#include <sys/inotify.h>

#include "inotify-glib.h"

/*
 * inotify_open - open the inotify device and return a fresh GIOChannel
 * or NULL on error.
 */
GIOChannel *
inf_open (void)
{
  int fd;
  GIOChannel *gio;
  GError *err = NULL;

  fd = inotify_init ();

  if (fd < 0)
    {
      perror ("inotify_init");
    }

  gio = g_io_channel_unix_new (fd);

  if (!gio) {
    g_warning ("Couldn't open GIOChannel %s\n", err->message);
    g_error_free (err);
  }

  return gio;
}

/*
 * inotify_close - close the GIOChannel
 */
void
inf_close (GIOChannel *gio)
{
  GError *err = NULL;
  GIOStatus status = g_io_channel_shutdown (gio, TRUE, &err);
  if (status == G_IO_STATUS_ERROR)
    {
      g_warning ("Couldn't close GIOChannel %s\n", err->message);
      g_error_free (err);
    }
}

/*
 * inotify_add_watch - Add an inotify watch on the object "name" to the
 * open inotify instance associated with "gio".  The user may do this any
 * number of times, even on the same device instance.
 */
int
inf_add_watch (GIOChannel *gio, const char *name)
{
  int in_fd, wd, ret;

  in_fd = g_io_channel_unix_get_fd (gio);
  wd = inotify_add_watch (in_fd, name, IN_ALL_EVENTS);
  if (wd < 0) {
    perror ("inotify_add_watch error!\n");
    return -1;
  }

  //should we do this here?
  /* if (close (wd)) */
  /*   perror ("close wd error!\n"); */

  return ret;
}

/* inotify lets us slurp a lot of events at once.  we go with a nice big 32k */
#define INOTIFY_BUF	32768

/*
 * __inotify_handle_cb - our internal GIOChannel G_IO_IN callback.  Slurps as
 * many events as are available and calls the user's callback (given as "data")
 * for each event.  If any invocations of the user's callback return FALSE, so
 * do we, terminating this watch.  Otherwise, we return TRUE.
 */
static gboolean
__inotify_handle_cb (GIOChannel *gio, GIOCondition condition, gpointer data)
{
  char buf[INOTIFY_BUF];
  InotifyCb f = data;
  GError* err = NULL;
  gsize len;
  int i = 0;

  /* read in as many pending events as we can */
  g_io_channel_read_chars (gio, buf, INOTIFY_BUF, &len, &err);
  if (err != NULL) {
    fprintf (stderr, "Error reading the inotify fd: %d\n", err->message);
    // g_warning ("Error reading the inotify fd: %d\n", err);
    return FALSE;
  }

  /* reconstruct each event and send to the user's callback */
  while (i < len) {
    const char *name = "The watch";
    struct inotify_event *event;

    event = (struct inotify_event *) &buf[i];
    if (event->len)
      name = &buf[i] + sizeof (struct inotify_event);

    if (f (name, event->wd, event->mask, event->cookie) == FALSE)
      return FALSE;

    i += sizeof (struct inotify_event) + event->len;
  }

  return TRUE;
}

/*
 * inotify_callback - associate a user InotifyCb callback with the given
 * GIOChannel.  This is normally done but once.
 */
void
inf_callback (GIOChannel *gio, InotifyCb f)
{
  g_io_add_watch (gio, G_IO_IN, __inotify_handle_cb, f);
}

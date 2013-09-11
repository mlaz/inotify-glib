/*
 * inotify.c - frontend for really simple inotify example
 *
 * Based on https://www.kernel.org/pub/linux/kernel/people/rml/inotify/glib/
 * By Robert Love <rml@novell.com>
 * Changes by Miguel Azevedo <migullazev@gmail.com>
 */

#include <glib.h>
#include <sys/inotify.h>
#include "inotify-glib.h"

/*
 * my_cb - our callback.  Events are relative to the watch, which is given by
 * "wd".  This simple example ignores it, for the most part, but realworld uses
 * probably want to map the watch descriptor back to the object they are
 * watching.  "event" describes the event (see inotify.h) and "cookie" is used
 * to synchronize two separate IN_MOVED_{TO,FROM} events (not done here).
 */
static gboolean
my_cb (const char *name, int wd, unsigned int event, unsigned int cookie)
{
  const char *type = "file";
  //GFileTest test;

  if (event & IN_ISDIR)
    type = "dir";

  if (event & IN_ACCESS)
    g_print ("wd=%d: %s (%s) was read\n", wd, name, type);
  if (event & IN_MODIFY)
    g_print ("wd=%d: %s (%s) was written\n", wd, name, type);
  if (event & IN_ATTRIB)
    g_print ("wd=%d: %s's (%s) metadata changed\n", wd, name, type);
  if (event & IN_CLOSE_WRITE)
    g_print ("wd=%d: %s (%s) was closed (was writable)\n",
             wd, name, type);
  if (event & IN_CLOSE_NOWRITE)
    g_print ("wd=%d: %s (%s) was closed (was not writable)\n",
             wd, name, type);
  if (event & IN_OPEN)
    g_print ("wd=%d: %s (%s) was opened\n", wd, name, type);
  if (event & IN_MOVED_FROM)
    g_print ("wd=%d: %s (%s) was moved away\n", wd, name, type);
  if (event & IN_MOVED_TO)
    g_print ("wd=%d: %s (%s) was moved here\n", wd, name, type);
  if (event & IN_DELETE)
    g_print ("wd=%d: %s (%s) was deleted\n", wd, name, type);
  if (event & IN_CREATE)
    g_print ("wd=%d: %s (%s) was created\n", wd, name, type);
  if (event & IN_DELETE_SELF)
    {
      g_print ("wd=%d: The watch was deleted!", wd);
      /* check if the file stll exists, if yes restart the watch. */
      /* g_file_test(wname ,test); */
      /* if (test == G_FILE_TEST_EXISTS) */
      /* { */
      /*   g_print ("wd=%d: Restarting watch\n", wd); */
      /* } */
      /* else */
      /*   g_print ("wd=%d: The watch was deleted!", wd); */
    }
  if (event & IN_UNMOUNT)
    g_print ("wd=%d: %s was unmounted\n", wd, name);
  if (event & IN_Q_OVERFLOW)
    g_print ("The queue overflowed!");
  if (event & IN_IGNORED)
    g_print ("wd=%d: %s is no longer watched\n", wd, name);

  return TRUE;
}

int
main (int argc, char *argv[])
{
  GMainLoop *mainloop;
  GIOChannel *gio;
  int i;

  if (argc < 2)
    {
      g_warning ("Usage: %s [directories or files ...]\n", argv[0]);
      return -1;
    }

  gio = inf_open ();
  if (!gio)
    return -1;

  for (i = 1; i < argc; i++)
    {
      int wd;

      g_print ("Adding watch on %s, wd=%d\n", argv[i], wd);
      wd = inf_add_watch (gio, argv[i], IN_ALL_EVENTS);
    }

  inf_callback (gio, my_cb);

  mainloop = g_main_new (FALSE);
  g_main_run (mainloop);

  inf_close (gio);

  return 0;
}

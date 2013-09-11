#ifndef INOTIFY_GLIB_H
#define INOTIFY_GLIB_H

typedef gboolean (*InotifyCb) (const char *, int, unsigned int, unsigned int);

GIOChannel * inf_open (void);
void inf_close (GIOChannel *);
int inf_add_watch (GIOChannel *, const char *, uint32_t mask);
void inf_callback (GIOChannel *, InotifyCb);

#endif	/* INOTIFY_GLIB_H */

// 6 april 2015
#include "uipriv_unix.h"

uiInitOptions options;

const char *uiInit(uiInitOptions *o)
{
	GError *err = NULL;
	const char *msg;

	options = *o;
	if (gtk_init_with_args(NULL, NULL, NULL, NULL, NULL, &err) == FALSE) {
		msg = g_strdup(err->message);
		g_error_free(err);
		return msg;
	}
	initAlloc();
	return NULL;
}

void uiUninit(void)
{
	uninitMenus();
	uninitAlloc();
}

void uiFreeInitError(const char *err)
{
	g_free((gpointer) err);
}

static gboolean (*iteration)(gboolean) = NULL;

void uiMain(void)
{
	iteration = gtk_main_iteration_do;
	gtk_main();
}

static gboolean stepsQuit = FALSE;

// the only difference is we ignore the return value from gtk_main_iteration_do(), since it will always be TRUE if gtk_main() was never called
// gtk_main_iteration_do() will still run the main loop regardless
static gboolean stepsIteration(gboolean block)
{
	gtk_main_iteration_do(block);
	return stepsQuit;
}

void uiMainSteps(void)
{
	iteration = stepsIteration;
}

int uiMainStep(int wait)
{
	gboolean block;

	block = FALSE;
	if (wait)
		block = TRUE;
	return (*iteration)(block) == FALSE;
}

// gtk_main_quit() may run immediately, or it may wait for other pending events; "it depends" (thanks mclasen in irc.gimp.net/#gtk+)
// PostQuitMessage() on Windows always waits, so we must do so too
// we'll do it by using an idle callback
static gboolean quit(gpointer data)
{
	if (iteration == stepsIteration)
		stepsQuit = TRUE;
		// TODO run a gtk_main() here just to do the cleanup steps of syncing the clipboard and other stuff gtk_main() does before it returns
	else
		gtk_main_quit();
	return FALSE;
}

void uiQuit(void)
{
	gdk_threads_add_idle(quit, NULL);
}

struct _timeout
{
	int (*f)(void *);
	void *data;
};

static gboolean dotimeout(gpointer data)
{
	struct _timeout *t = (struct _timeout *) data;

	if ((*(t->f))(t->data))
	{
		return TRUE;
	}

	uiFree(t);
	return FALSE;
}

void uiTimeout(int millisec, int (*f)(void *data), void *data)
{
	struct _timeout *t = uiNew(struct _timeout);
	t->f = f;
	t->data = data;
	g_timeout_add(millisec, dotimeout, t);
}

struct queued {
	void (*f)(void *);
	void *data;
};

static gboolean doqueued(gpointer data)
{
	struct queued *q = (struct queued *) data;

	(*(q->f))(q->data);
	uiFree(q);
	return FALSE;
}

void uiQueueMain(void (*f)(void *data), void *data)
{
	struct queued *q;

	q = uiNew(struct queued);
	q->f = f;
	q->data = data;
	gdk_threads_add_idle(doqueued, q);
}

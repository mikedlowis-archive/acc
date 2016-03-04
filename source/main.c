/*
    @brief Main file for ACC
    @author Michael D. Lowis
    @license BSD 2-Clause License
*/
#include "autil.h"
#include "ini.h"
#include "strophe.h"

char* ARGV0  = NULL;
char* User   = NULL;
char* Pass   = NULL;
char* Server = NULL;
int   Port   = 5222;
long  Flags  = 0;

static void usage(void);
static void load_config(void);
void conn_handler(xmpp_conn_t * const conn, const xmpp_conn_event_t status,
                  const int error, xmpp_stream_error_t * const stream_error,
                  void * const userdata);
int handle_reply(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata);

int main(int argc, char** argv)
{
    /* Parse command-line options */
    OPTBEGIN {
        default:
            usage();
    } OPTEND;
    /* Parse config file if it exists */
    load_config();
    /* Start the connection */
    xmpp_initialize();
    xmpp_log_t*  log  = xmpp_get_default_logger(XMPP_LEVEL_DEBUG);
    xmpp_ctx_t*  ctx  = xmpp_ctx_new(NULL, log);
    xmpp_conn_t* conn = xmpp_conn_new(ctx);
    /* setup authentication information and connect */
    xmpp_conn_set_flags(conn, Flags);
    xmpp_conn_set_jid(conn,  User);
    xmpp_conn_set_pass(conn, Pass);
    xmpp_connect_client(conn, Server, Port, conn_handler, ctx);
    /* enter the event loop our connect handler will trigger an exit */
    xmpp_run(ctx);
    /* gracefully shut everything down */
    xmpp_conn_release(conn);
    xmpp_ctx_free(ctx);
    xmpp_shutdown();
    return 0;
}

static void usage(void)
{
    fprintf(stderr, "Usage: %s [options] PROFILE\n", ARGV0);
    exit(1);
}

static void load_config(void)
{
    inientry_t entry  = { 0 };
    inifile_t cfgfile = { .file = fopen("./config.ini","r") };
    while (iniparse(&cfgfile, &entry)) {
        if (inimatch("user",&entry))
            User = estrdup(entry.value);
        else if (inimatch("pass",&entry))
            Pass = estrdup(entry.value);
        else if (inimatch("port",&entry))
            Port = strtoul(entry.value,NULL,0);
        else if (inimatch("server",&entry))
            Server = estrdup(entry.value);
    }
}

void conn_handler(xmpp_conn_t * const conn, const xmpp_conn_event_t status,
                  const int error, xmpp_stream_error_t * const stream_error,
                  void * const userdata)
{
    xmpp_ctx_t *ctx = (xmpp_ctx_t *)userdata;
    xmpp_stanza_t *iq, *query;
    int secured;

    if (status == XMPP_CONN_CONNECT) {
        fprintf(stderr, "DEBUG: connected\n");
        secured = xmpp_conn_is_secured(conn);
        fprintf(stderr, "DEBUG: connection is %s.\n",
                secured ? "secured" : "NOT secured");

        /* create iq stanza for request */
        iq = xmpp_stanza_new(ctx);
        xmpp_stanza_set_name(iq, "iq");
        xmpp_stanza_set_type(iq, "get");
        xmpp_stanza_set_id(iq, "roster1");

        query = xmpp_stanza_new(ctx);
        xmpp_stanza_set_name(query, "query");
        xmpp_stanza_set_ns(query, XMPP_NS_ROSTER);

        xmpp_stanza_add_child(iq, query);

        /* we can release the stanza since it belongs to iq now */
        xmpp_stanza_release(query);

        /* set up reply handler */
        xmpp_id_handler_add(conn, handle_reply, "roster1", ctx);

        /* send out the stanza */
        xmpp_send(conn, iq);

        /* release the stanza */
        xmpp_stanza_release(iq);
    } else {
        fprintf(stderr, "DEBUG: disconnected\n");
        xmpp_stop(ctx);
    }
}

int handle_reply(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata)
{
    xmpp_stanza_t *query, *item;
    char *type, *name;

    type = xmpp_stanza_get_type(stanza);
    if (strcmp(type, "error") == 0) {
        fprintf(stderr, "ERROR: query failed\n");
    } else {
        query = xmpp_stanza_get_child_by_name(stanza, "query");
        printf("Roster:\n");
        for (item = xmpp_stanza_get_children(query); item; item = xmpp_stanza_get_next(item))
            if ((name = xmpp_stanza_get_attribute(item, "name")))
                printf("\t %s (%s) sub=%s\n",
                       name,
                       xmpp_stanza_get_attribute(item, "jid"),
                       xmpp_stanza_get_attribute(item, "subscription"));
            else
                printf("\t %s sub=%s\n",
                       xmpp_stanza_get_attribute(item, "jid"),
                       xmpp_stanza_get_attribute(item, "subscription"));
        printf("END OF LIST\n");
    }

    /* disconnect */
    xmpp_disconnect(conn);

    return 0;
}

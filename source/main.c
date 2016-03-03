/*
    @brief Main file for ACC
    @author Michael D. Lowis
    @license BSD 2-Clause License
*/
#include "autil.h"
#include "ini.h"
#include "strophe.h"

char* ARGV0     = NULL;
char* User      = NULL;
char* Pass      = NULL;
char* Resource  = NULL;
char* Alias     = NULL;
char* Server    = NULL;
char* FileProxy = NULL;
int   Port      = 5222;
long  Flags     = 0;

static void usage(void);
static void load_config(void);
void conn_handler(xmpp_conn_t * const conn, const xmpp_conn_event_t status,
                  const int error, xmpp_stream_error_t * const stream_error,
                  void * const userdata);

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
    xmpp_connect_client(conn, Server, 0, conn_handler, ctx);
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
        //printf("[%s] '%s' = '%s'\n", entry.section, entry.name, entry.value);
        if (inimatch("user",&entry))
            User = estrdup(entry.value);
        else if (inimatch("pass",&entry))
            Pass = estrdup(entry.value);
        else if (inimatch("resource",&entry))
            Resource = estrdup(entry.value);
        else if (inimatch("alias",&entry))
            Alias = estrdup(entry.value);
        else if (inimatch("port",&entry))
            Port = strtoul(entry.value,NULL,0);
        else if (inimatch("server",&entry))
            Server = estrdup(entry.value);
        else if (inimatch("fileproxy",&entry))
            FileProxy = estrdup(entry.value);
    }
}

void conn_handler(xmpp_conn_t * const conn, const xmpp_conn_event_t status,
                  const int error, xmpp_stream_error_t * const stream_error,
                  void * const userdata)
{
    xmpp_ctx_t *ctx = (xmpp_ctx_t *)userdata;
    int secured;

    if (status == XMPP_CONN_CONNECT) {
        fprintf(stderr, "DEBUG: connected\n");
        secured = xmpp_conn_is_secured(conn);
        fprintf(stderr, "DEBUG: connection is %s.\n",
                secured ? "secured" : "NOT secured");
        xmpp_disconnect(conn);
    }
    else {
        fprintf(stderr, "DEBUG: disconnected\n");
        xmpp_stop(ctx);
    }
}


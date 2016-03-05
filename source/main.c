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

xmpp_ctx_t*  Ctx;
xmpp_conn_t* Conn;

static void usage(void);
static void load_config(void);
void onconnect(xmpp_conn_t * const conn, const xmpp_conn_event_t status,
               const int error, xmpp_stream_error_t * const stream_error,
               void * const userdata);
int onreceive(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata);
void sendmsg(char* recipient, char* message);
//int handle_roster(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata);

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
    Ctx  = xmpp_ctx_new(NULL, log);
    Conn = xmpp_conn_new(Ctx);
    /* setup authentication information and connect */
    xmpp_conn_set_flags(Conn, Flags);
    xmpp_conn_set_jid(Conn,  User);
    xmpp_conn_set_pass(Conn, Pass);
    xmpp_connect_client(Conn, Server, Port, onconnect, Ctx);
    /* enter the event loop our connect handler will trigger an exit */
    xmpp_run(Ctx);
    /* gracefully shut everything down */
    xmpp_conn_release(Conn);
    xmpp_ctx_free(Ctx);
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

void onconnect(xmpp_conn_t * const conn, const xmpp_conn_event_t status,
               const int error, xmpp_stream_error_t * const stream_error,
               void * const userdata)
{
    if (status == XMPP_CONN_CONNECT) {
        xmpp_handler_add(Conn, onreceive, NULL, "message", NULL, Ctx);
        /* Anounce presence */
        xmpp_stanza_t* pres = xmpp_stanza_new(Ctx);
        xmpp_stanza_set_name(pres, "presence");
        xmpp_send(Conn, pres);
        xmpp_stanza_release(pres);
    } else {
        fprintf(stderr, "DEBUG: disconnected\n");
        xmpp_stop(Ctx);
    }
}

int onreceive(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata)
{
    printf("============================================================\n");
    /* error checking */
    if(!xmpp_stanza_get_child_by_name(stanza, "body"))
        return 1;
    if(xmpp_stanza_get_type(stanza) !=NULL && !strcmp(xmpp_stanza_get_type(stanza), "error"))
        return 1;
    /* Print received message */
    char* intext = xmpp_stanza_get_text(xmpp_stanza_get_child_by_name(stanza, "body"));
    printf("Incoming message from %s: %s\n", xmpp_stanza_get_from(stanza), intext);
    /* Reply to the message */
    sendmsg(xmpp_stanza_get_attribute(stanza, "from"), "hello to you too!");
    return 1;
}

void sendmsg(char* recipient, char* message)
{
    xmpp_stanza_t* msg = xmpp_stanza_new(Ctx);
    xmpp_stanza_set_name(msg, "message");
    xmpp_stanza_set_attribute(msg, "to", recipient);
    xmpp_stanza_set_attribute(msg, "type", "chat");
    xmpp_stanza_t* newbody = xmpp_stanza_new(Ctx);
    xmpp_stanza_set_name(newbody, "body");
    xmpp_stanza_t* text = xmpp_stanza_new(Ctx);
    xmpp_stanza_set_text(text, message);
    xmpp_stanza_add_child(newbody, text);
    xmpp_stanza_add_child(msg, newbody);
    xmpp_send(Conn, msg);
    xmpp_stanza_release(msg);
}

//int handle_roster(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata)
//{
//    xmpp_stanza_t *query, *item;
//    char *type, *name;
//
//    type = xmpp_stanza_get_type(stanza);
//    if (strcmp(type, "error") == 0) {
//        fprintf(stderr, "ERROR: query failed\n");
//    } else {
//        query = xmpp_stanza_get_child_by_name(stanza, "query");
//        printf("Roster:\n");
//        for (item = xmpp_stanza_get_children(query); item; item = xmpp_stanza_get_next(item))
//            if ((name = xmpp_stanza_get_attribute(item, "name")))
//                printf("\t %s (%s) sub=%s\n",
//                       name,
//                       xmpp_stanza_get_attribute(item, "jid"),
//                       xmpp_stanza_get_attribute(item, "subscription"));
//            else
//                printf("\t %s sub=%s\n",
//                       xmpp_stanza_get_attribute(item, "jid"),
//                       xmpp_stanza_get_attribute(item, "subscription"));
//        printf("END OF LIST\n");
//    }
//
//    /* disconnect */
//    xmpp_disconnect(conn);
//
//    return 0;
//}


/*
	Xin Yang.
	Bug report: xyang.cq@gmail.com
*/

#ifndef _NGX_SOCK_H_INCLUDED_
#define _NGX_SOCK_H_INCLUDED_

#include <ngx_config.h>
#include <ngx_core.h>

#define NGX_SOCKS_CONF 0x02000000

typedef enum{
	ngx_socks_start = 0,
	ngx_socks_authorized,
	ngx_socks_upstream,
};

typedef struct{
	struct sockaddr_in addr;
	int timeout;
	int proxy_timeout;
}ngx_socks_conf_t;

typedef struct{
	ngx_str_t name;
	void *(*create_conf)(ngx_cycle_t *cycle);
	char *(*init_conf)(ngx_cycle_t *cycle, void *conf);
}ngx_socks_module_t;

typedef struct{
	ngx_connection_t *connection;
	ngx_str_t out;
	void *ctx;
	ngx_uint_t state;
}ngx_session_t;

typedef struct{
	ngx_str_t name;
	in_port_t port[4];
	ngx_uint_t type;
	
}ngx_socks_protocal_t;

typedef void (*ngx_socks_init_session_pt)(ngx_session_t *, ngx_connection_t *);
typedef void (*ngx_socks_init_protocal_pt)(ngx_event_t *);
typedef ngx_int_t (*ngx_socks_parse_command)(ngx_session_t *);
typedef void ngx_socks_auth_state(ngx_event_t *);

static char *ngx_socks_block(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *ngx_socks_port(ngx_conf_t *cf, ngx_command_t *cmd, void *conf); 

static void *ngx_socks_module_create_conf(ngx_cycle_t *cycle);
static char * ngx_socks_module_init_conf(ngx_cycle_t *cycle, void *conf);

void ngx_socks_init_connection(ngx_connection_t *c);
void ngx_socks_init_session(ngx_session_t *s, ngx_connection_t *c);
void ngx_socks_init_protocal(ngx_event_t *rev);
ngx_int_t ngx_socks_parse_command(ngx_session_t *s, );
void ngx_socks_auth_state(ngx_event_t *rev);
void ngx_socks_proxy_init(ngx_session_t *s);
void ngx_socks_proxy_read_handler();


#endif

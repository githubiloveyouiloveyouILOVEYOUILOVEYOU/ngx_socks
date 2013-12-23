#include <ngx_conf_file.h>
#include <ngx_event.h>
#include "ngx_socks.h"

int ngx_socks_max_module;

static ngx_command_t ngx_socks_commands[] = {
    {
		ngx_string("socks"),
		NGX_MAIN_CONF|NGX_CONF_BLOCK|NGX_CONF_NOARGS,
		ngx_socks_block,
		0,
		0,
		NULL,
	},

};

static ngx_command_t ngx_socks_core_commands[] = {
	{
		ngx_string("port"),
		NGX_SOCKS_CONF|NGX_CONF_TAKE1,
		ngx_socks_port,
		0,
		0,
		NULL,
	},
	
	{
		ngx_string("timeout"),
		NGX_SOCKS_CONF|NGX_CONF_TAKE1,
		ngx_conf_set_num_slot,
		0,
		offsetof(ngx_socks_conf_t, timeout),
		NULL,
	},
    
    {
        ngx_string("keepalive"),
        NGX_SOCKS_CONF|NGX_CONF_TAKE1,
        ngx_conf_set_num_slot,
        0,
        offsetof(ngx_socks_conf_t, keepalive),
        NULL,
    },
    
	{
		ngx_string("proxy_timeout"),
		NGX_SOCKS_CONF|NGX_CONF_TAKE1,
		ngx_conf_set_num_slot,
		0,
		offsetof(ngx_socks_conf_t, proxy_timeout),
		NULL,
	},

	ngx_null_command,
};

static ngx_core_module_t ngx_socks_module_ctx = {
	ngx_string("socks"),
    NULL,
    ngx_socks_module_init_conf,
};

static ngx_socks_module_t ngx_socks_core_module_ctx = {
    ngx_string("socks_core"),
    ngx_socks_core_module_create_conf,
    NULL,
};

ngx_module_t ngx_socks_module = {
	NGX_MODULE_V1,
	&ngx_socks_module_ctx,
	ngx_socks_commands,
	NGX_CORE_MODULE,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NGX_MODULE_V1_PADDING,
};

ngx_module_t ngx_socks_core_module = {
	NGX_MODULE_V1,
	&ngx_socks_core_module_ctx,
	ngx_socks_core_commands,
	NGX_SOCKS_MODULE,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NGX_MODULE_V1_PADDING,
};


char *ngx_socks_module_init_conf(ngx_cycle_t *cycle, void *conf){
    if(ngx_get_conf(cycle->conf_ctx, ngx_socks_module) == NULL){
        ngx_log_error(NGX_LOG_ERR, cycle->log, 0,
                      "no \"socks\" section in configuration");
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}

void *ngx_socks_core_module_create_conf(ngx_cycle_t *cycle){
	
	ngx_socks_conf_t *scf;
	scf = ngx_pcalloc(cycle->pool, sizeof(ngx_socks_conf_t));
	if(scf == NULL){
		return NULL;
	}
    
    scf->addr.sin_family = AF_INET;
	scf->addr.sin_addr.s_addr = htonl(INADDR_ANY);
	scf->addr.sin_port = htons(1080);
    scf->timeout = NGX_CONF_UNSET;
    scf->proxy_timeout = NGX_CONF_UNSET;
    scf->keepalive = NGX_CONF_UNSET;

	return scf;
}


char *ngx_socks_block(ngx_conf_t *cf, ngx_command_t *cmd, void *conf){

    int i;
	char *rv;
    ngx_listening_t *ls;
    void ***ctx;
    ngx_socks_module_t *m;
    ngx_conf_t pcf;
    ngx_socks_conf_t *scf;
    

    /* Count the number of the event modules and set up their indices*/
    ngx_socks_max_module = 0;
    for(i = 0; ngx_modules[i]; i++){
        if(ngx_modules[i]->type != NGX_SOCKS_MODULE){
            continue;
        }
        ngx_modules[i]->ctx_index = ngx_socks_max_module++;
    }

    ctx = ngx_pcalloc(cf->pool, sizeof(void *));
    if(ctx == NULL){
        return NGX_CONF_ERROR;
    }

    *ctx = ngx_pcalloc(cf->pool, sizeof(void *)*ngx_socks_max_module);
    if(*ctx == NULL){
        return NGX_CONF_ERROR;
    }
    *(void **)conf = ctx;

    for(i = 0; ngx_modules[i]; i++){
        if(ngx_modules[i]->type != NGX_SOCKS_MODULE){
            continue;
        }

        m = ngx_modules[i]->ctx;
        if(m->create_conf){
            (*ctx)[ngx_modules[i]->ctx_index] = m->create_conf(cf->cycle);
            if((*ctx)[ngx_modules[i]->ctx_index] == NULL){
                return NGX_CONF_ERROR;
            }
        }
    }
    
    ngx_log_error(NGX_LOG_DEBUG, cf->log, 0, "ngx_socks_block");
    
	pcf = *cf;
    cf->ctx = ctx;
    cf->module_type = NGX_SOCKS_MODULE;
	cf->cmd_type = NGX_SOCKS_CONF;
	rv = ngx_conf_parse(cf, NULL);

	*cf = pcf;
	
	if(rv != NGX_CONF_OK)
		return rv;

    // Initialize socks modules
    for(i = 0; ngx_modules[i]; i++){
        if(ngx_modules[i]->type != NGX_SOCKS_MODULE){
            continue;
        }

        m = ngx_modules[i]->ctx;
        if(m->init_conf){
            rv = m->init_conf(cf->cycle, (*ctx)[ngx_modules[i]->ctx_index]);
            if(rv != NGX_CONF_OK)
                return rv;
        }
    }

    scf = (*ctx)[ngx_socks_core_module.ctx_index];
    /* Create listening */
    ls = ngx_create_listening(cf, (struct sockaddr *)&scf->addr,
                              sizeof(struct sockaddr_in));
    if(ls == NULL)
        return NGX_CONF_ERROR;

    ls->addr_ntop = 1;
    ls->handler = ngx_socks_init_connection;
    ls->pool_size = 256;
    ls->logp = &cf->cycle->new_log;

	return NGX_CONF_OK;
}

char *ngx_socks_port(ngx_conf_t *cf, ngx_command_t *cmd, void *conf){

	ngx_socks_conf_t *scf = conf;
	ngx_str_t *value;
    
    ngx_log_error(NGX_LOG_DEBUG, cf->log, 0, "ngx_sock_port");
	value = cf->args->elts;
	scf->addr.sin_port = htons((unsigned short)ngx_atoi(value[1].data, value[1].len));

	return NGX_CONF_OK;
}

void ngx_socks_init_connection(ngx_connection_t *c){

    ngx_event_t *wev;
    unsigned char buf[] = "hello world\n";
    int n;

    ngx_log_error(NGX_LOG_DEBUG, c->log, 0, "ngx_socks_init_connection");
    wev = c->write;
    if(wev->ready){
        n = c->send(c, buf, sizeof(buf));
        ngx_log_error(NGX_LOG_DEBUG, c->log, 0, "send %d bytes", n);
    }
}



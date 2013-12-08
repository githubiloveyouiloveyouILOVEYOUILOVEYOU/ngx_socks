#include "ngx_sock.h"

static ngx_command_t ngx_sock_commands[] = {
	
	{
		ngx_string("sock"),
		NGX_MAIN_CONF|NGX_DIRECT_CONF|NGX_CONF_BLOCK|NGX_CONF_NOARGS	,
		ngx_sock_block,
		0,
		0,
		NULL,
	},
	
	{
		ngx_string("port"),
		NGX_DIRECT_CONF|NGX_SOCK_CONF|NGX_CONF_TAKE1,
		ngx_sock_port,
		0,
		0,
		NULL,
	},

	ngx_null_command,
};

static ngx_sock_module_t ngx_sock_module_ctx = {
	ngx_string("sock"),
	ngx_sock_module_create_conf,
	ngx_sock_module_init_conf,
};

ngx_module_t ngx_sock_module = {
	NGX_MODULE_V1,
	&ngx_sock_module_ctx,
	ngx_sock_commands,
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

void *ngx_sock_module_create_conf(ngx_cycle_t *cycle){
	
	ngx_sock_conf_t *scf;
	scf = ngx_pcalloc(cycle->pool, sizeof(ngx_sock_conf_t));
	if(scf == NULL){
		return NULL;
	}
	
	return scf;
}


char *ngx_sock_module_init_conf(ngx_cycle_t *cycle, void *conf){
	
	ngx_sock_conf_t *scf = conf;
	scf->addr.sin_family = AF_INET;
	scf->addr.sin_addr.s_addr = htonl(INADDR_ANY);
	scf->addr.sin_port = htons(1080);

	return NGX_CONF_OK;
}

char *ngx_sock_block(ngx_conf_t *cf, ngx_command_t *cmd, void *conf){

	char *rv;
	ngx_conf_t pcf;
	pcf = *cf;
	cf->cmd_type = NGX_SOCK_CONF;

	rv = ngx_conf_parse(cf, NULL);

	*cf = pcf;
	
	if(rv != NGX_CONF_OK)
		return rv;	

	return NGX_CONF_OK;
}

char *ngx_sock_port(ngx_conf_t *cf, ngx_command_t *cmd, void *conf){

	ngx_sock_conf_t *scf = conf;
	ngx_str_t *value;

	value = cf->args->elts;
	scf->addr.sin_port = htons((unsigned short)ngx_atoi(value[1].data, value[1].len));

	return NGX_CONF_OK;
}


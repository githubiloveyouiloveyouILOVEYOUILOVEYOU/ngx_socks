#ngx_socks

A nginx module that can offer sock5 service. Currently this module can only
support simple socks5 without user identification.

####INSTALL
  Configure with an extra option '--add-module=/path/to/ngx_sock/' and make && make install
  
####CONFIGURATION
  add these to your nginx.conf
  <pre><code>
  socks{
    port 8080; # whatever port you want
    timeout 10; # 10 seconds, timeout for negotiating between nginx and clients
    keepalive 5; # 5 minutes
    proxy_timetout 10; # 10 seconds, timeout between nginx and other proxy servers.
  }
  </code></pre>

####TODO:
1. BIND support 
2. Support user identification with user name and password
3. Support multi-port configuration as configured like 'server' in http module
  

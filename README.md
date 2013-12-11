#ngx_socks

A nginx module that can offer sock5 service. Currently this module can only
support simple socks5 without user identification.

####INSTALL
  Configure with an extra option '--add-module=/path/to/ngx_sock/' and make && make install
  
####CONFIGURATION
  add these to your nginx.conf
  <pre><code>
  sock{
    port 8080; # whatever port you want
    timeout 10; #timeout between nginx and clients
    proxy_timetout;  #timeout between nginx and other proxy servers.
  }
  </code></pre>

####TODO:
1. Support user identification with user name and password
2. Support multi-port configuration as configured like 'server' in http module
  

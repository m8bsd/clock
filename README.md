### Coding my own mailserver
### Install Go on FreeBSD using pkg:
```
sudo pkg update
sudo pkg install go git openssl
```
### Create Project
```
mkdir ~/mailserver
cd ~/mailserver

go mod init mailserver
go get github.com/emersion/go-smtp
```
### Generate TLS Certificates on FreeBSD
```
sudo mkdir -p /usr/local/etc/mailserver

cd /usr/local/etc/mailserver

sudo openssl req -x509 -newkey rsa:4096 \
-keyout key.pem \
-out cert.pem \
-days 365 \
-nodes
```
### Open Firewall Ports (PF) Edit /etc/pf.conf:
```
pass in proto tcp to any port 25
pass in proto tcp to any port 587
pass in proto tcp to any port 993
```
### Enable PF:
```
sudo service pf enable
sudo service pf start
```
### Build Binary
```
go build -o mailserver
```
### Run as Root (needed for port 25)
```
sudo ./mailserver
```
### Create FreeBSD Service Create: /usr/local/etc/rc.d/mailserver
```
#!/bin/sh

# PROVIDE: mailserver
# REQUIRE: NETWORKING
# KEYWORD: shutdown

. /etc/rc.subr

name="mailserver"
rcvar=mailserver_enable

command="/usr/local/bin/mailserver"

load_rc_config $name

: ${mailserver_enable:=no}

run_rc_command "$1"
```
### Make executable:
```
sudo chmod +x /usr/local/etc/rc.d/mailserver
```
### Install binary:
```
sudo cp mailserver /usr/local/bin/
```
### Enable service:
```
sudo sysrc mailserver_enable=YES
sudo service mailserver start
```
### DNS Setup Your domain needs:
```
mail.example.com   A      YOUR_SERVER_IP
example.com        MX 10  mail.example.com
```
Also add:

SPF
DKIM
DMARC
PTR reverse DNS

Otherwise Gmail/Outlook will reject mail.

### Recommended FreeBSD Packages Useful additions:
```
sudo pkg install \
redis \
rspamd \
dovecot \
nginx \
postgresql16-server
```
These give you:
| Package | Purpose               |
| ------- | --------------------- |
| Rspamd  | Spam filtering        |
| Dovecot | IMAP access           |
| Redis   | Mail queues/cache     |
| Nginx   | Webmail reverse proxy |

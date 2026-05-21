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

# Installation de MQTT

Nous allons installer mosquitto pour Raspberry Pi, une référence pour faire du MQTT

### Installation Mosquitto
Se connecter en ssh sur votre Pi, il doit y avoir les environnements de développement, sinon faites un  

`apt-get install build-essential git-core libcjson-dev libcurl4-openssl-dev cmake`

Ensuite :
```
$ mkdir MQTT
$ cd MQTT
$ wget https://mosquitto.org/files/source/mosquitto-2.0.15.tar.gz
$ wget https://github.com/warmcat/libwebsockets/archive/refs/tags/v4.3.2.tar.gz
$ mv v4.3.2.tar.gz libwebsockets-4.3.2.tar.gz

$ sudo apt-get install uuid-dev
$ sudo apt-get install xsltproc docbook-xsl

$ tar -zxvf libwebsockets-4.3.2.tar.gz
$ cd libwebsockets-4.3.2/
$ mkdir build
$ cd build
$ cmake -DLWS_WITH_SSL=0 -DLWS_IPV6=ON -DLWS_WITHOUT_CLIENT=ON -DLWS_WITHOUT_EXTENSIONS=ON -DLWS_WITH_ZIP_FOPS=OFF -DLWS_WITH_ZLIB=OFF -DLWS_WITH_SHARED=ON ..
$ make -j 4
$ sudo make install
$ sudo ldconfig

$ cd ~/MQTT
$ tar -zxvf mosquitto-2.0.15.tar.gz
$ cd mosquitto-2.0.15/
$ vi config.mk

Mettre # devant
WITH_TLS:=yes
WITH_TLS_PSK:=yes
Remplacer:
WITH_WEBSOCKETS:=no
par
WITH_WEBSOCKETS:=yes
---
Sauvegarder votre fichier (:wq)

$ make -j 4
$ sudo make install
$ sudo ldconfig
$ sudo cp mosquitto.conf /usr/local/etc/mosquitto.conf

Vous avez maintenant les outils mosquitto_pub, mosquitto_sub, mosquitto_rr dans /usr/local/bin

Vous avez maintenant la possibilité de lancer un serveur MQTT de type mosquitto dans /usr/local/sbin
avec la configuration dans mosquitto.conf $ sudo gedit /usr/local/etc/mosquitto.conf

ET les librairies dans /usr/local/include et /usr/local/lib (C et C++)
```

## Divers
Vous pouvez aller voir les nouveautés et autres projets

[1]: https://github.com/warmcat/libwebsockets
[2]: https://mosquitto.org/download/
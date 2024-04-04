# Teleinfo Universal Library

Programme d'exemple de la librairie universelle pour la téléinformation pour Raspberry Pi

## Documentation
J'ai écrit un article [dédié][10] sur cette librairie, vous pouvez aussi voir les [catégories][6] associées à la téléinfo sur mon [blog][7].

Pour les commentaires et le support vous pouvez allez sur le [forum][8] dédié ou dans la [communauté][9] 

### Installation base : Publication curl uniquement
Se connecter en ssh sur votre Pi, il doit y avoir les environnements de développement, sinon faites un  

`apt-get install build-essential git-core libcjson-dev libcurl4-openssl-dev`

Nous utilisons la version récente de développement de la librairie spdlog (Ne pas utiliser le packge):
Version Static (.a)
```
$ cd /home/pi/GIT    (Adapter en fonction de votre système / voir aussi dans Makefile ensuite)
$ git clone https://github.com/gabime/spdlog.git
$ cd spdlog && mkdir build && cd build
$ cmake .. && make -j
```

Version Dynamic (.so)
```
$ cd /home/pi/GIT    (Adapter en fonction de votre système / voir aussi dans Makefile ensuite)
$ git clone https://github.com/gabime/spdlog.git
$ cd spdlog && mkdir build && cd build
$ cmake .. -DSPDLOG_BUILD_BENCH=OFF -DSPDLOG_BUILD_EXAMPLES=OFF -DSPDLOG_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_INSTALL_LIBDIR=lib -DSPDLOG_BUILD_SHARED=ON
$ cd ../include & sudo cp -Rf spglog /usr/local/include/spdlog
$ sudo cp -a build/libspdlog.* /usr/local/lib/
$ sudo ldconfig
```

Ensuite :
```
git clone https://github.com/Tifaifai/LibTeleinfo
cd LibTeleinfo/examples/Raspberry_cJSON/
make
./raspjson
```

Exemple avec un alias de lancement :
alias linky='/usr/local/bin/raspjson_standard -m s -y /dev/ttyACM0 -e -k aa38f67...... -r http://....fr/input/post -n linky -d'

### Installation MQTT : Publication MQTT
Lire le README_MQTT.md pour installer MQTT (mosquitto 2.0.15 avec support de Websockets 4.3.2)

Ensuite :
```
git clone https://github.com/Tifaifai/LibTeleinfo
cd LibTeleinfo/examples/Raspberry_cJSON/
make raspjson_mqtt
./raspjson_mqtt
```

Et voilà ce que ça donne avec un dongle MicroTeleinfo en mode Historique sur le Linky

```
root@pi01(rw):~# git clone https://github.com/Tifaifai/LibTeleinfo
Cloning into 'LibTeleinfo'...
remote: Counting objects: 23, done.
remote: Compressing objects: 100% (20/20), done.
remote: Total 23 (delta 5), reused 21 (delta 3), pack-reused 0
Unpacking objects: 100% (23/23), done.
root@pi01(rw):~# cd LibTeleinfo/examples/Raspberry_cJSON/
root@pi01(rw):~/LibTeleinfo/examples/Raspberry_cJSON# make

....

root@pi01(rw):~/LibTeleinfo/examples/Raspberry_JSON# ./raspjson -d /dev/ttyUSB0
{"_UPTIME":34957, "ADCO":2147483647, "OPTARIF":"HC..", "ISOUSC":15, "HCHC":247418, "HCHP":0, "PTEC":"HC..", "IINST":1, "IMAX":1, "PAPP":150, "HHPHC":"A", "MOTDETAT":0}
{"PAPP":140}
{"HCHC":247419}
{"PAPP":160}
{"PAPP":150}
{"PAPP":160}
{"PAPP":150}
{"PAPP":160}
{"PAPP":140}
{"HCHC":247420}
{"PAPP":150}
{"PAPP":170}
{"PAPP":160}
{"_UPTIME":35017, "ADCO":2147483647, "OPTARIF":"HC..", "ISOUSC":15, "HCHC":247420, "HCHP":0, "PTEC":"HC..", "IINST":1, "IMAX":1, "PAPP":160, "HHPHC":"A", "MOTDETAT":0}
{"PAPP":150}
{"PAPP":140}
{"PAPP":150}
{"PAPP":160}
{"PAPP":150}
{"PAPP":140}
{"PAPP":160}
{"PAPP":170}
{"HCHC":247421, "PAPP":160}
{"PAPP":140}
{"PAPP":150}
{"PAPP":140}
{"PAPP":150}
{"PAPP":140}
{"PAPP":150}
{"PAPP":140}
{"PAPP":150}
{"PAPP":160}
{"PAPP":150}
{"PAPP":160}
{"HCHC":247422, "PAPP":150}
{"_UPTIME":35077, "ADCO":2147483647, "OPTARIF":"HC..", "ISOUSC":15, "HCHC":247422, "HCHP":0, "PTEC":"HC..", "IINST":1, "IMAX":1, "PAPP":140, "HHPHC":"A", "MOTDETAT":0}
{"PAPP":150}
{"PAPP":140}
````

## Divers
Vous pouvez aller voir les nouveautés et autres projets sur [blog][7] 

[6]: https://hallard.me/category/tinfo/
[7]: https://hallard.me
[8]: https://community.hallard.me/category/7
[9]: https://community.hallard.me
[10]: https://hallard.me/libteleinfo

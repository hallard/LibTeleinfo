Teleinfo Universal Library
==========================

Programme d'exemple de la librairie universelle pour la téléinformation pour Raspberry Pi

##Documentation
J'ai écrit un article [dédié][10] sur cette librairie, vous pouvez aussi voir les [catégories][6] associées à la téléinfo sur mon [blog][7].

Pour les commentaires et le support vous pouvez allez sur le [forum][8] dédié ou dans la [communauté][9] 

###Installation
Se connecter en ssh sur votre Pi, il doit y avoir les environnements de développement, sinon faites un
`apt-get install build-essential git-core`

Ensuite :
```
git clone https://github.com/hallard/LibTeleinfo
cd LibTeleinfo/examples/Raspberry_JSON/
make
./raspjson
```

Et voilà ce que ça donne avec un dongle MicroTeleinfo

```
root@pi01(rw):~/LibTeleinfo/examples/Raspberry_JSON# ./raspjson -d /dev/ttyUSB0
{"_UPTIME":30788, "ADCO":2147483647, "OPTARIF":"HC..", "ISOUSC":15, "HCHC":247295, "HCHP":0, "PTEC":"HC..", "IINST":1, "IMAX":1, "PAPP":140, "HHPHC":"A", "MOTDETAT":0}
{"HCHC":247296}
{"PAPP":170}
{"HCHC":247297}
q{"PAPP":160}
{"PAPP":140}
{"_UPTIME":30848, "ADCO":2147483647, "OPTARIF":"HC..", "ISOUSC":15, "HCHC":247297, "HCHP":0, "PTEC":"HC..", "IINST":1, "IMAX":1, "PAPP":140, "HHPHC":"A", "MOTDETAT":0}
{"PAPP":150}
{"PAPP":140}
{"PAPP":150}
{"PAPP":140}
{"PAPP":160}
{"PAPP":170}
{"PAPP":160}
{"HCHC":247298, "PAPP":140}
{"PAPP":150}
{"PAPP":140}
{"PAPP":150}
{"PAPP":140}
{"PAPP":150}
{"PAPP":140}
{"PAPP":150}
{"PAPP":160}
{"PAPP":150}
{"PAPP":170}
{"PAPP":160}
{"PAPP":170}
{"PAPP":150}
{"HCHC":247299, "PAPP":160}
{"PAPP":150}
{"_UPTIME":30908, "ADCO":2147483647, "OPTARIF":"HC..", "ISOUSC":15, "HCHC":247299, "HCHP":0, "PTEC":"HC..", "IINST":1, "IMAX":1, "PAPP":160, "HHPHC":"A", "MOTDETAT":0}
{"PAPP":140}
{"PAPP":150}
{"PAPP":140}
{"PAPP":150}
````

##Divers
Vous pouvez aller voir les nouveautés et autres projets sur [blog][7] 

[6]: https://hallard.me/category/tinfo/
[7]: https://hallard.me
[8]: https://community.hallard.me/category/7
[9]: https://community.hallard.me
[10]: https://hallard.me/libteleinfo



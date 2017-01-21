# Teleinfo Universal Library
This is a fork of Teleinfo Universal Library for the ESP8266 MCU  
This is a generic Teleinfo French Meter Measure Library  
- Github source : <https://github.com/hallard/LibTeleinfo>

# My Features
- Add all possible variable (without ejp) following : 

Ces différents messages donnent les indications suivantes en fonction de l’abonnement souscrit :
N° d’identification du compteur : ADCO (12 caractères)
Option tarifaire (type d’abonnement) : OPTARIF (4 car.)
Intensité souscrite : ISOUSC ( 2 car. unité = ampères)
Index si option = base : BASE ( 9 car. unité = Wh)
Index heures creuses si option = heures creuses : HCHC ( 9 car. unité = Wh)
Index heures pleines si option = heures creuses : HCHP ( 9 car. unité = Wh)
Index heures normales si option = EJP : EJP HN ( 9 car. unité = Wh)
Index heures de pointe mobile si option = EJP : EJP HPM ( 9 car. unité = Wh)
Index heures creuses jours bleus si option = tempo : BBR HC JB ( 9 car. unité = Wh)
Index heures pleines jours bleus si option = tempo : BBR HP JB ( 9 car. unité = Wh)
Index heures creuses jours blancs si option = tempo : BBR HC JW ( 9 car. unité = Wh)
Index heures pleines jours blancs si option = tempo : BBR HP JW ( 9 car. unité = Wh)
Index heures creuses jours rouges si option = tempo : BBR HC JR ( 9 car. unité = Wh)
Index heures pleines jours rouges si option = tempo : BBR HP JR ( 9 car. unité = Wh)
Préavis EJP si option = EJP : PEJP ( 2 car.) 30mn avant période EJP
Période tarifaire en cours : PTEC ( 4 car.)
Couleur du lendemain si option = tempo : DEMAIN
Intensité instantanée : IINST ( 3 car. unité = ampères)
Avertissement de dépassement de puissance souscrite : ADPS ( 3 car. unité = ampères) (message émis uniquement en cas de dépassement effectif, dans ce cas il est immédiat)
Intensité maximale : IMAX ( 3 car. unité = ampères)
Puissance apparente : PAPP ( 5 car. unité = Volt.ampères)
Groupe horaire si option = heures creuses ou tempo : HHPHC (1 car.)
Mot d’état (autocontrôle) : MOTDETAT (6 car.)

HTTP Request push datas 



But
====

Transformer un vieux panneau de contrôle industriel devenu inutile en
un objet vintage et fun à l'aide d'un microcontrôleur.


Description
=============

Ce panneau est composé de :

 - 2 gros galvanomêtres 
   - un ampermêtre gradué jusquầ 1000[A]
   - un voltmètre gradué jusqu'à 12[V].

 - 2 temoins lumineux.

 - 2 leviers (2 pos) et un interrupteur.


Transformations
================

galva
------

L'ampermètre a été remplacé par un voltmètre ce qui facilite
grandement ce hack. L'arduino ayant des sorties PWM, il suffit de
brancher les galva entre la terre et une sortie PWM pour qu'il affiche
la tension de cette sortie.

Premier problème, l'arduino fonctionne en 5V alors que les galva sont
gradué en 12V. Il a suffit de changer la resistance interne du
voltmètre (5[kOhm]). 

.. math:: I = \frac{U_{org}}{R_{org}} = \frac{U_{arduino}}{R_{new}}

.. math:: R_{new} = \frac{U_{arduinno}R_{org}}{U_{org}} = \frac{5 5000}{12} = 2kOhm


levier
-------

L'un des deux levier était "normalement fermé". Il a fallut le
démonter et changer son fonctionnement pour qu'il corresponde à
l'autre.

Les leviers sont au repo en position vertical et ont deux sorties
(doite ou gauche).
 

Temoins lumineux
=================

À l'origine, les temoins étaient des ampoules à inquendescence. Il
était plus simple de les remplacer par des LED.

Le verre d'un des temoins est teinté en rouge (LED rouge) mais l'autre
étant blanc, en mettant une RGB, on peut choisir sa couleur.



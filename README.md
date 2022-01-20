Bonjour.

Pour utiliser notre encodeur / decodeur il faut 2 terminaux.
Dans le premier utilisez la commande :

sudo ./encodeur nom-du-texte-a-tester

Dans le second utilisez la commande :

sudo ./decodeur

le resultat encodé est stocké dans le fichier "donne" et le resultat décodé est stocké dans le fichier "output".

Tout les textes présents dans le dossier fonctionne. Il ne faut pas de caractères spéciaux dans le fichier à tester sous risque
de créer des erreurs de segmentations.

On à décidé de coder deux programmes différents et donc d'utiliser un tube nommé pour transférer les données du codeur au décodeur.
L'éxecution de l'encodeur envoie les données dans la pipe nommée et écris les codes envoyés dans un fichier appelé 'donne'.
L'éxecution du décodeur écrit les données dans un fichier appelé 'output'.

Dans l'algorithme du décodeur on a pas exactement implémenté votre algorithme LZW. Pour résoudre le problème que l'on a vu en cours
on a plutôt décidé de résoudre le problème dans le décodeur donc il y a une condition suplémentaire dans l'algorithme du décodeur LZW.

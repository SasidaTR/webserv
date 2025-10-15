Voici ce qu’il reste à faire ou à vérifier pour être conforme au sujet :

1 Points généraux
**- Robustesse : le serveur ne doit jamais crasher, même en cas d’erreur mémoire ou d’exception (déjà bien géré avec les try/catch dans main.cpp).
**- Makefile : il est conforme (règles demandées, pas de relinking inutile, flags OK).
- C++98 : pas d’utilisation de C++11 (vérifie bien l’absence de auto, nullptr, std::to_string, etc.).
**- Pas de libs externes : OK.
2 Fonctionnalités obligatoires
- Configuration : le parsing du fichier de config est fait (configParse), mais vérifie que toutes les directives du sujet sont bien supportées (erreur - - par défaut, upload_dir, redirect, etc.).
- Multi-port : déjà géré (listen_fds dans main.cpp).
- Non-blockant : tout passe par poll, pas de read/write hors poll (semble OK).
- Gestion des déconnexions : timeout et suppression des clients inactifs (OK).
- Méthodes HTTP : GET, POST, DELETE sont gérées dans le router, mais vérifie que chaque route/location peut restreindre les méthodes (isMethodAllowed).
- Upload : handler présent, à tester avec un POST réel.
- CGI : architecture en place, à tester avec un script réel.
- Réponses d’erreur : fichiers HTML d’erreur présents, la fonction setErrorBody les utilise.
- Directory listing : handler dédié, à tester.
- Redirections : parsing et gestion dans les locations, à tester.
- Serveur statique : handler OK, à tester sur plusieurs fichiers (html, css, js, images).
- Compatibilité navigateur : à tester (curl + navigateur).
- Chunked requests : la fonction dechunkBody existe, à tester.
- Body size limit : parsing dans la config, à tester.
- Pas de fork hors CGI : OK.
- 3 À tester et valider
- Test POST/DELETE/GET sur différentes routes (upload, delete, CGI, statique).
- Test d’upload de fichier (curl -F ou via formulaire HTML).
- Test d’exécution CGI (POST et GET sur un script).
- Test de redirection HTTP.
- Test d’autoindex (directory listing).
- Test d’erreur 404/403/405/413 (fichier manquant, méthode non autorisée, body trop gros).
- Test multi-port (plusieurs serveurs dans la config).
- Test de robustesse (déconnexion, timeout, stress test).
- Test navigateur (index.html + style.css + images).

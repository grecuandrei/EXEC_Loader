		Copyright @ 2021 Grecu Andrei-George 335CA, All rights reserved
			   
Tema 3 Loader de Executabile

TEXT : https://ocw.cs.pub.ro/courses/so/teme/tema-3
GITHUB : https://github.com/grecuandrei/EXEC_loader

## Structura:

    exec_parser.c
    exec_parser.h
    loader.c (modificat)
    loader.h
    Makefile

## Tema

    Implementarea este sub forma unei biblioteci partajate/dinamice a unui loader
    de fișiere executabile în format ELF pentru Linux. Loader-ul va încărca fișierul
    executabil în memorie pagină cu pagină, folosind un mecanism de tipul demand
    paging - o pagină va fi încărcată doar în momentul în care este nevoie de ea.
    Pentru simplitate, loader-ul va rula doar executabile statice - care nu sunt
    link-ate cu biblioteci partajate/dinamice.

    Un handler pentru SIGSEGV a fost scris pentru eventualele erori ale executabilului.

## Descriere

    Functia de so_execute este cea care deschide executabilul, il porneste, si aloca
    memorie in cadrul fiecarui segment a unui vector de int-uri ce va fi folosit pentru
    a decide daca pagina a fost mapata sau nu (mapped ia valori 0 sau 1).

    So_init_loader este functia care initializeaza loader-ul si face setup-ul pentru
    handler(conform laboratorului).

    Handlerul (segv_handler) va fi functia apelata in cazul unei erori.

        Daca aceasta eroare este de alt tip decat SIGSEGV, nu o vom trata noi, si se va apela
        handlerul default.

        Daca insa este un SIGSEGV, se verifica pentru fiecare segment daca adresa paginii
        este inclusa in intervalul unui segment alocat. 

        Daca adresa este valida(se afla intr-un segment),
        Pentru orice adresa valida(minim una), se seteaza flagul valid pe 1.
        Se calculeaza numarul paginii si informatia extra stocata in data.

        Daca pagina nu este inca mapata,
        Daca pagina nu a trecut inca de file_size,
        Se pozitioneaza file pointerul la offsetul corect in fisier, se mapeaza pagina,
        se citeste in buffer, se scrie in memorie datele respective din buffer (cazul in care
        file_size este mai mic decat o pagina(si trebuie zeroizat o bucata) este acoperit de
        faptul ca bufferul se aloca cu calloc).
        Altfel se face zero toata pagina (bufferul este deja tot zero).
        Altfel pagina este deja mapata si trebuie intoarsa.

        Daca pagina nu a fost valida in nici un segment(flagul valid ramane setat pe 0),
        se apeleaza handlerul default cu SIGSEGV pentru ca este un acces invalid.

## Teste
    
    Nu am intalnit probleme in cadrul testelor

## Rulare

    make
    make -f Makefile.checker
    ./run_all.sh
    ./_test/run_test.sh no (unde no = numarul testului 0-9)



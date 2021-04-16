
int main(int argc, char *argv[]) {
    // serwer <nazwa-katalogu-z-plikami> <plik-z-serwerami-skorelowanymi> [<numer-portu-serwera>]
    // serwer katalog skorelowane.txt port

    // Parametr z nazwą katalogu jest parametrem obowiązkowym i może być podany jako ścieżka
    // bezwzględna lub względna. W przypadku ścieżki względnej serwer próbuje odnaleźć wskazany
    // katalog w bieżącym katalogu roboczym.

    // Parametr wskazujący na listę serwerów skorelowanych jest parametrem obowiązkowym

    // Parametr z numerem portu serwera jest parametrem opcjonalnym i wskazuje numer portu
    // na jakim serwer powinien nasłuchiwać połączeń od klientów. Domyślny numer portu to 8080.

    /*****************************************************************************************/

    // Po uruchomieniu serwer powinien odnaleźć wskazany katalog z plikami i rozpocząć
    // nasłuchiwanie na połączenia TCP od klientów na wskazanym porcie. Jeśli otwarcie katalogu,
    // odczyt z katalogu, bądź otwarcie gniazda sieciowego nie powiodą się, to program powinien
    // zakończyć swoje działanie z kodem błędu EXIT_FAILURE.

    /*****************************************************************************************/

    // Serwer po ustanowieniu połączenia z klientem oczekuje na żądanie klienta.
    // Serwer powinien zakończyć połączenie w przypadku przesłania przez klienta niepoprawnego
    // żądania. W takim przypadku serwer powinien wysłać komunikat o błędzie, ze statusem 400, a
    // następnie zakończyć połączenie.
    //
    // Jeśli żądanie klienta było jednak poprawne, to serwer powinien oczekiwać na
    // ewentualne kolejne żądanie tego samego klienta lub zakończenie połączenia przez klienta.

    return 0;
}

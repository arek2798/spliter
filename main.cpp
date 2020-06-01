#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

using namespace std;

#define dzielnik 0b10110111001011101
#define bits_17 0xffff
#define bits_8 0xff

void tworzenie_tablicy_crc(unsigned int tablica_crc[256]) ///funkcja tworzy tablice lookup przyspieszajaca liczenie crc(liczenie po bajcie a nie po bicie)
{
    unsigned int liczba = 0;
    for(int i=0; i<256; i++) {
        liczba = i << 9;
        tablica_crc[i] = liczba ^ dzielnik;
    }
}

unsigned short int oblicz_crc(unsigned int tablica_crc[256], fstream &plik, bool sprawdzenie, unsigned short int crc_obl = 0)
{
    unsigned int reszta = 0xffffffff;               ///funkcja liczy crc pliku na podstawie tablicy
    uint8_t wartosc;
    unsigned int dane=0;
    char znak;
    int i=0;
    unsigned short int crc;

    plik.clear();
    plik.seekg(0);

    plik.get(znak);
    dane = static_cast<uint8_t>(znak);      ///zaladowanie pierwszych znakow do danych
    plik.get(znak);
    dane <<=8;
    dane += static_cast<uint8_t>(znak);
    plik.get(znak);
    dane <<=8;
    dane += static_cast<uint8_t>(znak);

    reszta = dane >> 15;

    while(plik.get(znak)) {                 ///liczenie crc tak dlugo az dojdziemy do konca pliku
        dane <<=8;                          ///ladowanie danych po 8 bitow
        dane += static_cast<uint8_t>(znak);
        wartosc = reszta >> 9;
        reszta = ((reszta << 8) | (dane >> 16)) & bits_17;
        reszta = reszta^tablica_crc[wartosc];
    }
    if(sprawdzenie) {                       ///sprawdzanie czy obliczone crc jest poprawne(zamiast 16 zer na koncu danych ustawiany jest obliczony crc)
        while(i++ < 4) {
            if(i < 2) {
                dane <<=8;
                dane += (crc >> ((1-i)*8)) & bits_8;
            } else {
                dane <<=8;
                dane += 0;
            }
            wartosc = reszta >> 9;
            reszta = ((reszta << 8) | (dane >> 16)) & bits_17;
            reszta = reszta^tablica_crc[wartosc];
        }
        crc = crc_obl-(reszta & bits_17);
        return crc;
    } else {
        while(i++ < 4) {
            dane <<=8;
            dane += 0;
            wartosc = reszta >> 9;
            reszta = ((reszta << 8) | (dane >> 16)) & bits_17;
            reszta = reszta^tablica_crc[wartosc];
        }
        crc = reszta & bits_17;
        return crc;
    }
}


int podaj_rozmiar_pliku(ifstream &plik)
{
    plik.clear();
    plik.seekg(0, ios::end);
    int rozmiar =  plik.tellg();
    plik.clear();
    plik.seekg(0);
    return rozmiar;
}

void dzielenie_pliku(int roz_plikow, char nazwa_pliku[], unsigned int tablica_crc[256], unsigned short int crc) {
    ifstream plik(nazwa_pliku);
    if(plik.fail()) {
        cout << "Podany plik nie istnieje!" << endl;
        return;
    }
    string nazwa_plikow="", nazwa, oznaczenie, nazwa_nastepnego;
    fstream plik_podz;
    fstream plik_kopia;
    int nr_nas;
    char plik_nazwa[100], znak;
    int roz_pliku = podaj_rozmiar_pliku(plik);
    int ile_plikow = roz_pliku/roz_plikow;
    if(roz_pliku%roz_plikow != 0) ile_plikow++;
    int nr=0;
    while(nazwa_pliku[nr] != '.') {
        nazwa_plikow += nazwa_pliku[nr];
        nr++;
    }
    for(int i=1; i<=ile_plikow; i++) {
        nr_nas = i+1;
        nazwa = nazwa_plikow;
        nazwa_nastepnego = nazwa_plikow;
        stringstream tekst, tekst2;         ///ustawienie odpowiedniej nazwy plikow(rowniez nastepnego)
        if(i<10) {
            tekst << i;
            tekst >> oznaczenie;
            nazwa += "00" + oznaczenie + ".dat";
            tekst2 << nr_nas;
            tekst2 >> oznaczenie;
            if(nr_nas == 10) {
                nazwa_nastepnego += "0" + oznaczenie + ".dat";
            } else {
                nazwa_nastepnego += "00" + oznaczenie + ".dat";
            }
        } else if(i>9 && i<100) {
            tekst << i;
            tekst >> oznaczenie;
            nazwa += "0" + oznaczenie + ".dat";
            tekst2 << nr_nas;
            tekst2 >> oznaczenie;
            if(nr_nas == 100) {
                nazwa_nastepnego += oznaczenie + ".dat";
            } else {
                nazwa_nastepnego += "0" + oznaczenie + ".dat";
            }
        } else if(i>99 && i<1000) {
            tekst << i;
            tekst >> oznaczenie;
            nazwa += oznaczenie + ".dat";
            tekst2 << nr_nas;
            tekst2 >> oznaczenie;
            nazwa_nastepnego += oznaczenie + ".dat";
        }
        if(i == ile_plikow) nazwa_nastepnego = "0";             ///dodanie naglowka do pliku
        strcpy(plik_nazwa, nazwa.c_str());
        plik_podz.open(plik_nazwa, ios::out | ios::binary);
        plik_kopia.open("kopia.txt", ios::out | ios::trunc);
        plik_podz << roz_pliku << " ";
        plik_podz << ile_plikow << " ";
        plik_podz << nazwa_nastepnego << " ";
        plik_podz << crc << " ";
        plik_podz << dzielnik << endl;
        int l_kopi = 0;
        while (l_kopi < roz_plikow && plik.get(znak)) {         ///wczytanie tylu znakow do pliku aby osiagnac rozmiar podany w konsoli
            plik_podz << znak;
            plik_kopia << znak;
            l_kopi++;
        }
        plik_kopia.close();
        plik_kopia.open("kopia.txt", ios::in);
        unsigned short int crc_podzielonego = oblicz_crc(tablica_crc, plik_kopia, false); ///liczenie crc juz podzielonego pliku i dodanie do stopki
        plik_podz << '\n';
        znak = static_cast<char>(crc_podzielonego >> 8);
        plik_podz << znak;
        znak = static_cast<char>(crc_podzielonego);
        plik_podz << znak;
        plik_kopia.close();
        remove("kopia.txt");
        plik_podz.close();
    }
    cout << "Plik zostal podzielony poprawnie." << endl;
    plik.close();
}

void scalanie_pliku(char nazwa_pliku[],  unsigned int tablica_crc[256])
{
    string naglowek, naz, naz_sc="";
    char nazwa_scalonego[100];
    fstream plik_zapis, plik_spr;
    stringstream naz_plik;
    naz_plik << nazwa_pliku;                    ///ustawienie nazwy koncowej scalonego pliku na podstawie pierszego pliku do scalenia
    getline(naz_plik, naz, '.');
    int j=0;
    while(j < naz.length()-3) {
        naz_sc += naz[j];
        j++;
    }
    naz_sc += ".txt";
    strcpy(nazwa_scalonego, naz_sc.c_str());
    plik_zapis.open(nazwa_scalonego, ios::out);
    ifstream plik_podz;
    char znak;
    string rozmiar_duzego, ile_plikow, nazwa_nastepnego, crc_duzego, dzielnik_crc;
    int l_plikow, roz_duz, crc_duz, roz_mal, ile_wczytac;
    unsigned short int crc_mal, crc_mal_obl;
    char nazwa_nas[100];
    int i=1;
    int liczba_wpisanych = 0;
    unsigned short int crc_obl;                 ///otworzenie kazdego pliku i skopiowanie z niego tresci(bez stopki i naglowka)
    do {
        if(i == 1) {
            plik_podz.open(nazwa_pliku, ios::in);
            if(plik_podz.fail()) {
                cout << "Podany plik nie istnieje!" << endl;
                return;
            }
        }
        else plik_podz.open(nazwa_nas, ios::in);
        plik_spr.open("plik_spr.txt", ios::out);
        roz_mal = podaj_rozmiar_pliku(plik_podz);
        stringstream tekst, liczba;
        getline(plik_podz, naglowek);           ///wyodrebnienie z naglowka odpowiednich informacji(rozmiar duzego pliku, nazwa nastepnego itp.)
        ile_wczytac = roz_mal - plik_podz.tellg() - 4;
        tekst << naglowek;
        getline(tekst, rozmiar_duzego, ' ');
        liczba << rozmiar_duzego;
        liczba >> roz_duz;
        getline(tekst, ile_plikow, ' ');
        liczba.clear();
        liczba << ile_plikow;
        liczba >> l_plikow;
        getline(tekst, nazwa_nastepnego, ' ');
        strcpy(nazwa_nas, nazwa_nastepnego.c_str());
        getline(tekst, crc_duzego, ' ');
        liczba.clear();
        liczba << crc_duzego;
        liczba >> crc_duz;
        getline(tekst, dzielnik_crc, ' ');
        liczba_wpisanych = 0;
        while(liczba_wpisanych <= ile_wczytac && plik_podz.get(znak)) {     ///kopiowanie znakow
            plik_zapis << znak;
            plik_spr << znak;
            liczba_wpisanych++;
        }
        plik_podz.get(znak);
        plik_podz.get(znak);
        crc_mal = (static_cast<int>(znak)) << 8;
        plik_podz.get(znak);
        crc_mal += static_cast<int>(znak);          ///policzenie crc mniejszego pliku i sprawdzenie czy zgadza on sie z podanym
        plik_podz.close();
        plik_spr.close();
        plik_spr.open("plik_spr.txt", ios::in);
        crc_mal_obl = oblicz_crc(tablica_crc, plik_spr, true, crc_mal);
        if(crc_mal_obl != 0) {
            cout << "Scalanie nie powiodlo sie!" << endl;
            cout << crc_mal_obl << endl;
            return;
        }
        plik_spr.close();
        remove("plik_spr.txt");
    } while (i++ < l_plikow);

    plik_zapis.close();                             ///policzenie crc duzego pliku i sprawdzenie czy jest on poprawny
    plik_zapis.open(nazwa_scalonego);
    crc_obl = oblicz_crc(tablica_crc, plik_zapis, true, crc_duz);
    plik_zapis.close();
    if(crc_obl == 0) cout << "Scalanie plikow przebieglo pomyslnie!" << endl;
    else {
        cout << "Scalanie nie powidlo sie!" << endl;
        remove(nazwa_scalonego);
    }
}

void wykonanie_polecenia(int argc, char *argv[])
{
    unsigned int tablica_crc[256];
    unsigned short int crc;
    int rozmiar_plikow;
    char nazwa_pliku[100];
    string opcja;
    fstream plik;
    tworzenie_tablicy_crc(tablica_crc);
    stringstream napis;
    napis << argv[1];
    napis >> opcja;
    if(opcja != "sc" && opcja != "dz") {
        cout << "Podana opcja nie jest poprawna! (mozliwe opcje: sc - scalanie plikow, dz - dzilenie pliku)" << endl;
    }
    if(argc == 4) {
        napis.clear();
        napis << argv[2];
        napis >> rozmiar_plikow;
    }
    if(argc == 4) {
        napis.clear();
        napis << argv[3];
        napis >> nazwa_pliku;
    } else if(argc == 3) {
        napis.clear();
        napis << argv[2];
        napis >> nazwa_pliku;
    }
    plik.close();
    if(opcja == "dz") {
        plik.open(nazwa_pliku, ios::in);
        crc = oblicz_crc(tablica_crc, plik, false);
        plik.clear();
        plik.seekg(0);
        plik.close();
        dzielenie_pliku(rozmiar_plikow, nazwa_pliku, tablica_crc, crc);
    } else if(opcja == "sc") {
        scalanie_pliku(nazwa_pliku, tablica_crc);
    }
}

int main(int argc, char *argv[])
{
    if(argc < 3) {
        cout << "Podano zla liczbe parametrow!" << endl;
    } else {
        wykonanie_polecenia(argc, argv);
    }
    return 0;
}

## Programowanie Systemowe
## Laboratorium nr 6 - Wywołania systemowe
### Magdalena Pastuła

Z tego laboratorium zostały wykonane zadania 3, 4 i 5. Aby móc skompilować pliki z testami z flagą `-static` należało zainstalować biblioteki glibc i libstdc do obsługi statycznego linkowania za pomocą komendy:
```
sudo dnf install glibc-static libstdc++-static -y;
```

### Zadanie 3 - wywołanie kernelps.

Program `test_kernelps` w przypadku braku argumentów wywołuje wywołanie systemowe z jednym argumentem, aby pozyskać liczbę procesów. Jeśli do programu został podany argument, to uznawany on jest za rozmiar tablicy, do której ma być wpisana lista procesów.

### Zadanie 4 - wywołanie freeblocks.

W przypadku uruchomienia programu `test_freeblocks` bez argumentów, program wywołuje wywołanie systemowe z argumentem wskazującym na ścieżkę, w której się znajduje.

### Zadanie 5 - wywołanie pidtoname.

W przypadku, gdy program `test_pidtoname` zostanie wykonany bez argumentów, jako argument wywołania systemowego podaway jest pid tego programu.



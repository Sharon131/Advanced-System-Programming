## Programowanie Systemowe
## Laboratorium nr 5 - Synchronizacja modułów
### Magdalena Pastuła

Dla modułu linked_buffer w kolejnych zadaniach synchronizacja została wykonana dla całego procesu zapisu do urządzenia, a nie tylko jednego bloku danych o wielkości 4 bajtów.

### Zadanie 1 - synchronizacja za pomocą semaforów.

Zadanie zostało wykonane zarówno dla modułu simple_sync jak i linked_buffer. W przypadku linked_buffer zwięszkono czas opóźnienia przy zapisie z 10 ms do 1000 ms.

### Zadanie 2 - synchronizacja za pomocą muteksów.

Zadanie zostało wykonane zarówno dla modułu simple_sync jak i linked_buffer. Dodatkowo, dla simple_module dodano opóźnienie 10 ms po odblokowaniu muteksa, aby drugi wątek czytający miał większą szansę na jego przejęcie.

### Zadanie 3 - synchoronizacja za pomocą mechanizmu spilock.

Zadanie zostało wykonane zarówno dla modułu simple_sync jak i linked_buffer.
Dla modułu simple_module oprócz dodanie spinlocku zmieniono również odrobinę kod, ponieważ spinlock nie powinien być założony, gdy funkcja zasypia. W związku z tym, wartość msg_pos jest pobierana i zapisywana do zmiennej lokalnej podczas założonego spinlocku, a po zdjęciu spinlocka wykorzystywana jest zapisana wartość.
Dla modułu linked_buffer użyto spinlocka na fragmencie kodu, w którym funkcja się usypia, aby możliwe było zsynchronizowanie całego procesu zapisu.

### Zadanie 4 - synchronizacja za pomocą RCU.

Zadanie nie zostało wykonane.

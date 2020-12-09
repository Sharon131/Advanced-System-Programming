## Programowanie Systemowe
## Laboratorium 4 - Debugowanie jądra
### Magdalena Pastuła

## Zadanie 1 - debugowanie modułów.

### Moduł nr 1

Poniżej znajduje się screen części informującej o błędzie przez Oops:

![Screen początku komunikatów opps](./module1_error.png)

Natomiast następny screen to dalszy ciąg komunikatów z Oops:

![Screen dalszej części komunikatów oops](./module1_stack_RIP.png)

Jak można zauważyć, błąd sygnalizowany przez Oops to problem z dostępem do strony w pamięci przestrzeni jądra. Na drugim screenie RIP podpowiada, że błąd wystąpił przy użyciu funkcji kfree pod adresem 0x170 w segmencie 0x53, a z zapisów Call trace można wyczytać, że błąd wystąpił w module broken_module w funkcji broken_read. Poniżej znajduje się kod funkcji, w której wystąpił błąd.

```C
ssize_t broken_read(struct file *filp, char *user_buf, size_t count,
	loff_t *f_pos)
{
	char *mybuf = NULL;
	int mybuf_size = 100;
	int len, err;

	mybuf = kmalloc(mybuf_size, GFP_KERNEL);
	if (!mybuf) {
		return -ENOMEM;
	}

	fill_buffer(mybuf, mybuf_size);

	len = strlen(mybuf);
	err = copy_to_user(user_buf, mybuf, len);
	kfree(user_buf);

	read_count++;

	if (!err && *f_pos == 0) {
		*f_pos += len;
		return len;
	}
	return 0;
}
```

Po przeanalizowaniu kodu okazuje się, że błędem jest próba zwolnienia pamięci z przestrzeni użytkownika, do której procesy z przestrzeni jądra nie mają bezpośredniego dostępu, co wyjaśnia problem z dostępem do pamięci zgłoszony w Oops. Po zmianie fragmentu `kfree(user_buf)` na `kfree(mybuf)` moduł już działa poprawnie i nie zgłasza żadnych błędów. Poniżej znajduje się screen komunikatów jądra po załadowaniu i usunięciu poprawionego modułu.

![Komunikaty jądra po poprawieniu błędu w module broken_module]()

### Moduł nr 2



### Moduł nr 3


### Moduł nr 4


## Zadanie 2 - GDB.


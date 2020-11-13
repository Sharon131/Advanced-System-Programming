## Programowanie Systemowe
## Laboratorium 2 - Wprowadzenie
### Magdalena Pastuła

Do wykonania ćwiczenia wykorzystano dostarczoną maszynę wirtualną z systemem Fedora w wersji 25 i  wersją kernela 4.9.12-200.fc25.x86_64.

### 1. LXR i dokumentacja.
1.1 Znajdź definicję struktury `file_operations`. Zapoznaj się z jej polami.

Znaleziona definicja struktury:
```C
struct file_operations {
    struct module *owner;
    loff_t (*llseek) (struct file *, loff_t, int);
    ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
    ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
    ssize_t (*read_iter) (struct kiocb *, struct iov_iter *);
    ssize_t (*write_iter) (struct kiocb *, struct iov_iter *);
    int (*iopoll)(struct kiocb *kiocb, bool spin);
    int (*iterate) (struct file *, struct dir_context *);
    int (*iterate_shared) (struct file *, struct dir_context *);
    __poll_t (*poll) (struct file *, struct poll_table_struct *);
    long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
    long (*compat_ioctl) (struct file *, unsigned int, unsigned long);
    int (*mmap) (struct file *, struct vm_area_struct *);
    unsigned long mmap_supported_flags;
    int (*open) (struct inode *, struct file *);
    int (*flush) (struct file *, fl_owner_t id);
    int (*release) (struct inode *, struct file *);
    int (*fsync) (struct file *, loff_t, loff_t, int datasync);
    int (*fasync) (int, struct file *, int);
    int (*lock) (struct file *, int, struct file_lock *);
    ssize_t (*sendpage) (struct file *, struct page *, int, size_t, loff_t *, int);
    unsigned long (*get_unmapped_area)(struct file *, unsigned long, unsigned long, unsigned long, unsigned long);
    int (*check_flags)(int);
    int (*flock) (struct file *, int, struct file_lock *);
    ssize_t (*splice_write)(struct pipe_inode_info *, struct file *, loff_t *, size_t, unsigned int);
    ssize_t (*splice_read)(struct file *, loff_t *, struct pipe_inode_info *, size_t, unsigned int);
    int (*setlease)(struct file *, long, struct file_lock **, void **);
    long (*fallocate)(struct file *file, int mode, loff_t offset,
                    loff_t len);
    void (*show_fdinfo)(struct seq_file *m, struct file *f);
#ifndef CONFIG_MMU
    unsigned (*mmap_capabilities)(struct file *);
#endif
    ssize_t (*copy_file_range)(struct file *, loff_t, struct file *,
                            loff_t, size_t, unsigned int);
    loff_t (*remap_file_range)(struct file *file_in, loff_t pos_in,
                            struct file *file_out, loff_t pos_out,
                            loff_t len, unsigned int remap_flags);
    int (*fadvise)(struct file *, loff_t, loff_t, int);
} __randomize_layout;
```

1.2 Spróbuj odnaleźć użycie tej struktury w systemie plików `hostfs`.

Znalezione użycie struktury `file_operations` w systemie plików `hostfs` w pliku `hostfs_kern.c`:
```C
static const struct file_operations hostfs_file_fops = {
        .llseek         = generic_file_llseek,
        .splice_read    = generic_file_splice_read,
        .read_iter      = generic_file_read_iter,
        .write_iter     = generic_file_write_iter,
        .mmap           = generic_file_mmap,
        .open           = hostfs_open,
        .release        = hostfs_file_release,
        .fsync          = hostfs_fsync,
};

static const struct file_operations hostfs_dir_fops = {
        .llseek         = generic_file_llseek,
        .iterate_shared = hostfs_readdir,
        .read           = generic_read_dir,
        .open           = hostfs_open,
        .fsync          = hostfs_fsync,
};
```

1.3 Czy potrafisz zinterpretować zastosowanie poszczególnych pól?

Poszczególne pola odpowiadają operacjom możliwym do wykonania na pliku (`hostfs_file_fops`)i katalogu (`hostfs_dir_fops`). Możliwe operacje to między innymi:
- llseek -> ustawienie wskaźnika w pliku
- read_iter -> czytanie z pliku
- write_iter -> pisanie do pliku
- open -> otwarcie pliku
- release -> zamknięcie pliku

1.4 Zlokalizuj w dokumentacji (katalog `Documentation` lub wyszukiwarka) informacje o tym, jak przechodzić po strukturze katalogów w jądrze.

Głównym plikiem opisującym strukturę katalogów w jądrze jest plik `/filesystems/path_lookup.txt`.

1.5 Zlokalizuj plik `namei.h` a w nim funkcje `kern_path` i `user_path`. Czym się one różnią i kiedy mają zastosowanie?

Definicja funkcji `kern_path`:
```C
int kern_path(const char *name, unsigned int flags, struct path *path)
{
    return filename_lookup(AT_FDCWD, getname_kernel(name),
                            flags, path, NULL);
}
```

Definicja funkcji `user_path`:
```C
static inline int user_path_at(int dfd, const char __user *name, 
                                unsigned flags, struct path *path)
{
    return user_path_at_empty(dfd, name, flags, path, NULL);
}
```

Funkcje `kern_path` oraz `user_path` wyszukują ścieżkę do pliku podanego w argumencie `name`. W tym celu funkcje te wyszukują strukturę `dentry`, która odpowiada danemu plikowi, a następnie zwracają tą strukturę poprzez argument `path`. Dodatkowo, zwiększany jest licznik odwołań do tych struktur.

Różnicą między tymi funkcjami jest środowisko: `kern_path` wyszukuje dla łańcucha znajdującego się w pamięci jądra, natomiast `user_path` dla łańcucha znajdujęcego się w pamięci użytkownika.

1.6 Znajdź definicję struktury `dentry`.

Znaleziona definicja struktury `dentry`:
```C
struct dentry {
    /* RCU lookup touched fields */
    unsigned int d_flags;           /* protected by d_lock */
    seqcount_t d_seq;               /* per dentry seqlock */
    struct hlist_bl_node d_hash;    /* lookup hash list */
    struct dentry *d_parent;        /* parent directory */
    struct qstr d_name;
    struct inode *d_inode;          /* Where the name belongs to - NULL is
                                     * negative */
    unsigned char d_iname[DNAME_INLINE_LEN];        /* small names */
    /* Ref lookup also touches following */
    struct lockref d_lockref;       /* per-dentry lock and refcount */
    const struct dentry_operations *d_op;
    struct super_block *d_sb;       /* The root of the dentry tree */
    unsigned long d_time;           /* used by d_revalidate */
    void *d_fsdata;                 /* fs-specific data */
    
    union {
            struct list_head d_lru;         /* LRU list */
            wait_queue_head_t *d_wait;      /* in-lookup ones only */
    };
    struct list_head d_child;       /* child of parent list */
    struct list_head d_subdirs;     /* our children */
    /*
     * d_alias and d_rcu can share memory
     */
    union {
            struct hlist_node d_alias;      /* inode alias list */
            struct hlist_bl_node d_in_lookup_hash;  /* only for in-lookup ones */
            struct rcu_head d_rcu;
    } d_u;
} __randomize_layout;
```
1.7 Co robi funkcja `dget` i po co?
Znaleziona definicja funkcji `dget`:
```C
/**
 *      dget, dget_dlock -      get a reference to a dentry
 *      @dentry: dentry to get a reference to
 *
 *      Given a dentry or %NULL pointer increment the reference count
 *      if appropriate and return the dentry. A dentry will not be 
 *      destroyed when it has references.
 */
static inline struct dentry *dget_dlock(struct dentry *dentry)
{
    if (dentry)
        dentry->d_lockref.count++;
    return dentry;
}

static inline struct dentry *dget(struct dentry *dentry)
{
    if (dentry)
        lockref_get(&dentry->d_lockref);
    return dentry;
}
```

Jak wskazuje opis, funkcja ta zwiększa liczbę odwołań do struktury. Dzięki nowym odwołaniom struktura `dentry` nie zostanie usunięta.

### 2. Kompilacja jadra.

2.1 Zapoznaj się z plikiem `.config`.

Plik opisuje oraz zawiera konfigurację jądra.

2.2 Wykonaj polecenie `make help`.

Po wykonaniu polecenia `make help` wypisuje się przewodnik po Makefile'a do budowy jądra.

2.3 Co robi polecenie `make oldconfig`?

Polecenie `make oldconfig` dokonuje aktualizacji obecnej konfiguracji korzystając z pliku `.config.

2.4 Co robi polecenie `make menuconfig`?

Polecenie `make menuconfig` wyświetla menu graficzne, za pomocą którego można uaktualnić konfigurację w pliku `.config`

2.5 Ustaw dowolną, ale charakterystyczną wersję lokalną (`CONFIG_LOCALVERSION` albo `General setup/Local version`).

Ustawiono `JanTrzeci`.

2.6 Zrób małą zmianę w konfiguracji (np. włącz obsługę któregoś systemu plików).

Dodano obsługę systemu plików F2FS.

2.7 Co robi polecenie `make all`?

Polecenie `make all` buduje wszystkie targety oznaczone gwiazdką, czyli `vmlinux` i `modules` i skompresowane jądro (`bzImage`).

2.8 Zmierz czas kompilacji jądra po modyfikacji. Jakie informacje się wyświetlają podczas kompilacji?

Podczas kompilacji w terminalu wypisują się kolejno kompilowane pliki i moduły.

Zmierzony czas kompilacji:
real	62m 38.067s
user	194m 32.753s
sys	    30m 9.810s

2.9 Co robią polecenia `make modules_install` i `make install`?

Polecenie `make modules_install` instaluje wszystkie moduły jądra, natomiast komenda `make install` instaluje jądro w aktualnym systemie.

2.10 Zainstaluj jądro w systemie.

2.11 Zrestartuj system i uruchom nowe jądro.

### 3. Moduł trivial_module.

3.2 Wylistuj moduły.
Wypisane moduły:

3.3 Komunikaty jądra:

3.5 Komunikaty jądra.

### 4. Uruchomianie jądra w QEMU.

### 5. Kompilacja jądra UML.

5.1 Wykonaj komendę `make ARCH=um defconfig`. Co ona robi?

5.2 Skompiluj jądro komendą `make ARCH=um`. Ile czasu zajęła kopilacja.

5.5 Zamontuj hostfs za pomocą komendy `mount none /host -t hostfs` Co pojawiło się po zamontowaniu systemu plików?

5.6 Jakie procesy są widoczne w gościu i po stronie hosta?
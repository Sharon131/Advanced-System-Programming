## Programowanie Systemowe
## Laboratorium 2 - Wprowadzenie
### Magdalena Pastuła

Do wykonania ćwiczenia wykorzystano dostarczoną maszynę wirtualną z systemem Fedora w wersji 25, wersja kernela: 4.9.12-200.fc25.x86_64.

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


1.4 Zlokalizuj w dokumentacji (katalog `Documentation` lub wyszukiwarka) informacje o tym, jak przechodzić po strukturze katalogów w jądrze.

1.5 Zlokalizuj plik `namei.h` a w nim funkcje `kern_path` i `user_path`. Czym się one różnią i kiedy mają zastosowanie?

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
static inline struct dentry *dget(struct dentry *dentry)
{
    if (dentry)
        lockref_get(&dentry->d_lockref);
    return dentry;
}
```
### 2. Kompilacja jadra.

2.3 Co robi polecenie `make oldconfig`?
`make oldconfig` -> Update current config utilising a provided .config as base. Aktualizacja obecnej konfiguracji bazująca na pliku .config.

2.4 Co robi polecenie `make menuconfig`?

`make menuconfig` -> Update current config utilising a menu based program. Aktualizacja obecnej konfiguracji wykorzystując program oparty o menu.

2.7 Co robi polecenie `make all`?

`make all` -> Build all targets marked with [*]
Budowanie wszystkich targetów oznaczonych gwiazdką. Między innymi: `vmlinux` (sam kernel), `modules` (wszystkie moduły).

2.8 Zmierz czas kompilacji jądra po modyfikacji. Jakie informacje się wyświetlają podczas kompilacji?

2.9 Co robią polecenia `make modules_install` i `make install`?

`make modules_install` -> Install all modules to INSTALL_MOD_PATH (default: /). Instalacja wszystkich modułów pod ścieżką INSTALL_MOD_PATH. Domyślnie instalują się pod ścieżką `/`

`make install` -> Install kernel using
                  (your) ~/bin/installkernel or
                  (distribution) /sbin/installkernel or
                  install to $(INSTALL_PATH) and run lilo


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
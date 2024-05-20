# OS semogakelar
> by semogakelar
> Sistem operasi yang akan dibuat akan berjalan pada arsitektur x86 32-bit

## Daftar Isi
- [Deskripsi Singkat](#deskripsi-singkat)
- [Requirements](#requirements)
- [Cara Mengkompilasi dan Menjalankan Program](#cara-mengkompilasi-dan-menjalankan-program)
- [Author](#author)

## Deskripsi Singkat
Tugas Besar ini bertujuan membuat sistem operasi yang berjalan pada arsitektur x86 32-bit.<br>
Sistem operasi akan dijalankan dengan operator QEMU.<br>
Tugas terbagi menjadi empat milestone sebagai berikut:

### Milestone 0 - Toolchain, Kernel, GDT
1. Menyiapkan alat dan repository
2. Kernel dasar
3. Otomasi build
4. GDT

### Milestone 1 - Interrupt, Driver, dan Filesystem
1. Text Framebuffer
2. Intterupt
3. Keyboard driver
4. File System FAT32

### Milestone 2 - Paging, User Mode, dan Shell
1. Paging
2. User Mode
3. Shell

### Milestone 3 - Process, Scheduler, Multitasking
1. Process Control Block
2. Scheduler dan context switch
3. Multitask

## Requirements
- GCC compiler (versi 11.2.0 atau yang lebih baru)
- Visual Studio Code
- Windows Subsystem for Linux (WSL2) dengan distribusi minimal Ubuntu 20.04
- Emulator QEMU

## Cara Mengkompilasi dan Menjalankan Program
1. Lakukan *clone repository* melalui terminal dengan *command* berikut
    ``` bash
    $ git clone https://github.com/labsister21/os-2024-semogakelar.git
    ```
2. Lakukan eksekusi pada makefile dengan memasukkan *command* `make run`.

Jika berhasil, maka sistem operasi akan muncul pada layar.

## Author
| NIM      | Nama                       | Github Profile                              |
| -------- | ---------------------------|---------------------------------------------|
| 13522007 | Irfan Sidiq Permana        | [IrfanSidiq](https://github.com/IrfanSidiq) |
| 13522033 | Bryan Cornelius Lauwrence  | [BryanLauw](https://github.com/BryanLauw)   |
| 13522041 | Ahmad Hasan Albana         | [Bana-man](https://github.com/Bana-man)     |
| 13522101 | Abdullah Mubarak           | [b33rk](https://github.com/b33rk)           |
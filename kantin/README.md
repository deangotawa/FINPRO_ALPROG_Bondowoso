# Aplikasi Pemesanan Kantin UI
### Tugas Akhir — Algoritma dan Pemrograman | Teknik Komputer UI

---

## Anggota Kelompok
- (isi nama anggota)

---

## Deskripsi
Aplikasi client-server berbasis terminal untuk sistem pemesanan makanan di kantin UI.
Mahasiswa (client) dapat melihat menu, mencari, memesan, sementara kasir (juga client)
dapat melihat antrian pesanan dan mengupdate statusnya.

---

## Struktur File
```
kantin/
├── MenuItem.h          # Abstract class + Food + Drink (OOP)
├── MenuLinkedList.h    # Linked List manual (Bonus)
├── Algorithms.h        # QuickSort, MergeSort, BinarySearch, LinearSearch
├── Order.h             # Class Order & OrderItem
├── JsonHelper.h        # JSON builder & parser
├── server.cpp          # Server multithreaded
├── client.cpp          # Client aplikasi
└── Makefile
```

---

## Cara Kompilasi & Menjalankan

### Kompilasi
```bash
make
```

### Jalankan Server (Terminal 1)
```bash
./server
```

### Jalankan Client (Terminal 2, 3, dst)
```bash
./client              # jika server di localhost
./client 192.168.x.x  # jika server di IP lain (LAN)
```

---

## Requirement Terpenuhi

### Struktur Data
- **Linked List Manual** (`MenuLinkedList.h`) — tidak menggunakan `std::list`/`std::vector` untuk penyimpanan menu utama
- `std::vector` digunakan hanya untuk operasi sorting sementara

### Algoritma Searching
| Algoritma | Fungsi | Big O |
|---|---|---|
| Linear Search | Cari menu by nama | O(n) |
| Binary Search | Cari menu by ID (setelah sort) | O(log n) |

### Algoritma Sorting
| Algoritma | Fungsi | Big O |
|---|---|---|
| Quick Sort | Urutkan menu by harga | Best/Avg: O(n log n), Worst: O(n²) |
| Merge Sort | Urutkan menu by nama | All cases: O(n log n) |

### OOP — 4 Pilar
| Pilar | Implementasi |
|---|---|
| **Abstraksi** | `MenuItem` adalah abstract class dengan pure virtual method `getCategory()` dan `getDescription()` |
| **Enkapsulasi** | Semua field `private`/`protected`, akses via getter/setter |
| **Pewarisan** | `Food` dan `Drink` extends `MenuItem` |
| **Polimorfisme** | `getDescription()` di-override berbeda di `Food` vs `Drink` |

### Networking
- Socket TCP/IP antara `client.cpp` dan `server.cpp`
- Port: 8080
- Server menerima banyak client sekaligus (multithreading)

### JSON
- Semua komunikasi client ↔ server menggunakan format JSON
- Contoh request: `{"type":"ORDER","customer":"Budi","items":[{"menuId":1,"quantity":2}]}`
- Contoh response: `{"status":"SUCCESS","orderId":1001,"total":30000,...}`

---

## Bonus Terpenuhi

- ✅ **Linked List Manual** — `MenuLinkedList.h` tanpa `std::list`/`std::vector`
- ✅ **Analisis Big O** — tercantum di `Algorithms.h` dan tabel di atas
- ✅ **Multithreading** — setiap client di-handle thread terpisah dengan `mutex` untuk thread-safety

---

## Data Menu

### Warung Pak Budi (Makanan Berat)
| ID | Menu | Harga |
|---|---|---|
| 1 | Nasi Goreng Spesial | Rp15.000 |
| 2 | Nasi Ayam Geprek | Rp18.000 |
| 3 | Nasi Rendang | Rp20.000 |
| 4 | Mie Goreng Telur | Rp13.000 |
| 5 | Nasi Uduk Komplit | Rp16.000 |

### Warung Bu Sari (Makanan Ringan)
| ID | Menu | Harga |
|---|---|---|
| 6 | Gado-Gado | Rp14.000 |
| 7 | Ketoprak | Rp12.000 |
| 8 | Siomay Bandung | Rp13.000 |
| 9 | Batagor Crispy | Rp12.000 |
| 10 | Lontong Sayur | Rp11.000 |

### Warung Bu Dewi (Minuman & Dessert)
| ID | Menu | Harga |
|---|---|---|
| 11 | Es Teh Manis | Rp5.000 |
| 12 | Es Jeruk | Rp7.000 |
| 13 | Jus Alpukat | Rp12.000 |
| 14 | Kopi Susu Kekinian | Rp12.000 |
| 15 | Teh Tarik Panas | Rp8.000 |
| 16 | Es Pisang Ijo | Rp10.000 |
| 17 | Puding Coklat | Rp8.000 |

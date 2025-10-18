#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <windows.h> // Windows API'si için gerekli
#include "sqlite3.h"

// === YENİ RENK TANIMLAMALARI (Windows API için) ===
// Bu sayılar Windows'un kendi renk kodlarıdır.
#define RENK_VARSAYILAN 7  // Beyaz
#define RENK_BASLIK     11 // Parlak Turkuaz
#define RENK_BASARI     10 // Parlak Yeşil
#define RENK_HATA       12 // Parlak Kırmızı
#define RENK_UYARI      14 // Parlak Sarı
#define RENK_BILGI      9  // Parlak Mavi
#define RENK_BOLD_BEYAZ 15 // Parlak Beyaz (Kalın gibi)

// Rengi değiştirmek için kullanacağımız yardımcı fonksiyon
void renk_ayarla(int renk_kodu) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), renk_kodu);
}

// === Fonksiyon Bildirimleri (Prototypes) ===
void menuyu_goster();
void raporlar_menusu(sqlite3 *db);
void yonetim_menusu(sqlite3 *db);
void islem_guncelle(sqlite3 *db);
int sayi_al();
void devam_etmek_icin_bekle();

// === Callback ve Giriş Fonksiyonları ===
int listeleme_callback(void *data, int sutun_sayisi, char **sutun_verileri, char **sutun_adlari) {
    for (int i = 0; i < sutun_sayisi; i++) { printf("%-20s", sutun_verileri[i] ? sutun_verileri[i] : "NULL"); }
    printf("\n"); return 0;
}
int toplam_callback(void *data, int sutun_sayisi, char **sutun_verileri, char **sutun_adlari) {
    double *toplam = (double *)data; if (sutun_verileri[0]) { *toplam = atof(sutun_verileri[0]); }
    return 0;
}
int sayi_al() {
    char girdi[100]; int sayi;
    while (1) {
        if (fgets(girdi, sizeof(girdi), stdin) != NULL) {
            if (sscanf(girdi, "%d", &sayi) == 1) { return sayi; }
        }
        renk_ayarla(RENK_UYARI); printf("Gecersiz giris! Lutfen bir SAYI girin: "); renk_ayarla(RENK_VARSAYILAN);
    }
}
void devam_etmek_icin_bekle() {
    renk_ayarla(RENK_UYARI); printf("\nDevam etmek icin Enter'a basin..."); renk_ayarla(RENK_VARSAYILAN);
    getchar();
}

// === ANA FONKSİYONLAR ===
void kategorileri_listele(sqlite3 *db) {
    const char *sql = "SELECT ID, AD, TUR FROM KATEGORILER ORDER BY ID;";
    renk_ayarla(RENK_BILGI); printf("\n--- MEVCUT KATEGORILER ---\n");
    renk_ayarla(RENK_BOLD_BEYAZ); printf("%-20s%-20s%-20s\n", "ID", "AD", "TUR");
    renk_ayarla(RENK_BILGI); printf("------------------------------------------------------------\n");
    renk_ayarla(RENK_VARSAYILAN);
    sqlite3_exec(db, sql, listeleme_callback, 0, NULL);
    renk_ayarla(RENK_BILGI); printf("------------------------------------------------------------\n\n");
    renk_ayarla(RENK_VARSAYILAN);
}

void kategori_ekle(sqlite3 *db) {
    char ad[100], tur[20], sql[256], *hata_mesaji = 0;
    printf("Eklenecek kategorinin adini girin: ");
    fgets(ad, sizeof(ad), stdin); ad[strcspn(ad, "\n")] = 0;
    printf("Kategorinin turunu girin (gelir/gider): ");
    fgets(tur, sizeof(tur), stdin); tur[strcspn(tur, "\n")] = 0;
    sprintf(sql, "INSERT INTO KATEGORILER (AD, TUR) VALUES('%s', '%s');", ad, tur);
    if (sqlite3_exec(db, sql, 0, 0, &hata_mesaji) != SQLITE_OK) {
        renk_ayarla(RENK_HATA); fprintf(stderr, "Hata: %s\n", hata_mesaji); renk_ayarla(RENK_VARSAYILAN);
        sqlite3_free(hata_mesaji);
    } else {
        renk_ayarla(RENK_BASARI); printf("\nKategori basariyla eklendi!\n"); renk_ayarla(RENK_VARSAYILAN);
    }
}

void islemleri_listele(sqlite3 *db) {
    const char *sql = "SELECT I.ID, K.AD, I.MIKTAR, I.ACIKLAMA, I.TARIH FROM ISLEMLER AS I JOIN KATEGORILER AS K ON I.KATEGORI_ID = K.ID ORDER BY I.TARIH DESC, I.ID DESC;";
    renk_ayarla(RENK_BILGI); printf("\n--- TUM ISLEMLER ---\n");
    renk_ayarla(RENK_BOLD_BEYAZ); printf("%-20s%-20s%-20s%-20s%-20s\n", "ISLEM_ID", "KATEGORI", "MIKTAR", "ACIKLAMA", "TARIH");
    renk_ayarla(RENK_BILGI); printf("----------------------------------------------------------------------------------------------------\n");
    renk_ayarla(RENK_VARSAYILAN);
    sqlite3_exec(db, sql, listeleme_callback, 0, NULL);
    renk_ayarla(RENK_BILGI); printf("----------------------------------------------------------------------------------------------------\n");
    renk_ayarla(RENK_VARSAYILAN);
}

void islem_ekle(sqlite3 *db) {
    int kategori_id; double miktar; char aciklama[200], tarih[20], sql[512], girdi_buffer[256], *hata_mesaji = 0;
    kategorileri_listele(db);
    printf("Islem yapilacak kategori ID'sini secin: "); kategori_id = sayi_al();
    printf("Islem miktarini girin: ");
    while (1) {
        fgets(girdi_buffer, sizeof(girdi_buffer), stdin);
        if (sscanf(girdi_buffer, "%lf", &miktar) == 1 && miktar >= 0) { break; }
        renk_ayarla(RENK_UYARI); printf("Gecersiz veya negatif miktar! Lutfen pozitif bir sayi girin: "); renk_ayarla(RENK_VARSAYILAN);
    }
    printf("Aciklama girin (Orn: Aksam Yemegi): ");
    fgets(aciklama, sizeof(aciklama), stdin); aciklama[strcspn(aciklama, "\n")] = 0;
    time_t t = time(NULL); struct tm tm = *localtime(&t);
    sprintf(tarih, "%d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    sprintf(sql, "INSERT INTO ISLEMLER (KATEGORI_ID, MIKTAR, ACIKLAMA, TARIH) VALUES(%d, %.2f, '%s', '%s');", kategori_id, miktar, aciklama, tarih);
    if (sqlite3_exec(db, sql, 0, 0, &hata_mesaji) != SQLITE_OK) {
        renk_ayarla(RENK_HATA); fprintf(stderr, "Hata: %s\n", hata_mesaji); renk_ayarla(RENK_VARSAYILAN);
        sqlite3_free(hata_mesaji);
    } else {
        renk_ayarla(RENK_BASARI); printf("\nIslem basariyla kaydedildi!\n"); renk_ayarla(RENK_VARSAYILAN);
    }
}

void islem_guncelle(sqlite3 *db) {
    int islem_id; double yeni_miktar; char yeni_aciklama[200], sql[512], girdi_buffer[256], *hata_mesaji = 0;
    islemleri_listele(db);
    printf("Guncellemek istediginiz ISLEM ID'sini girin: "); islem_id = sayi_al();
    printf("Yeni miktari girin: ");
    while (1) {
        fgets(girdi_buffer, sizeof(girdi_buffer), stdin);
        if (sscanf(girdi_buffer, "%lf", &yeni_miktar) == 1 && yeni_miktar >= 0) { break; }
        renk_ayarla(RENK_UYARI); printf("Gecersiz veya negatif miktar! Lutfen pozitif bir sayi girin: "); renk_ayarla(RENK_VARSAYILAN);
    }
    printf("Yeni aciklamayi girin: ");
    fgets(yeni_aciklama, sizeof(yeni_aciklama), stdin); yeni_aciklama[strcspn(yeni_aciklama, "\n")] = 0;
    sprintf(sql, "UPDATE ISLEMLER SET MIKTAR = %.2f, ACIKLAMA = '%s' WHERE ID = %d;", yeni_miktar, yeni_aciklama, islem_id);
    if (sqlite3_exec(db, sql, 0, 0, &hata_mesaji) != SQLITE_OK) {
        renk_ayarla(RENK_HATA); fprintf(stderr, "Hata: Islem guncellenemedi: %s\n", hata_mesaji); renk_ayarla(RENK_VARSAYILAN);
        sqlite3_free(hata_mesaji);
    } else {
        renk_ayarla(RENK_BASARI); printf("\nIslem %d basariyla guncellendi!\n", islem_id); renk_ayarla(RENK_VARSAYILAN);
    }
}

void islem_sil(sqlite3 *db) {
    int islem_id; char sql[100], *hata_mesaji = 0;
    islemleri_listele(db);
    printf("Silmek istediginiz ISLEM ID'sini girin: "); islem_id = sayi_al();
    sprintf(sql, "DELETE FROM ISLEMLER WHERE ID = %d;", islem_id);
    if (sqlite3_exec(db, sql, 0, 0, &hata_mesaji) != SQLITE_OK) {
        renk_ayarla(RENK_HATA); fprintf(stderr, "Hata: %s\n", hata_mesaji); renk_ayarla(RENK_VARSAYILAN);
        sqlite3_free(hata_mesaji);
    } else {
        renk_ayarla(RENK_BASARI); printf("\nIslem basariyla silindi!\n"); renk_ayarla(RENK_VARSAYILAN);
    }
}

void kategori_sil(sqlite3 *db) {
    int kategori_id; char sql[100], *hata_mesaji = 0;
    kategorileri_listele(db);
    renk_ayarla(RENK_UYARI); printf("UYARI: Bir kategoriyi silmek, o kategoriye ait tum islemleri de siler!\n"); renk_ayarla(RENK_VARSAYILAN);
    printf("Silmek istediginiz KATEGORI ID'sini girin: "); kategori_id = sayi_al();
    sprintf(sql, "DELETE FROM KATEGORILER WHERE ID = %d;", kategori_id);
    if (sqlite3_exec(db, sql, 0, 0, &hata_mesaji) != SQLITE_OK) {
        renk_ayarla(RENK_HATA); fprintf(stderr, "Hata: %s\n", hata_mesaji); renk_ayarla(RENK_VARSAYILAN);
        sqlite3_free(hata_mesaji);
    } else {
        renk_ayarla(RENK_BASARI); printf("\nKategori ve iliskili islemler basariyla silindi!\n"); renk_ayarla(RENK_VARSAYILAN);
    }
}

void aylik_rapor(sqlite3 *db) {
    int yil, ay; double toplam_harcama = 0; char sql[256], tarih_filtre[8], *hata_mesaji = 0;
    printf("Raporlamak istediginiz yili girin (örn: 2025): "); yil = sayi_al();
    printf("Raporlamak istediginiz ayi girin (1-12): "); ay = sayi_al();
    sprintf(tarih_filtre, "%d-%02d", yil, ay);
    sprintf(sql, "SELECT SUM(I.MIKTAR) FROM ISLEMLER AS I JOIN KATEGORILER AS K ON I.KATEGORI_ID = K.ID WHERE K.TUR = 'gider' AND strftime('%%Y-%%m', I.TARIH) = '%s';", tarih_filtre);
    if (sqlite3_exec(db, sql, toplam_callback, &toplam_harcama, &hata_mesaji) != SQLITE_OK) {
        renk_ayarla(RENK_HATA); fprintf(stderr, "Hata: %s\n", hata_mesaji); renk_ayarla(RENK_VARSAYILAN);
        sqlite3_free(hata_mesaji);
    } else {
        renk_ayarla(RENK_BASARI); printf("\n>> %s donemi icin toplam harcama: %.2f TL\n", tarih_filtre, toplam_harcama); renk_ayarla(RENK_VARSAYILAN);
    }
}

void kategori_dokumu(sqlite3 *db) {
    const char *sql = "SELECT K.AD, SUM(I.MIKTAR) FROM ISLEMLER AS I JOIN KATEGORILER AS K ON I.KATEGORI_ID = K.ID WHERE K.TUR = 'gider' GROUP BY K.AD ORDER BY SUM(I.MIKTAR) DESC;";
    renk_ayarla(RENK_BILGI); printf("\n--- KATEGORILERE GORE HARCAMA DOKUMU ---\n");
    renk_ayarla(RENK_BOLD_BEYAZ); printf("%-20s%-20s\n", "KATEGORI", "TOPLAM HARCAMA (TL)");
    renk_ayarla(RENK_BILGI); printf("----------------------------------------\n");
    renk_ayarla(RENK_VARSAYILAN);
    sqlite3_exec(db, sql, listeleme_callback, 0, NULL);
    renk_ayarla(RENK_BILGI); printf("----------------------------------------\n");
    renk_ayarla(RENK_VARSAYILAN);
}

// === MENÜLER ===
void yonetim_menusu(sqlite3 *db) {
    int secim = -1;
    while (secim != 0) {
        system("cls");
        renk_ayarla(RENK_BASLIK); printf("\n--- YONETIM MENUSU ---\n"); renk_ayarla(RENK_VARSAYILAN);
        printf("1. Islem Sil\n2. Kategori Sil\n");
        renk_ayarla(RENK_UYARI); printf("0. Ana Menuye Don\n"); renk_ayarla(RENK_VARSAYILAN);
        printf("Seciminiz: ");
        secim = sayi_al();
        switch(secim) {
            case 1: islem_sil(db); devam_etmek_icin_bekle(); break;
            case 2: kategori_sil(db); devam_etmek_icin_bekle(); break;
            case 0: break;
            default: renk_ayarla(RENK_UYARI); printf("Gecersiz secim!\n"); renk_ayarla(RENK_VARSAYILAN); devam_etmek_icin_bekle(); break;
        }
    }
}

void raporlar_menusu(sqlite3 *db) {
    int secim = -1;
    while (secim != 0) {
        system("cls");
        renk_ayarla(RENK_BASLIK); printf("\n--- RAPORLAR MENUSU ---\n"); renk_ayarla(RENK_VARSAYILAN);
        printf("1. Aylik Harcama Raporu\n2. Kategoriye Gore Harcama Dokumu\n");
        renk_ayarla(RENK_UYARI); printf("0. Ana Menuye Don\n"); renk_ayarla(RENK_VARSAYILAN);
        printf("Seciminiz: ");
        secim = sayi_al();
        switch(secim) {
            case 1: aylik_rapor(db); devam_etmek_icin_bekle(); break;
            case 2: kategori_dokumu(db); devam_etmek_icin_bekle(); break;
            case 0: break;
            default: renk_ayarla(RENK_UYARI); printf("Gecersiz secim!\n"); renk_ayarla(RENK_VARSAYILAN); devam_etmek_icin_bekle(); break;
        }
    }
}

void menuyu_goster() {
    renk_ayarla(RENK_BASLIK); printf("===== KISISEL FINANS TAKIP UYGULAMASI v3.1 (Final) =====\n");
    renk_ayarla(RENK_VARSAYILAN);
    printf("1. Yeni Islem Ekle\n2. Bir Islemi Guncelle\n3. Tum Islemleri Listele\n4. Kategori Ekle\n5. Kategorileri Listele\n6. Yonetim (Silme Islemleri)\n7. Raporlar\n");
    renk_ayarla(RENK_UYARI); printf("0. Cikis\n");
    renk_ayarla(RENK_BASLIK); printf("========================================================\n");
    renk_ayarla(RENK_VARSAYILAN);
    printf("Seciminiz: ");
}

// === ANA PROGRAM ===
int main() {
    sqlite3 *db; int secim = -1;
    
    // ASCII ART AÇILIŞ EKRANI
    system("cls");
    renk_ayarla(RENK_BASARI);
    printf("\n\n");
    printf("                 _._\n");
    printf("              .-'   `'-.\n");
    printf("             /           \\\n");
    printf("            |      $      |\n");
    printf("             \\           /\n");
    printf("              `._ _ _ _.'\n");
    printf("\n\n");
    renk_ayarla(RENK_VARSAYILAN);
    printf("        Finans Takip Asistaniniza Hos Geldiniz!\n");
    Sleep(2500);

    if (sqlite3_open("finans.db", &db) != SQLITE_OK) { return 1; }
    
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", 0, 0, 0);
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS KATEGORILER(ID INTEGER PRIMARY KEY, AD TEXT UNIQUE NOT NULL, TUR TEXT NOT NULL);", 0, 0, 0);
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS ISLEMLER(ID INTEGER PRIMARY KEY, KATEGORI_ID INTEGER, MIKTAR REAL NOT NULL, ACIKLAMA TEXT, TARIH TEXT, FOREIGN KEY(KATEGORI_ID) REFERENCES KATEGORILER(ID) ON DELETE CASCADE);", 0, 0, 0);

    while (1) {
        system("cls");
        menuyu_goster();
        secim = sayi_al();
        
        switch (secim) {
            case 1: islem_ekle(db); devam_etmek_icin_bekle(); break;
            case 2: islem_guncelle(db); devam_etmek_icin_bekle(); break;
            case 3: islemleri_listele(db); devam_etmek_icin_bekle(); break;
            case 4: kategori_ekle(db); devam_etmek_icin_bekle(); break;
            case 5: kategorileri_listele(db); devam_etmek_icin_bekle(); break;
            case 6: yonetim_menusu(db); break;
            case 7: raporlar_menusu(db); break;
            case 0: renk_ayarla(RENK_UYARI); printf("Programdan cikiliyor...\n"); renk_ayarla(RENK_VARSAYILAN); sqlite3_close(db); return 0;
            default: renk_ayarla(RENK_UYARI); printf("Gecersiz secim! Lutfen tekrar deneyin.\n"); renk_ayarla(RENK_VARSAYILAN); devam_etmek_icin_bekle(); break;
        }
    }
    return 0;
}

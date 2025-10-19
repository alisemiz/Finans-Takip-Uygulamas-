# C ile Finans Takip Uygulaması

Bu proje, C dili ve SQLite veritabanı kullanılarak geliştirilmiş, komut satırı tabanlı bir kişisel bütçe yönetimi uygulamasıdır.

Uygulama, kullanıcıların gelir ve giderlerini kategorilere ayırarak kaydetmesine, bu kayıtları yönetmesine (ekleme, güncelleme, silme) ve temel finansal raporlar almasına olanak tanır. Proje, sağlam hata yönetimi ve renkli arayüzü ile kullanıcı dostu bir deneyim sunmayı hedefler.

## Temel Özellikler

- **Kategori Yönetimi:** Gelir ve giderler için özel kategoriler oluşturun, listeleyin ve silin.
- **İşlem Yönetimi:** Harcamalarınızı ve gelirlerinizi tarih, miktar ve açıklama ile kaydedin.
- **Kayıt Güncelleme:** Mevcut bir işlemin bilgilerini kolayca düzeltin.
- **Kayıt Silme:** Artık ihtiyaç duymadığınız işlemleri veya kategorileri güvenle silin.
- **Raporlama:** Belirttiğiniz aya ait toplam harcamayı veya harcamaların kategorilere göre dökümünü alın.
- **Sağlam Arayüz:** Sayısal alanlara metin girilmesi gibi kullanıcı hatalarına karşı çökmeyi engelleyen bir yapıya sahiptir.

## Kurulum ve Çalıştırma

### Gereksinimler

- Bir C derleyicisi (Örn: **MinGW/GCC** Windows için)
- **Git**

### Adımlar

1.  **Depoyu Klonlayın:**

    ```bash
    git clone [https://github.com/alisemiz/Finans-Takip-Uygulamas-.git]
    ```

2.  **Klasöre Gidin:**

    ```bash
    cd Finans-Takip-Uygulamas-
    ```

3.  **Derleyin:**

    ```bash
    gcc main.c sqlite3.c -o finans_takip.exe
    ```

4.  **Çalıştırın:**
    ```bash
    .\finans_takip.exe
    ```

## Kullanılan Teknolojiler

- **Dil:** C
- **Veritabanı:** SQLite
- **Derleyici:** GCC / MinGW

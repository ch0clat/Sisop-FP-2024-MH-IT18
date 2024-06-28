# Sisop-FP-2024-MH-IT18

## Anggota kelompok
   ### Callista Meyra Azizah 5027231060
   ### Abhirama Triadyatma Hermawan 5027231061
   ### Adi Satria Pangestu 5027231043

A. Autentikasi (Register dan Login)
![image](https://github.com/ch0clat/Sisop-FP-2024-MH-IT18/assets/151893499/e1b7e921-f6a6-48ac-a1de-29678bd6f00a)

B. Penggunaan DiscorIT
1. List Channel dan Room
![image](https://github.com/ch0clat/Sisop-FP-2024-MH-IT18/assets/151893499/d77fe60a-23ea-4ee6-8a89-2bd1cc0a142e)
(masukin gambar list room)
3. Akses Channel dan Room
(masukin gambar)
4. Fitur Chat
(masukin gambar)

C. Root
Akun yang pertama kali mendaftar otomatis mendapatkan peran "root".
Root dapat masuk ke channel manapun tanpa key dan create, update, dan delete pada channel dan room, mirip dengan admin [D].
Root memiliki kemampuan khusus untuk mengelola user, seperti: list, edit, dan Remove.
![image](https://github.com/ch0clat/Sisop-FP-2024-MH-IT18/assets/151893499/6132febb-22e5-49f9-a92c-35ac56f8c262)

D. Admin Channel
Setiap user yang membuat channel otomatis menjadi admin di channel tersebut. Informasi tentang user disimpan dalam file auth.csv.
Admin dapat create, update, dan delete pada channel dan room, serta dapat remove, ban, dan unban user di channel mereka.
![image](https://github.com/ch0clat/Sisop-FP-2024-MH-IT18/assets/151893499/eb5e68d1-d465-4941-89ac-09482a774a31)
1. Channel
Informasi tentang semua channel disimpan dalam file channel.csv. Semua perubahan dan aktivitas user pada channel dicatat dalam file users.log.
CREATE CHANNEL
![image](https://github.com/ch0clat/Sisop-FP-2024-MH-IT18/assets/151893499/884db7d7-eaeb-4cf5-b863-68a31a99af03)
EDIT CHANNEL
(masukin gambar)
DELETE CHANNEL
(masukin gambar)
2. Room
Semua perubahan dan aktivitas user pada room dicatat dalam file users.log.
CREATE ROOM
(masukin gambar)
EDIT ROOM
(masukin gambar)
DELETE ROOM
(masukin gambar)
3. Ban 
Admin dapat melakukan ban pada user yang nakal. Aktivitas ban tercatat pada users.log. Ketika di ban, role "user" berubah menjadi "banned". Data tetap tersimpan dan user tidak dapat masuk ke dalam channel.
(masukin gambar)
4. Unban 
Admin dapat melakukan unban pada user yang sudah berperilaku baik. Aktivitas unban tercatat pada users.log. Ketika di unban, role "banned" berubah kembali menjadi "user" dan dapat masuk ke dalam channel.
(masukin gambar)
5. Remove user
Admin dapat remove user dan tercatat pada users.log.
(masukin gambar)
E. User
User dapat mengubah informasi profil mereka, user yang di ban tidak dapat masuk kedalam channel dan dapat keluar dari room, channel, atau keluar sepenuhnya dari DiscorIT.
1. Edit User Username
(masukin gambar)
2. Edit User Password
(masukin gambar)
3. Banned User
(masukin gambar)
4. Exit
(masukin gambar)

F. Error Handling
Jika ada command yang tidak sesuai penggunaannya. Maka akan mengeluarkan pesan error dan tanpa keluar dari program client.
(masukin gambar)

G. Monitor
User dapat menampilkan isi chat secara real-time menggunakan program monitor. Jika ada perubahan pada isi chat, perubahan tersebut akan langsung ditampilkan di terminal.
Sebelum dapat menggunakan monitor, pengguna harus login terlebih dahulu dengan cara yang mirip seperti login di DiscorIT.
Untuk keluar dari room dan menghentikan program monitor dengan perintah "EXIT".
Monitor dapat digunakan untuk menampilkan semua chat pada room, mulai dari chat pertama hingga chat yang akan datang nantinya.
(masukin gambar list room)



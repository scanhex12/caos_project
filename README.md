# Сервер удалённого исполнения

Сделал базовую часть

Пример использования:

Client:
```c
gcc -o server server.c && ./server 1234
```

Server:
```c
gcc -o client client.c && ./client 0.0.0.0:1234 spawn ls 
```

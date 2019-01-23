# SyncCinemaNet_Server Video Server Structure:
## Базовые интерфейсы:
В директории interfaces содержатся исходники базовых интерфейсов для работы сервера.
### tcp.h
Файл отвечает за создание и работу TCP-сервера
* Константа LOCAL означает работу только на localhost (можно использовать для тестирования и запуска в Docker), INET означает работу на публичном IP
* int tcp_create(int net, int port, int size): функция создания TCP-сокета, первый аргумент отвечает за выбор сети - LOCAL или INET, второй - номер порта, третий - максимально количество клиентов в очереди
* int tcp_server(struct epoll_event \*event, int net, int port): функция запуска ожидания клиентов, первый аргумент - структура для работы epoll, net и port - тоже самое, что и для tcp_create
### errors.h
Содержит функции обработки ошибок сервера
* Список собственных кодов ошибок:
  - SE_DEV - определённая функция в проекте ещё не проработана
* void die_err(int err): Закончить работу сервера и вывести сообщение по собстенному коду ошибки
* void die_pos_err(int err): Закончить работу сервера и вывести сообщение по стандартому коду ошибки POSIX
## Сборка проекта
    cd video
    gcc demo.c interfaces/tcp.c interfaces/errors.c
# Разработайте веб-сервер игры
---

## 1. Предоставляю клиентам REST API со списком карт.

### Формат входных данных
* Программе через единственный параметр командной строки подаётся путь к конфигурационному файлу в формате JSON, содержащему информацию об игровом поле.
Следующая команда запускает сервер и передаёт ему файл config.json:
*game_server config.json*

* Файл содержит JSON-объект с единственным свойством maps. Это свойство описывает массив игровых карт или уровней.
>{\
>   &emsp;"maps": []\
>}


Каждый элемент этого массива — объект, описывающий дороги, здания и офисы на игровой карте. Свойства этого объекта:
- id — уникальный идентификатор карты. Тип: строка.
- name — название карты, которое выводится пользователю. Тип: строка.
- roads — дороги игровой карты. Тип: массив объектов. Массив должен содержать хотя бы один элемент.
- buildings — здания. Тип: массив объектов. Массив может быть пустым.
- offices — офисы бюро находок. Тип: массив объектов. Массив может быть пустым.


**Дороги.** В массиве roads содержатся элементы двух типов, каждый из которых описывает горизонтальный либо вертикальный отрезок дороги. Порядок следования элементов в этом массиве может быть произвольным.
- Горизонтальный отрезок дороги — объект со свойствами x0, y0, x1, задающих целочисленные значения координат начала и конца отрезка. Пример:
>{ "x0": 0, "y0": 0, "x1": 40 }
- Вертикальный описывается объектом со свойствами x0, y0, y1, которые задают целочисленные значения координат начала и конца отрезка дороги. Пример:
>{ "x0": 0, "y0": 0, "y1": 20 }


**Здание** на карте представлено в виде объекта со свойствами x, y, w, h, которые задают целочисленные координаты верхнего левого угла дома, его ширину и высоту. Пример:
>{ "x": 5, "y": 5, "w": 30, "h": 20 }


**Офис** представлен объектом со свойствами:
- id — уникальный идентификатор офиса. Строка.
- x, y — координаты офиса. Целые числа. Офисы располагаются на дороге, так как собаки могут перемещаться только по дорогам.
- offsetX, offsetY — смещение изображения офиса относительно координат x и y. Эти параметры используются только на стороне клиента для визуализации офиса и на игровую логику не влияют.\
Пример описания офиса:
>{\
>   &emsp;"id": "o0",\
>   &emsp;"x": 40,\
>   &emsp;"y": 30,\
>   &emsp;"offsetX": 5,\
>   &emsp;"offsetY": 0\
>}

Пример конфигурационного файла:\

>{\
>  &emsp;"maps": [\
>    &emsp;&emsp;{\
>      &emsp;&emsp;&emsp;"id": "map1",\
>      &emsp;&emsp;&emsp;"name": "Map 1",\
>      &emsp;&emsp;&emsp;"roads": [\
>        &emsp;&emsp;&emsp;&emsp;{ "x0": 0, "y0": 0, "x1": 40 },\
>        &emsp;&emsp;&emsp;&emsp;{ "x0": 40, "y0": 0, "y1": 30 },\
>        &emsp;&emsp;&emsp;&emsp;{ "x0": 40, "y0": 30, "x1": 0 },\
>        &emsp;&emsp;&emsp;&emsp;{ "x0": 0, "y0": 0, "y1": 30 }\
>      &emsp;&emsp;&emsp;],\
>      &emsp;&emsp;&emsp;"buildings": [\
>        &emsp;&emsp;&emsp;&emsp;{ "x": 5, "y": 5, "w": 30, "h": 20 }\
>      &emsp;&emsp;&emsp;],\
>      &emsp;&emsp;&emsp;"offices": [\
>        &emsp;&emsp;&emsp;&emsp;{ "id": "o0", "x": 40, "y": 30, "offsetX": 5, "offsetY": 0 }\
>      &emsp;&emsp;&emsp;]\
>    &emsp;&emsp;}\
>  &emsp;]\
>}

В конфигурационном файле содержится одна карта с названием “Map 1” и id, равным “map1”. Карта состоит из 4 отрезков дороги, 1 здания и 1 бюро находок.


### Описание поведения веб-сервера
Сервер должен принимать HTTP-запросы на порт 8080. Нужно реализовать поддержку запросов двух типов:
1) получение списка карт,\
*/api/v1/maps* — GET-запрос возвращает в теле ответа краткую информацию обо всех картах в виде JSON-массива объектов с полями id и name. Ответ должен иметь обязательный заголовок Content-Type со значением application/json. Карты в ответе должны быть перечислены в том же порядке, что и в конфигурационном файле.\
Пример тела ответа:
>[{"id": "map1", "name": "Map 1"}]

2) получение описания карты с заданным id.\
*/api/v1/maps/{id-карты}* — GET-запрос возвращает в теле ответа JSON-описание карты с указанным id, семантически эквивалентное представлению карты из конфигурационного файла. Ответ сервера должен иметь заголовок Content-Type со значением application/json.\
Пример тела ответа на запрос *GET /api/v1/maps/map1 HTTP/1.1*:
>{\
>  &emsp;"id": "map1",\
>  &emsp;"name": "Map 1",\
>  &emsp;"roads": [\
>    &emsp;&emsp;{ "x0": 0, "y0": 0, "x1": 40 },\
>    &emsp;&emsp;{ "x0": 40, "y0": 0, "y1": 30 },\
>    &emsp;&emsp;{ "x0": 40, "y0": 30, "x1": 0 },\
>    &emsp;&emsp;{ "x0": 0, "y0": 0, "y1": 30 }\
>  &emsp;],\
>  &emsp;"buildings": [\
>    &emsp;&emsp;{ "x": 5, "y": 5, "w": 30, "h": 20 }\
>  &emsp;],\
>  &emsp;"offices": [\
>    &emsp;&emsp;{ "id": "o0", "x": 40, "y": 30, "offsetX": 5, "offsetY": 0 }\
>  &emsp;]\
>}


### Обработка ошибок
В случае ошибки сервер должен возвращать ответ с соответствующим HTTP-кодом ошибки. Заголовок Content-Type ответа сервера должен иметь значение application/json. Тело ответа — JSON-объект с полями code и message, содержащими код ошибки и её понятное описание.
- Если в запросе на получение описания карты указан id несуществующей карты, сервер должен вернуть ответ со статус-кодом 404 и следующим телом ответа:
>{\
>  &emsp;"code": "mapNotFound",\
>  &emsp;"message": "Map not found"\
>}

Поле message может содержать и другой понятный человеку текст, однако поле code должно иметь значение “mapNotFound”.
- Если URI-строка запроса начинается с /api/, но не подпадает ни под один из текущих форматов, сервер должен вернуть ответ с 400 статус-кодом. Тело ответа должно содержать такой JSON-объект:
>{\
>  &emsp;"code": "badRequest",\
>  &emsp;"message": "Bad request"\
>}

Здесь поле message также может иметь другой понятный текст, однако поле code должно иметь значение “badRequest”.


### Обработка сигналов
После своего старта сервер должен реагировать на сигналы SIGINT и SIGTERM и корректно завершать свою работу при получении этих сигналов.


---
## 2. Добавил возможность отдавать содержимое статических файлов, расположенных внутри заданного каталога.

### Входные данные:
Имя каталога файлами клиентской части игры передаётся в аргументах командной строки при запуске

### Выходные данные
GET- и HEAD-запросы, в которых URI-строка начинается не с */api/*, должны интерпретироваться сервером как запросы статических файлов внутри соответствующего каталога.
Например, в теле ответа на запрос *GET /images/image1.png HTTP/1.1* сервер должен отдать содержимое файла *images/image1*.png относительно каталога со статическими файлами. Ответ должен содержать следующие заголовки ответа:
- Content-Type — MIME-тип контента, зависящий от расширения файла.
- Content-Length — размер файла в байтах
Значение заголовка Content-Type должно зависеть от расширения запрашиваемого файла:
- .htm, .html: text/html
- .css: text/css
- .txt: text/plain
- .js: text/javascript
- .json: application/json
- .xml: application/xml
- .png: image/png
- .jpg, .jpe, .jpeg: image/jpeg
- .gif: image/gif
- .bmp: image/bmp
- .ico: image/vnd.microsoft.icon
- .tiff, .tif: image/tiff
- .svg, .svgz: image/svg+xml
- .mp3: audio/mpeg
- Для файлов с пустым или неизвестным расширением заголовок Content-Type должен иметь значение application/octet-stream.
Определение MIME-типа по расширению файла должно выполняться без учёта регистра символов.


Если URI-строка запроса задаёт путь к каталогу в файловой системе, должно вернуться содержимое файла *index.html* из этого каталога. Например, следующие строки запроса ссылаются на файл *index.html* в корневой директории веб-сервера.
*GET / HTTP/1.1*
*GET /index.html HTTP/1.1*
Сервер должен трактовать URI-строку запроса как URL-encoded. Например, по запросу *GET /hello%20world.txt HTTP/1.1* должно вернуться содержимое файла *hello world.txt* из корневого каталога веб-сервера. Необходимо осуществлять URL-декодирование строки.


### Обработка ошибок
- Если URI-строка ссылается на несуществующий файл, сервер должен вернуть ответ со статус-кодом 404 Not Found и Content-Type: text/plain. Текст в ответе остаётся на ваше усмотрение.
- Если результирующий путь к файлу оказался вне корневого каталога со статическими файлами, сервер должен вернуть ответ со статус-кодом 400 Bad Request и Content-Type: text/plain. Текст ошибки в ответе остаётся на ваше усмотрение.


---
## 3. Реализовать логирование

Для логирования в нашем игровом сервере будем использвать Boost.Log.
Логи будем писать в стандартный вывод, чтобы их удобно было читать извне docker-контейнера.
Выберем для этого формат JSON: будем записывать отдельный объект на каждой строке.
JSON-запись представляет собой объект с такими полями:
- timestamp — время логирования в формате ISO (полученное из объекта записи и преобразованное функцией to_iso_extended_string),
- message — сообщение,
- data — произвольные данные в виде JSON-объекта, переданные в лог через атрибут.


**В сервере нужно логировать следующие события:**
1) запуск сервера,
2) остановку сервера,
3) получение запроса,
4) формирование ответа,
5) возникновение ошибки.


Чтобы при логировании выводить JSON-сообщения, нужно задать собственный форматер.
В нём представьте окончательный объект в виде boost::json::value, а затем выведите его в поток логгера.


### Формат выходных данных
Ниже приводятся значения полей записи, которые должны поступать в stdout при каждом случае логирования:
1) При запуске сервера:
  * message — строка *"server started"*;
  * data — объект с полями:
    - port — порт, на котором работает сервер (обычно 8080),
    - address — адрес интерфейса (обычно 0.0.0.0).
2) При остановке сервера:
  * message — строка *"server exited"*;
  * data — объект с полями:
    - code — код возврата (0 при успехе, *EXIT_FAILURE* при ошибке),
    - exception — если выход по исключению, то описание исключения (```std::exception::what()```).
3) При получении запроса:
  * message — строка *"request received"*;
  * data — объект с полями:
    - ip — IP-адрес клиента (полученный через ```endpoint.address().to_string())```,
    - URI — запрошенный адрес,
    - method — использованный метод HTTP.
4) При формировании ответа:
  * message — строка *"response sent"*;
  * data — объект с полями:
    - ip — IP-адрес клиента (полученный через ```endpoint.address().to_string()```),
    - response_time — время формирования ответа в миллисекундах (целое число).
    - code — статус-код ответа, например, 200 (```http::response<T>::result_int()```).
    - content_type — строка или ```null```, если заголовок в ответе отсутствует.
5) При возникновении сетевой ошибки:
  * message — строка *"error"*;
  * data — объект с полями:
    - code — код ошибки (```beast::error_code::value()```).
    - text — сообщение ошибки (```beast::error_code::message()```).
    - where — место возникновения (```read, write, accept```).
Также ко всем выводимым сообщениям добавляется поле *"timestamp"*.


---
## 4. Реализовать вход на сервер и получение списка карт сессии игрока.

Реализуйте в игровом сервере операции входа в игру и получения информации об игроках.
Пользователь может присоединиться к игре на любой из доступных карт.
Для входа в игру он должен сообщить серверу кличку своего пса и id карты.


Сервер должен создать пса на указанной карте и игрока, управляющего псом, а затем вернуть клиенту
id игрока и токен для аутентификации. Id нужен клиенту, чтобы отличать своего игрока от других.
Клиент должен предъявить токен серверу, чтобы получать информацию о состоянии игрового сеанса и
управлять своим игроком.


### Вход в игру
Для входа в игру реализуйте обработку POST-запроса к точке входа /api/v1/game/join.
Параметры запроса:
* Обязательный заголовок Content-Type должен иметь тип application/json.
* Тело запроса — JSON-объект с обязательными полями userName и mapId: имя игрока и id карты. Имя игрока совпадает с именем пса.


В случае успеха должен возвращаться ответ, обладающий следующими свойствами:
* Статус-код *200 OK*.
* Заголовок Content-Type должен иметь тип application/json.
* Заголовок Content-Length должен хранить размер тела ответа.
* Обязательный заголовок Cache-Control должен иметь значение no-cache.
* Тело ответа — JSON-объект с полями authToken и playerId:
     - Поле playerId — целое число, задающее id игрока.
     - Поле authToken — токен для авторизации в игре — строка, состоящая из 32 случайных шестнадцатеричных цифр.


Если в качестве mapId указан несуществующий id карты, должен вернуться ответ:
* со статус-кодом *404 Not found*
* заголовками:
    - Content-Length: <размер тела ответа>,
    - Content-Type: application/json,
    - Cache-Control: no-cache.
* Тело ответа — JSON-объект с полями code и message:
    - Поле code — строка *"mapNotFound"*.
    - Поле message — строка с понятным человеку текстом ошибки.


Если было передано пустое имя игрока, должен вернуться ответ:
* со статус-кодом *400 Bad request*
* заголовками:
    - Content-Length: <размер тела ответа>,
    - Content-Type: application/json,
    - Cache-Control: no-cache.
* Тело ответа — JSON-объект с полями code и message:
    - Поле code — строка *"invalidArgument"*.
    - Поле message — строка с понятным человеку текстом ошибки.


Если при парсинге JSON или получении его свойств произошла ошибка, должен вернуться ответ:
* со статус-кодом *400 Bad request*
* заголовками:
    - Content-Length: <размер тела ответа>,
    - Content-Type: application/json
    - Cache-Control: no-cache.
* Тело ответа — JSON-объект с полями code и message:
    - Поле code — строка *"invalidArgument"*.
    - Поле message — строка с понятным человеку текстом ошибки.


Если метод запроса отличается от *POST*, должен вернуться ответ:
* со статус-кодом *405 Method Not Allowed*
* заголовками:
    - Content-Length: <размер тела ответа>,
    - Content-Type: application/json,
    - Allow: POST,
    - Cache-Control: no-cache.
* Тело ответа — JSON-объект с полями code и message:
    - Поле code — строка *"invalidMethod"*.
    - Поле message — строка с понятным человеку текстом ошибки.

### Получение списка игроков
Чтобы получить список игроков, находящихся в одной игровой сессии с игроком, используется GET-запрос к точке входа */api/v1/game/players*. Параметры запроса:
* Обязательный заголовок Authorization: Bearer <токен пользователя>.
В качестве токена пользователя следует передать токен, полученный при входе в игру.
Этот токен сервер использует, чтобы аутентифицировать игрока и определить, на какой карте он находится.


В случае успеха должен вернуться ответ со следующими свойствами:
* Статус-код *200 OK*.
* Заголовок Content-Type: application/json.
* Заголовок Content-Length: <размер тела ответа>.
* Заголовок Cache-Control: no-cache.
* Тело ответа — JSON-объект. Его ключи — идентификаторы пользователей на карте.
Значение каждого из этих ключей — JSON-объект с единственным полем name, строкой, задающей имя пользователя, под которым он вошёл в игру.


Если заголовок **Authorization** в запросе отсутствует либо его значение не удовлетворяет требованиям задачи, должен вернуться ответ со следующими свойствами:
* Статус-код *401 Unauthorized*.
* Заголовок Content-Type: appication/json.
* Заголовок Content-Length: <размер тела ответа>.
* Заголовок Cache-Control: no-cache.
* Тело ответа — JSON-объект с полями code и message:
    - Поле code должно иметь значение *"invalidToken"*.
    - Поле message — содержать понятное человеку описание ошибки.


Если заголовок **Authorization** содержит валидное значение токена, но в игре нет пользователя с таким токеном, должен вернуться ответ со следующими свойствами:
* Статус-код *401 Unauthorized*.
* Заголовок Content-Type: appication/json.
* Заголовок Content-Length: <размер тела ответа>.
* Заголовок Cache-Control: no-cache.
* Тело ответа — JSON-объект с полями code и message:
    - Поле code должно иметь значение *"unknownToken"*.
    - Поле message — содержать понятное человеку описание ошибки.

Если метод запроса отличается от *GET* или *HEAD*, должен вернуться ответ со следующими свойствами:
* Статус-код *405 Method Not Allowed*.
* Заголовок Content-Type: appication/json.
* Заголовок Allow: GET, HEAD.
* Заголовок Content-Length: <размер тела ответа>.
* Заголовок Cache-Control: no-cache.
* Тело ответа — JSON-объект с полями code и message:
    - Поле code должно иметь значение *"invalidMethod"*.
    - Поле message — содержать понятное человеку описание ошибки.


---
## 5. Реализовать в игровом сервере операции получения игрового состояния и управления действиями персонажей.

* Добавьте в модель пса информацию о его координатах, скорости и направлении в пространстве:
  - Координаты пса на карте задаются двумя вещественными числами. Для описания вещественных координат разработайте структуру или класс.
  - Скорость пса на карте задаётся также двумя вещественными числами. Скорость измеряется в единицах карты за одну секунду.
  - Направление в пространстве принимает одно из четырех значений: NORTH (север), SOUTH (юг), WEST (запад), EAST (восток).
- После добавления на карту пёс должен иметь имеет скорость, равную нулю.
- Координаты пса — случайно выбранная точка на случайно выбранном отрезке дороги этой карты.
- Направление пса по умолчанию — на север.

* Чтобы получить информацию о состоянии игры, реализуйте обработку GET-запроса к точке входа /api/v1/game/state.
### Параметры запроса:
1. Обязательный заголовок Authorization: Bearer <токен пользователя> передаёт токен пользователя, полученный при входе в игру.

В случае успеха сервер должен вернуть ответ, обладающий свойствами:
* Статус-код *200 OK*.
* Заголовок Content-Type: application/json.
* Заголовок Content-Length: <размер тела ответа>.
* Обязательный заголовок Cache-Control: no-cache.
* Тело ответа — JSON объект, в теле ответа содержит свойство players — объект, ключи которого — это id игроков,
а значения — это объекты с полями: pos, speed и dir:
  - pos — массив из двух вещественных чисел, координат x и y игрока на карте.
  - speed — массив из двух вещественных чисел: скорость пса, которым управляет игрок по горизонтальной и вертикальной оси.
  Измеряется скорость в координатных единицах за секунду. Управление персонажами будет реализовано позже, поэтому скорость объектов пока равна нулю.
  - dir — строка, задающая направление главного героя:
    - "L"  — влево (на запад),
    - "R" — вправо (на восток),
    - "U" — вверх (на север),
    - "D" — вниз (на юг).
  Управление персонажами будет реализовано позже, поэтому должно возвращаться направление по умолчанию "U".


Если метод запроса отличается от *GET* или *HEAD*, должен вернуться ответ со следующими свойствами:
* Статус-код *405 Method Not Allowed*.
* Заголовок Allow: GET, HEAD.
* Заголовок Content-Type: application/json.
* Заголовок Content-Length: <размер тела ответа>.
* Заголовок Cache-Control: no-cache.
* Тело ответа — JSON-объект с полями code и message:
    - Поле code должно иметь значение *"invalidMethod"*.
    - Поле message — содержать понятное человеку описание ошибки.


---
## 6. Реализовать в игровом сервере возможность конфигурировать скорость движения персонажей на карте

### Реализуйте возможность настраивать скорость персонажей.
Для этого в конфигурационный файл добавляются два новых свойства:
  - Скорость персонажей на всех картах задаёт опциональное поле defaultDogSpeed в корневом JSON-объекте.
Если это поле отсутствует, скорость по умолчанию считается равной 1.
  - Скорость персонажей на конкретной карте задаёт опциональное поле dogSpeed в соответствующем объекте карты.
Если это поле отсутствует, на карте используется скорость по умолчанию.

### Чтобы управлять действиями персонажа, реализуйте обработку POST-запроса к точке входа */api/v1/game/player/action*.
Параметры запроса:
* Обязательный заголовок Content-Type: application/json.
* Обязательный заголовок Authorization: Bearer <токен пользователя> передаёт токен пользователя, полученный при входе в игру.
* Тело запроса — JSON-объект с полем move, которое принимает одно из значений:
    - "L" — задаёт направление движения персонажа влево (на запад).
    - "R" — задаёт направление движения персонажа вправо (на восток).
    - "U" — задаёт направление движения персонажа вверх (на север).
    - "D" — задаёт направление движения персонажа вниз (на юг).
    - "" — останавливает персонажа.


При обработке этих операций должна измениться скорость движения персонажа. Буквой s обозначена скорость персонажей на данной карте:
* При движении влево скорость равна {-s, 0}.
* При движении вправо скорость равна {s, 0}.
* При движении вверх скорость равна {0, -s}.
* При движении вниз скорость равна {0, s}.
* При остановке скорость равна {0, 0}.


При успешной обработке запроса ответ на запрос должен обладать следующими свойствами:
* Статус-код *200 OK*.
* Заголовок Cache-Control: no-cache.
* Заголовок Content-Type: application/json.
* Заголовок Content-Length: <размер тела ответа>.
* Тело ответа — пустой JSON-объект.


Должна осуществляться проверка:
* авторизации
* JSON объекта в запросе
* заголовка Content-Type
* запрос только POST


*В этом задании в игре время считается остановившимся. Поэтому объекты даже с  ненулевой скоростью не должны менять своих координат.*


Получив команду управления движением, пес должен изменить значение скорости и направление движения, однако его координаты изменяться не должны.


---
## 7. Реализовать в игровом сервере обновление игрового состояния с течением времени.

**Чтобы было легче протестировать вашу программу автотестами, временно доработайте код так, чтобы после входа в игру пёс игрока появлялся в начальной точке первой дороги карты, а не в случайно сгенерированной.**

### Добавить в REST API POST-запрос */api/v1/game/tick*.
Тестирующая программа будет его отправлять, чтобы вручную контролировать ход времени сервера.

- Правила перемещения собак
  * Персонажи-собаки, управляемые игроками, могут перемещаться только вдоль дорог.
  * Ширина дороги равна 0.8 координатных единиц. Таким образом собака может удаляться от оси дороги не более, чем
на 0.4 координатные единицы.
- За момент времени Δt персонаж, движущийся со скоростью V, должен переместиться на расстояние V * Δt, если нет препятствий.
- Псам не разрешается перемещаться за пределами дорог.


**Если вдоль вектора перемещения находится граница дороги, пёс должен остановиться у границы, и его скорость должна стать равна нулю.**


**Чтобы быстро находить участок дороги по координатам, рекомендуем построить вспомогательную структуру данных, чтобы вместо линейного перебора всех дорог на карте искать дороги внутри map, unordered_map или отсортированного vector.**


- Запрос для управления временем на карте
Добавьте в REST API POST-запрос для управления игровым временем к точке входа */api/v1/game/tick*.
Параметры запроса:
* Обязательный заголовок Content-Type: application/json.
* Тело запроса — JSON-объект с целочисленным полем timeDelta, задающим значение параметра Δt в миллисекундах.


При успешном выполнении этого запроса ответ сервера должен обладать следующими свойствами:
* Статус код *200 OK*.
* Заголовок Cache-Control: no-cache.
* Заголовок Content-Type: application/json.
* Заголовок Content-Length: <размер тела ответа>.
* Тело ответа — пустой JSON-объект.


После выполнения этого запроса все персонажи должны переместиться по правилам перемещения персонажей.
Последующие запросы игрового состояния должны возвращать новые координаты персонажей.


Если тело запроса содержит невалидный JSON, либо JSON-объект, поле *timeDelta* которого содержит невалидное значение, должен вернуться ответ со свойствами:
* Статус-код *400 Bad request*.
* Заголовок Content-Type: application/json.
* Заголовок Cache-Control: no-cache.
* Заголовок Content-Length: <размер тела ответа>.
* Тело ответа — JSON-объект с полями code и message:
    - Поле code должно иметь значение *"invalidArgument"*.
    - Поле message — содержать понятное человеку описание ошибки.


---
## 8. Реализовать в игровом сервере автоматическое обновление игрового состояния с течением времени и возможность запускать приложение в режиме тестирования.

### Примените Boost.ProgramOptions и обновите параметры командной строки приложения:
* Параметр --tick-period (-t) задаёт период автоматического обновления игрового состояния в миллисекундах. Если этот параметр указан, каждые N миллисекунд сервер должен обновлять координаты объектов. Если этот параметр не указан, время в игре должно управляться с помощью запроса /api/v1/game/tick к REST API.
* Параметр --config-file (-c) задаёт путь к конфигурационному JSON-файлу игры.
* Параметр --www-root (-w) задаёт путь к каталогу со статическими файлами игры.
* Параметр --randomize-spawn-points включает режим, при котором пёс игрока появляется в случайной точке случайно выбранной дороги карты.
* Параметр --help (-h) должен выводить информацию о параметрах командной строки.


Если при запуске сервера параметр --randomize-spawn-points не указан, пёс игрока должен появляться в начальной точке первой дороги на карте.


Когда сервер запущен с параметром --tick-period, сервер должен отвечать на запросы к точке входа */api/v1/game/tick* так же, как на запросы к любой другой несуществующей точке входа:
* Статус-код *400 Bad request*.
* Заголовок Content-Type: application/json.
* Заголовок Content-Length: <размер тела ответа>.
* Заголовок Cache-Control: no-cache.
* Тело запроса — JSON-объект с полями code и message:
    - Поле code должно иметь значение *"badRequest"*.
    - Поле message — понятное человеку сообщение об ошибке.


### Реализовать режим реального времени, сам сервер должен с заданной периодичностью обновлять игровое время.

---
## 9. Реализовать в игровом сервере генерирование потерянных вещей на карте.

* Доработайте модель игры, чтобы с течением времени на игровой карте случайным образом появлялись предметы. Для этого используйте класс LootGenerator из заготовки кода.
* Код генерирования игровых предметов покройте юнит-тестами, используя фреймворк для тестов Catch2.
* Доработайте REST API, чтобы запрос /api/v1/game/state возвращал информацию о потерянных предметах, находящихся на карте.
* Доработайте REST API, чтобы запрос /api/v1/maps/{id} возвращал необходимую фронтенду информацию о потерянных предметах, которые могут появиться на этой карте. Служебная информация должна загружаться из конфигурационного JSON-файла игры при старте сервера.

### Входные данные программы
В конфигурационный JSON-файл игры вносятся следующие изменения:
- В корневой объект добавляется свойство lootGeneratorConfig типа Object с полями period и probability. Например:

```{```
```    ...```
```    "lootGeneratorConfig": {```
```        "period": 5.0,```
```        "probability": 0.5```
```    },```
```    ...```
```}```

Это значит, что вероятность появления на карте потерянного объекта в течение 5 секунд равна 0.5 (то есть, появится либо не появится). Параметры period и probability должны использоваться для инициализации класса LootGenerator. Его описание будет ниже.


В описание каждой карты добавляется поле lootTypes — массив объектов с произвольным содержимым, необходимым фронтенду. На логику работы бэкенда в этом задании влияет только количество элементов в этом массиве. Пример:

```{```
```    ...```
```    "maps": [```
```        {```
```            "id": "map1",```
```            "name","Map 1",```
```            "lootTypes": [```
```                {```
```                    "name": "key",```
```                    "file": "assets/key.obj",```
```                    "type": "obj",```
```                    "rotation": 90,```
```                    "color" : "#338844",```
```                    "scale": 0.03```
```                },```
```                {```
```                    "name": "wallet",```
```                    "file": "assets/wallet.obj",```
```                    "type": "obj",```
```                    "rotation": 0,```
```                    "color" : "#883344",```
```                    "scale": 0.01```
```                }```
```            ],```
```            ...```
```        },```
```        ...```
```    ]```
```}```

- На карте map1 возможно генерирование объектов двух типов: ключей (key) и бумажников (wallet).
- В валидном конфигурационном файле должно содержаться не менее одного типа трофеев.

### Выходные данные программы
По GET-запросу /api/v1/maps/{идентификатор} в описании карты должно быть свойство lootTypes c содержимым из конфигурационного файла.

```{```
```    "id":"map1",```
```    "name":"Map 1",```
```    "roads":[```
```        { "x0":0, "y0":0, "x1":40 },```
```        { "x0":40, "y0":0, "y1":30 },```
```        { "x0":40, "y0":30, "x1":0 },```
```        { "x0":0, "y0":0, "y1":30 }```
```    ],```
```    "buildings":[```
```        { "x":5, "y":5, "w":30, "h":20 }```
```    ],```
```    "offices":[```
```        { "id":"o0", "x":40, "y":30, "offsetX":5, "offsetY":0 }```
```    ],```
```    "lootTypes":[```
```        {```
```            "name":"key",```
```            "file":"assets/key.obj",```
```            "type":"obj",```
```            "rotation":90,```
```            "color":"#338844",```
```            "scale":3E-2```
```        },```
```        {```
```            "name":"wallet",```
```            "file":"assets/wallet.obj",```
```            "type":"obj",```
```            "rotation":0,```
```            "color":"#883344",```
```            "scale":1E-2```
```        }```
```    ]```
```}```

В ответ на GET-запрос /api/v1/game/state в JSON-объект добавьте свойство lostObjects типа Object. Его ключи (целые числа) идентификаторы потерянных объектов, а значения — объекты с полями:
- type — целое число, задающее тип объекта в диапазоне [0, N-1], где N — количество различных типов трофеев, заданных в массиве lootTypes на карте. Например, для карты, содержащей массив lootTypes из 7 элементов, поле type должно находиться в диапазоне от 0 до 6 включительно.
- pos — массив из двух чисел: координаты x и y объекта на карте.

```{```
```    "players":{```
```        "0":{"pos":[0, 0], "speed":[0, 0], "dir":"U"}```
```    },```
```    "lostObjects":{```
```        "0":{```
```            "type":1,```
```            "pos":[36.5, 0]```
```        }```
```    }```
```}```

### Правила генерирования объектов на карте
Класс LootGenerator в заготовке кода возвращает количество трофеев, которые должны быть сгенерированы в течение заданного промежутка времени на карте, где находятся N трофеев (loot) и M мародёров (looter).

В вашей игре роль трофеев играют потерянные предметы, а в роли мародёров здорового человека — собаки, которые собирают эти предметы и доставляют их на базу. Каждый игровой тик используйте этот класс, чтобы узнать, сколько потерянных объектов должно появиться на карте за время, прошедшее с предыдущего игрового тика. Генератор устроен так, чтобы количество трофеев на карте не превышало количество мародёров.

Параметры инициализации LootGenerator задаются в конфигурационном JSON-файле. Для этого используется свойство lootGeneratorConfig, о котором говорилось выше.

Получив от генератора количество новых потерянных предметов случайным образом, сгенерируйте для каждого из них тип и координаты:
- Тип предмета — целое число от 0 до K−1 включительно, где K — количество элементов в массиве lootTypes, указанного в конфигурационном файле соответствующей карты.
- Объект генерируется в случайно выбранной точке на случайно выбранной дороге карты.

### Юнит-тестирование кода, генерирующего трофеи на карте
* Добавьте в conanfile.txt вашего проекта зависимость от фреймворка Catch2
* Добавьте в файл CMakeLists.txt вашего проекта исполняемый файл game_server_tests. Включите в него .cpp и .h файлы тестов (разместите их в каталоге tests), а также тестируемые .cpp и .h файлы игрового сервера из каталога src. Кроме файлов с тестируемыми классами может быть нужно будет добавить файлы, которые они используют.

## 10. Реализуйте в проекте сбор предметов и возвращение предметов в офис.

- Сбор предметов нужно осуществлять во время тика. При этом учитывается перемещение игроков, произошедшее за время тика. Нужно учитывать начальную и конечную координаты каждого игрока. Мы считаем, что игрок двигался равномерно по прямой в течение тика.
- Игроки имеют рюкзак, вместимость которого указывается в конфигурационном файле. Игрок берёт все предметы, мимо которых он проходит, если рюкзак не полон. Игрок пропускает предмет, если рюкзак полон. Проходя мимо базы, игрок убирает все предметы из рюкзака.
- Обрабатывать коллизии нужно в хронологическом порядке. При этом учтите, что за время одного тика даже с одним игроком может произойти несколько событий. Например игрок может:
  * Взять предмет.
  * Пропустить другой предмет, потому что рюкзак полон.
  * Сдать все предметы на базу.
  * Взять новый предмет.

Если два или более игрока с незаполненными рюкзаками проходят мимо одного предмета, то его берёт тот игрок, который достиг предмет ранее.

Ширина объектов игры такая:
  * Предметы — ширина ноль,
  * Игроки — ширина 0,6,
  * Базы — ширина 0,5.

Таким образом, половина игрока находится вне дороги при движении по её краю. Так как ширина дороги 0.4, игрок в этом случае не соберёт предмет, лежащий в центре дороги, но может вернуть предметы на базу. Учтите, <b>для проверки коллизии нужно использовать половину ширины объектов</b>. Например, <u>игрок соберёт предмет, находящийся от него на расстоянии 0.3 или меньше</u>. Предметы вернутся на базу, если до неё от игрока не более 0.55 единиц расстояния: 0.5 / 2 + 0.6 / 2.
Если два события происходят одновременно, порядок их выполнения не определён.

### Изменения в конфигурационном файле
В конфигурационный файл добавляются два новых свойства:
- Вместимость рюкзаков на всех картах задаёт опциональное поле defaultBagCapacity в корневом JSON-объекте. Если это поле отсутствует, вместимость по умолчанию считается равной 3.
- Вместимость рюкзаков на конкретной карте задаёт опциональное поле bagCapacity в соответствующем объекте карты. Если это поле отсутствует, на карте используется вместимость по умолчанию.

### Изменение в протоколе взаимодействия с клиентом
В ответе на запрос к /api/v1/game/state теперь нужно отдавать <b>содержимое рюкзака игроков</b>. Для этого в информацию об игроке добавьте поле bag. Его тип — массив, содержащий информацию о собранных предметах. Информация задаётся в виде объекта со следующими полями:
- id (целое число) — идентификатор предмета. Совпадает с тем идентификатором, который имел предмет до того, как его нашли.
- type (целое число) — тип предмета. Тип также не должен меняться при подборе.

Следует передавать предметы в том порядке, в котором они были собраны. Вот пример ответа сервера:

```{```
```   "players":{```
```      "13":{```
```         "pos":[10.5,3.8],```
```         "speed":[0.5, 0],```
```         "dir":"L",```
```         "bag":[```
```            {"id":9, "type":4},```
```            {"id":8, "type":4}```
```         ]```
```      }```
```   },```
```   "lostObjects":{```
```      "11":{```
```         "type":3,```
```         "pos":[13.2,17.2]```
```      }```
```   }```
```}```

## 11. Добавьте в игру возможность начисления очков за сдачу предметов на базу.

В конфигурационном файле в описании каждого предмета задаётся поле value. Оно определяет, сколько очков будет начисляться пользователю за находку и возврат предметов подобного типа.

Каждому игроку добавьте поле, которое будет хранить его счёт в данной партии. Начальное количество очков — ноль. При возврате предметов игроком увеличивайте его счёт на стоимость найденного предмета.

### Изменение в протоколе взаимодействия с клиентом
В ответе на запрос к /api/v1/game/state нужно выводить счёт каждого игрока в поле score.
## Разработайте программу Bookypedia — консольную программу, которая хранит книги в БД Postgres.

При запуске программа должна считать URL базы данных из переменной окружения `BOOKYPEDIA_DB_URL`.

При разработке использовать паттерн "Репозиторий".

Применить паттерн "Unit of work" — единица работы. Идея состоит в том, что теперь приложение не имеет прямого доступа к репозиторию. Репозиторий можно получить только имея объект типа UnitOfWork. Сам объект типа UnitOfWork можно сконструировать имея фабрику объектов этого типа. Именно фабрика изначально доступна приложению.

Каждая команда должна выполняться в рамках самостоятельной транзакции. Ошибки, возникшие во время выполнения команды, не должны приводить к изменениям.

### Формат базы данных

В базе данных содержатся таблицы: authors и books.
* Таблица **authors** хранит информацию об авторах. Поля таблицы:
    - **id** — уникальный идентификатор автора. Имеет тип **uuid**. Например: `4b3170d8-7aeb-4df2-b065-128898fb7810`. Поле **id** — выступает в роли первичного ключа таблицы.
    - **name** — имя автора книги, строка длиной до 100 символов. Дублирующиеся значения в этом столбце не допускаются. Имена, записанные в разном регистре, считаются разными: `Antoine De Saint-Exupery и Antoine de Saint-Exupery`.
* Таблица **books** хранит информацию о книгах:
    - **id** — уникальный идентификатор книги типа **uuid**. Выступает в роли первичного ключа таблицы.
    - **author_id** — идентификатор автора книги типа **uuid**. Значения `NULL` не допускаются.
    - **title** — название книги, строка длиной до 100 символов. Допускается дублирование значений в этом поле. Значения `NULL` не допускаются. При поиске книги названия, записанные в разном регистре, считаются разными.
    - **publication_year** — год публикации, целое число.
* Таблица **book_tags** хранит теги книг:
    - **book_id** типа **uuid**. Идентификатор книги, к которой относится тег.
    - **tag** — строка длиной до 30 символов. Собственно, сам тег.

Если при старте программы какой-либо из таблиц **author**, **books**, **book_tags** не существует, программа должна создать их. Если указанные таблицы существуют, предполагается, что они содержат правильную структуру.

### Формат входных и выходных данных программы

На стандартный вход программы подаются команды — по одной в каждой строке. Команды имеют формат:

> Команда <аргументы команды>

Работа программы завершается после того как в `stdin` данные закончатся.

### Команды
#### Команда AddAuthor

Команда **AddAuthor <name>** добавляет в БД автора книги с указанным именем, а **id** автора генерируется случайным образом. Имя состоит из одного или нескольких слов, разделённых пробелами. Пробелы в начале и конце имени удаляются. Примеры:
```
AddAuthor Joanne Rowling
AddAuthor Antoine de Saint-Exupery
```

Если автор был добавлен успешно, команда ничего выводить в stdout не должна. Есть при добавлении произошла ошибка, программа должна вывести в `stdout` строку `Failed to add author`. В следующем примере выводится ошибка, когда пользователь вводит имя автора повторно, а также когда имя автора пустое:
```
AddAuthor Joanne Rowling
AddAuthor Joanne Rowling
Failed to add author
AddAuthor
Failed to add author
```

#### Команда DeleteAuthor

Команда **DeleteAuthor** удаляет выбранного автора, все его книги и связанные с этими книгами теги. Книги других авторов и их теги не затрагиваются.
```
DeleteAuthor
1 Jack London
2 Joanne Rowling
Enter author # or empty line to cancel
1
ShowAuthors
1 Joanne Rowling
```

Можно указать имя удаляемого автора сразу в команде DeleteAuthor:
```
ShowAuthors
1 Jack London
2 Joanne Rowling
DeleteAuthor Jack London
ShowAuthors
1 Joanne Rowling
```

Если выбранный автор уже был удалён в другом экземпляре приложения либо не существовал никогда, должно быть выведено сообщение `Failed to delete author`.

#### Команда EditAuthor

Команда **EditAuthor** служит для редактирования выбранного автора. Можно указать имя автора в команде либо выбрать из списка:
```
EditAuthor
1 Jack London
2 Joanne Rowling
Enter author # or empty line to cancel
2
Enter new name:
J. K. Rowling
EditAuthor Jack London
Enter new name:
John Griffith Chaney
ShowAuthors
1 J. K. Rowling
2 John Griffith Chaney
```

Если пользователь указал имя несуществующего автора либо в процессе ввода данных автор был удалён параллельно запущенным экземпляром программы, программа должна выдать сообщение `Failed to edit author`.

#### Команда ShowAuthors

Команда **ShowAuthors** не имеет аргументов. Она выводит нумерованный список авторов, отсортированный по алфавиту. Пример:
```
AddAuthor Joanne Rowling
AddAuthor Jack London
ShowAuthors
1. Jack London
2. Joanne Rowling
```

Если список авторов пуст, команда **ShowAuthors** ничего не выводит.

#### Команда AddBook

Команда **AddBook <pub_year> <title>** добавляет в БД книгу с указанным названием и годом публикации. При выполнении команды программа должна предложить ввести имя автора напрямую либо выбрать автора из предложенного списка.
> Enter author name or empty line to select from list:

1) Если пользователь ввёл имя автора вручную, и среди авторов такого нет, программа должна предложить автоматически добавить автора.
> No author found. Do you want to add Jack London (y/n)?

2) Если пользователь ввёл ответ, отличный от `Y` или `y`, то добавление книги отменяется и выводится сообщение `Failed to add book`.

3) Если пользователь согласился добавить автора, то в таблицу авторов добавляется новый автор и ввод данных о книге продолжается.

4) Если пользователь ввёл пустую строку, вывести строку `Select author:`, за которой следует нумерованный список авторов, отсортированных по алфавиту. В конце программа должна вывести строку `Enter author # or empty line to cancel` и прочитать из `stdin` порядковый номер автора.

5) Если введена пустая строка, добавление книги отменяется. Если введён допустимый номер автора, в таблицу книг должна быть добавлена книга. Поле **author_id** добавленной книги должно быть равно **id** автора книги.
```
AddBook 1998 Harry Potter and the Chamber of Secrets
Select author:
1 Jack London
2 Joanne Rowling
Enter author # or empty line to cancel
2
AddBook 1851 Moby-Dick
Select author:
1 Jack Londong
2 Joanne Rowling
Enter author # or empty line to cancel

AddAuthor Herman Melville
AddBook 1851 Moby-Dick
Select author:
1 Jack Londong
2 Joanne Rowling
3 Herman Melville
Enter author # or empty line to cancel
3
```

6) Программа должна попросить ввести теги, которые относятся к книге, разделяя их запятыми. Допускается нулевое количество тегов у книги.
```
Enter tags (comma separated):
adventure, dog,   gold   rush  ,  dog,,dogs
```

7) После ввода тегов программа должна добавить книгу. Перед добавлением теги приводятся к нормализованному виду:
    - Пробелы в начале и в конце тега удаляются.
    - Лишние пробелы между словами тега удаляются. Например, в примере выше должен остаться только один пробел между словами gold и rush.
    - Пустые теги и дубликаты существующих тегов удаляются. В примере нужно оставить только один из двух тегов dog, а также удалить пустой тег между dog и dogs.

В приведённом выше примере должны получиться четыре тега:
```
adventure
gold rush
dog
dogs
```

#### Команда ShowAuthorBooks

Команда **ShowAuthorBooks** не имеет параметров. В ответ на эту команду в `stdout` выводится нумерованный список имён авторов, отсортированный по алфавиту, и просьба ввести номер автора, как в команде **AddAuthor**. После выбора автора в `stdout` должен быть выведен нумерованный список книг с годами их выпуска, отсортированный по году публикации. Если одна или несколько книг были выпущены в одном и том же году, они выводятся в порядке возрастания их названия. Пример:
```
ShowAuthorBooks
Select author:
1 Boris Akunin
2 Jack London
3 Joanne Rowling
4 Herman Melville
Enter author # or empty line to cancel
1
1 Azazelle, 1998
2 Murder on the Leviathan, 1998
3 The Turkish Gambit, 1998
4 She Lover of Death, 2001
```

Если у введённого автора нет книг, либо введена пустая строка вместо порядкового номера автора, команда ничего не выводит.

#### Команда ShowBooks

Команда **ShowBooks** не имеет параметров. Она выводит в `stdout` нумерованный список книг, отсортированный по названию. Информация об книге содержит название, год публикации и автора. Формат вывода:
> ПорядковыйНомер НазваниеКниги by ИмяАвтора, ГодПубликации

Книги должны быть отсортированы:
- по названию книги,
- книги с одинаковым названием — по имени автора,
- книги с одинаковым названием и именем автора — в порядке возрастания года публикации.

```
ShowBooks
1 The Cloud Atlas by David Mitchell, 2004
2 The Cloud Atlas by Liam Callanan, 2004
3 White Fang by Jack London, 1906
```

Если книг в базе нет, в `stdout` ничего выводиться не должно.

#### Команда ShowBook

Команда **ShowBook** выводит подробную информацию о книге: автор, название, год публикации, теги, если есть. Можно указать название книги в самой команде, либо выбрать из списка.

Если книг с введенным названием несколько, программа предлагает выбрать его из списка.

Если у книги есть теги, то выводится надпись **“Tags: “**, за которой выводятся теги в алфавитном порядке, разделённые запятой с пробелом. Если тегов нет, то строка **Tags:** не выводится.
```
ShowBooks
1 The Cloud Atlas by David Mitchell, 2004
2 The Cloud Atlas by Liam Callanan, 2004
3 White Fang by Jack London, 1906
ShowBook White Fang
Title: White Fang
Author: Jack London
Publication year: 1906
Tags: adventure, dog, gold rush
ShowBook The Cloud Atlas
1 The Cloud Atlas by David Mitchell, 2004
2 The Cloud Atlas by Liam Callanan, 2004
Enter the book # or empty line to cancel:
2
Title: The Cloud Atlas
Author: Liam Callanan
Publication year: 2004
ShowBook
1 The Cloud Atlas by David Mitchell, 2004
2 The Cloud Atlas by Liam Callanan, 2004
3 White Fang by Jack London, 1906
Enter the book # or empty line to cancel:
3
Title: White Fang
Author: Jack London
Publication year: 1906
Tags: adventure, dog, gold rush
```

Если книги с введённым названием нет, ничего не должно выводиться

#### Команда DeleteBook

Команда **DeleteBook** позволяет удалить книгу, указав её название напрямую или выбрав из списка. Если книг с введённым названием несколько, либо пользователь не ввёл её название, программа должна вывести имеющиеся книги с таким названием и спросить, какую из книг нужно удалить.
```
ShowBooks
1 The Cloud Atlas by David Mitchell, 2004
2 The Cloud Atlas by Liam Callanan, 2004
3 White Fang by Jack London, 1906
DeleteBook The Cloud Atlas
1 The Cloud Atlas by David Mitchell, 2004
2 The Cloud Atlas by Liam Callanan, 2004
Enter the book # or empty line to cancel:
2
ShowBooks
1 The Cloud Atlas by David Mitchell, 2004
2 White Fang by Jack London, 1906
DeleteBook
1 The Cloud Atlas by David Mitchell, 2004
2 White Fang by Jack London, 1906
Enter the book # or empty line to cancel:
1
```

В случае ошибки удаления книги программа должна вывести сообщение `Failed to delete book`.

#### Команда EditBook

Команда **EditBook** позволяет отредактировать книгу. Можно указать название книги напрямую или выбрать из списка. Если книг с введённым названием несколько, программа должна попросить пользователя выбрать нужную книгу из списка.
```
ShowBooks
1 The Cloud Atlas by David Mitchell, 2004
2 The Cloud Atlas by Liam Callanan, 2004
3 White Fan by Jack London, 1906
EditBook White Fan
Enter new title or empty line to use the current one (White Fan):
White Fang
Enter publication year or empty line to use the current one (1906):

Enter tags (current tags: adventure, cat, gold rush):
adventure, gold rush, dog
ShowBook White Fang
Title: White Fang
Author: Jack London
Publication year: 1096
Tags: adventure, dog, gold rush
EditBook
1 The Cloud Atlas by David Mitchell, 2004
2 The Cloud Atlas by Liam Callanan, 2004
3 White Fang by Jack London, 1906
Enter the book # or empty line to cancel:
3
Enter new title or empty line to use the current one (White Fang):

Enter publication year or empty line to use the current one (1906):

Enter tags (current tags: adventure, dog, gold rush):
adventure, gold rush, dog, wolf
```

Если введённая книга отсутствует, либо была удалена из другого экземпляра программы, должна вывестись надпись `Book not found`.

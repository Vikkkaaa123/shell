# my_shell - Простая оболочка Linux

### Описание
Оболочка командной строки, написанная на языке C, с базовыми функциями и специальными возможностями.

## 🔧 Функционал

### Основные команды
- Чтение и выполнение пользовательских команд:
 - `echo [текст]` - вывод указанного текста
 -  Поддержка выхода из оболочки (`exit`, `\q`, `Ctrl+D`)
 - `\e $VAR` - вывод переменных окружения

### Специальные возможности
- Проверка корректности вводимых команд
- Проверка загрузочных дисков (`\l sda`)
- Дамп памяти процесса (`\mem PID`)
- Обработка сигнала SIGHUP
- Сохранение истории команд в файл `history.txt`

## 🚀 Быстрый старт

1. Скомпилируйте:
```bash
gcc -o my_shell my_shell.c
```
2. Запустите:
```bash
./my_shell
```

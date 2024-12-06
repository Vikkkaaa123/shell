#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdbool.h>
#include <signal.h>

#define BUFFER_SIZE 1024 
#define HISTORY_FILE "history.txt"

// Функция для записи истории команд в файл
void save_to_history(const char *buffer) {
    FILE *file = fopen(HISTORY_FILE, "a");
    if (file) {
        fprintf(file, "%s\n", buffer);
        fclose(file);
    } else {
        perror("Не удалось открыть файл истории");
    }
}


//функция для вывода длины строки без учета пробелов
int str_size_without(const char *buffer){
int count=0;
while(*buffer){
  if (*buffer != ' ') count++;
buffer++;
}
return count;
}

// Функция для вывода переменной окружения
void print_env_variable(const char *var) {
    char *value = getenv(var);
//getenv() ищет в списке окружения переменную окружения и возвращает указатель на строку, содержащую значение указанного имени переменной в текущей среде
    if (value) {
        printf("%s\n", value);
    } else {
        printf("Переменная окружения не найдена.\n");
    }
}


// выполнение бинарника
void executeCommand(const char* command) {  
  pid_t pid = fork();  // с помощью системного вызова создаем новый процесс
//pid_t-тип данных, который определяет идентификатор процесса (PID)
//fork() возвращает 0 в дочернем процессе,положительное значение (PID дочернего процесса) в родительском процессе,отрицательное значение, если произошла ошибка
  if(pid == 0) {  // если процесс дочерний
    execlp(command, command, (char*)NULL); // ищем команду в каталогах PATH. Если эта переменная не определена, то используется путь поиска ":/bin:/usr/bin" по умолчанию
//execlp используется для запуска новой программы. В отличие от fork(), которая создает новый процесс,exec заменяет текущий процесс новым
//Если команда найдена, текущий процесс заменяется на исполняемую программу.
// Первый аргумент — это имя исполняемого файла, который нужно запустить.
//Второй аргумент — это также имя программы (обычно оно повторяется), которое будет передано как первый аргумент в новую программу. Это часто используется для доступа к аргументам командной строки.
//Третий аргумент — завершение списка аргументов для execlp(). Функция execlp требует, чтобы список аргументов заканчивался NULL
    perror("такого файла нет");  // Если команда не найдена, управление возвращается в дочерний процесс, и выполняется строка, которая выводит сообщение об ошибке
    exit(1);
  } else if(pid < 0) {  // произошла ошибка при создании нового процесса 
    perror("ошибка!!!");
  } else {
    wait(NULL);  //Если fork() возвращает положительное значение, это означает, что мы в родительском процессе. В этом случае родительский процесс вызывает wait(NULL), чтобы дождаться завершения дочернего процесса, прежде чем продолжить выполнение.
  }
}



// Обработчик сигнала SIGHUP
void handle_SIGHUP(int signal) {
    if (signal == SIGHUP) {
        printf("Configuration reloaded\n");
    }
}


// Функция для проверки загрузочного диска
//чтобы узнать, какой диск действительно является загрузочным, в другом терминале вызвать sudo fdisk -l (можно еще lsblk)
bool is_boot_disk(const char *disk) {
    char command[256];
    snprintf(command, sizeof(command), "fdisk -l /dev/%s | grep '*' | wc -l", disk);
//Команда выглядит так: fdisk -l <disk> | grep '*' | wc -l,  <disk> заменяется на имя диска, переданное в функцию
//fdisk -l <disk> — показывает информацию о всех дисках и их разделах в системе
///dev - эта директория содержит файлы устройств, подключенных к серверу
//grep '*' — ищет строки, содержащие символ '*', который указывает на загрузочный раздел
//wc -l — считает количество строк, найденных в предыдущем шаге

    
    FILE *fp;   //объявляем указатель fp, который будет хранить адрес в памяти, где находится информация о файле или потоке
    char result[10];   //создаем массив result, чтобы сохранить вывод команды
    
    fp = popen(command, "r"); //popen создает новый процесс, который выполняет команду, указанную в command
    if (fp == NULL) {
        perror("Ошибка!!!");  //если не удалось открыть процесс
        return -1;
    }

    fgets(result, sizeof(result), fp);  //читаем результат выполнения команды из потока fp и сохраняем его в result
    pclose(fp);

    return atoi(result);  //используем atoi для преобразования строки result в целое число, которое будет равно количеству строк с `*`, найденных в результате выполнения команды.
}


// функция для объединения содержимого двух файлов
bool gluing_new_files(char* path1, char* path2) {
    FILE *f1 = fopen(path1, "a");  // открываем для записи
    FILE *f2 = fopen(path2, "r");  //открываем для чтения
    if (!f1 || !f2) {
        printf("Ошибка открытия файлов %s\n", path2);
        return false;
    }
    char buf[256];

    while (fgets(buf, 256, f2) != NULL) { // этом цикле мы читаем строки из файла f2 и записываем их в файл f1
        fputs(buf, f1);    
    }
    fclose(f1);
    fclose(f2);
    return true;
}

//получение дампа памяти процесса
//Параметры функции: dir — это указатель на директорию, из которой мы читаем файлы, а path — это путь к этой директории.
void create_Dump(DIR* dir, char* path) {  
    FILE* res = fopen("res.txt", "w+"); //открываем файл для чтения и записи.Если он уже существует,то будет очищен,иначе — создан.
    fclose(res); //Затем закрываем его. Это подготовка файла для записи данных.
    struct dirent* ent;  //указатель на структуру dirent, которая содержит информацию о файле или подкаталоге в директории. Будем использовать для считывания содержимого директории
    char* file_path;     //указатель на строку, которая будет хранить полный путь ко всем файлам, найденным в директории
    while ((ent = readdir(dir)) != NULL) {   //readdir() считывает следующую запись о директории
//цикл читает содержимое директории, пока не достигнет конца, т.е. пока функция readdir не вернёт NULL, что указывает на конец содержимого директории
//readdir(dir) считывает следующую запись (файл или подкаталог) из директории, на которую указывает dir, и сохраняет её в ent
        asprintf(&file_path, "%s/%s", path, ent->d_name); 
//asprintf — это функция, которая форматирует строку и позволяет динамически выделять память для неё
//Здесь мы создаём строку, представляющую полный путь к файлу, соединяя path (путь к директории) и ent->d_name (имя файла или подкаталога)
//Результат записывается в file_path.
        if(!gluing_new_files("res.txt", file_path)) {
//Вызов функции gluing_new_files, чтобы объединить содержимое каждого файла (к которому мы пришли по file_path) с файлом res.txt
            return;   // Если эта функция 0, то выполнение create_Dump прерывается
        }
    }
    printf("Выполнено!\n");  //в конце выводим сообщение о выполнении работы
}




int main() {
    char buffer[BUFFER_SIZE]; // Массив для хранения введённой строки

    signal (SIGHUP, handle_SIGHUP);   //Установка обработчика сигнала SIGHUP   
// в другом терминале через ps aux смотрим PID. И далее kill -SIGHUP <PID> 

    while (1) {
        printf("\nВведите команду (для выхода - exit или \\q, либо Ctrl+D):\n");

        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) break; // Чтение строки ввода и выход при Ctrl+D

        buffer[strcspn(buffer, "\n")] = 0; // Удаление символа новой строки, если есть

        save_to_history(buffer); // Сохраняем команду в историю

        // Проверка на команды выхода
        if (strcmp(buffer, "exit") == 0 || strcmp(buffer, "\\q") == 0) break;

        // Обработка команды echo
        if (strncmp(buffer, "echo", 4) == 0) {
             if (str_size_without(buffer + 4)) printf("Введенная команда:%s\n", buffer + 4); // Печатаем все, что после echo
                else printf("Команды нет\n");
            continue;
        }

        // проверка на пустую строку
        if (str_size_without(buffer) == 0) {  // проверка на пустую строку
                printf("Команды нет\n"); 
                continue;
            }  

        // Обработка переменной окружения
        if (strncmp(buffer, "\\e ", 3) == 0) {
            print_env_variable(buffer + 3); // Вывод переменной окружения
            continue;
        }

        // Обработка проверки загрузочного диска
        if (strncmp(buffer, "\\l ", 3) == 0) {
            if (is_boot_disk(buffer + 3)) {
                printf("Диск %s является загрузочным.\n", buffer + 3);
            } else {
                printf("Диск %s не является загрузочным.\n", buffer + 3);
            }
            continue;
        }

        // Выполнение бинарника
         if(strncmp(buffer, "./", 2) == 0) {
      char* run_command = buffer + 2;
      run_command[strcspn(run_command, "\n")] = 0;
      executeCommand(run_command);
      printf("\n");
      continue;
    }

// /proc/[PID]/map_files — инструмент для получения информации о процессе и его использовании памяти
         if (strncmp(buffer, "\\proc ", 6) == 0) {
        char* path;
        asprintf(&path, "/proc/%s/map_files", buffer+6);
///proc — это виртуальная файловая система, которая содержит информацию о процессах и другой системной информации
//Внутри директории процесса содержится несколько файлов и подкаталогов, включая map_files, там находятся символические ссылки на файлы, которые загружены в адресное пространство процесса. Это позволяет пользователям и разработчикам легко видеть, какие файлы используются процессом и какие библиотеки загружены
        DIR* dir = opendir(path);
//Функция opendir() открывает директорию, на которую указывает path. Если директория существует и открывается успешно, dir получает указатель на открытую директорию.
        if (dir) {
            create_Dump(dir, path);
        }
        else {
            printf("Процесс не найден\n");
        }
        continue;
    }


        printf("Неизвестная команда: %s\n", buffer); // Если команда неизвестна
    }

    printf("Выход из программы :)\n");
    return 0; // Выходим из программы
}


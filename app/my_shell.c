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

#define BUFFER_SIZE 1024 
#define HISTORY_FILE "history.txt"

// Функция для записи истории команд в файл
void savetohistory(const char *buffer) {
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
void printenvvariable(const char *var) {
    char *value = getenv(var);
    if (value) {
        printf("%s\n", value);
    } else {
        printf("Переменная окружения не найдена.\n");
    }
}


void executeCommand(const char* command) {  // выполнение бинарника
  pid_t pid = fork();  // с помощью системного вызова создаем новый процесс
  if(pid == 0) {  // если процесс дочерний
    execlp(command, command, (char*)NULL); // ищем команду в каталогах PATH
    perror("execlp failed");  // если не нашли команду
    exit(1);
  } else if(pid < 0) {  // произошла ошибка при создании нового процесса 
    perror("fork failed");
  } else {
    wait(NULL);  // родительский процесс ждет завершения дочернего
  }
}



// Обработчик сигнала SIGHUP
void handle_SIGHUP(int signal) {
    if (signal == SIGHUP) {
        printf("Configuration reloaded\n");
        exit(0);
    }
}


// Функция для проверки загрузочного диска
int isbootdisk(const char *disk) {
    char command[256];
    snprintf(command, sizeof(command), "fdisk -l %s | grep '*' | wc -l", disk);
    
    // Выполняем команду в shell и получаем результат
    FILE *fp;
    char result[10];
    
    fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen");
        return -1;
    }

    fgets(result, sizeof(result), fp);
    pclose(fp);

    // Если результат больше 0, значит диск загрузочный
    return atoi(result) > 0;
}




//12 по 'mem <procid>' получить дамп памяти процесса
bool appendToFile(char* path1, char* path2) {
    FILE *f1 = fopen(path1, "a");
    FILE *f2 = fopen(path2, "r");
    if (!f1 || !f2) {
        printf("Error while reading file %s\n", path2);
        return false;
    }
    char buf[256];

    while (fgets(buf, 256, f2) != NULL) {
        fputs(buf, f1);
    }
    fclose(f1);
    fclose(f2);
    return true;
}

void makeDump(DIR* dir, char* path) {
    FILE* res = fopen("res.txt", "w+");
    fclose(res);
    struct dirent* ent;
    char* file_path;
    while ((ent = readdir(dir)) != NULL) {

        asprintf(&file_path, "%s/%s", path, ent->d_name); // asprintf работает
        if(!appendToFile("res.txt", file_path)) {
            return;
        }
    }
    printf("Dump completed!\n");
}




int main() {
    char buffer[BUFFER_SIZE]; // Массив для хранения введённой строки

    signal (SIGHUP, handle_SIGHUP);   //Установка обработчика сигнала SIGHUP   (kill -SIGHUP <PID> )

    while (1) {
        printf("\nВведите команду (для выхода - exit или \\q, либо Ctrl+D):\n");

        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) break; // Чтение строки ввода и выход при Ctrl+D

        buffer[strcspn(buffer, "\n")] = 0; // Удаление символа новой строки, если есть

        savetohistory(buffer); // Сохраняем команду в историю

        // Проверка на команды выхода
        if (strcmp(buffer, "exit") == 0 || strcmp(buffer, "\\q") == 0) break;

        // Обработка команды echo
        if (strncmp(buffer, "echo", 4) == 0) {
             if (str_size_without(buffer + 4)) printf("%s\n", buffer + 5); // Печатаем все, что после echo
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
            printenvvariable(buffer + 3); // Вывод переменной окружения
            continue;
        }

        // Обработка проверки загрузочного диска
        if (strncmp(buffer, "\\l ", 3) == 0) {
            if (isbootdisk(buffer + 3)) {
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

         if (strncmp(buffer, "\\proc ", 6) == 0) {
        char* path;
        asprintf(&path, "/proc/%s/map_files", buffer+6);

        DIR* dir = opendir(path);
        if (dir) {
            makeDump(dir, path);
        }
        else {
            printf("Process not found\n");
        }
        continue;
    }


        printf("Неизвестная команда: %s\n", buffer); // Если команда неизвестна
    }

    printf("Выход из программы :)\n");
    return 0; // Выходим из программы
}



/*------------------------------------------------------------------------------
Proyecto Shell de UNIX. Sistemas Operativos
Grados I. Inform�tica, Computadores & Software
Dept. Arquitectura de Computadores - UMA

Algunas secciones est�n inspiradas en ejercicios publicados en el libro
"Fundamentos de Sistemas Operativos", Silberschatz et al.

Para compilar este programa: gcc ProyectoShell.c ApoyoTareas.c -o MiShell
Para ejecutar este programa: ./MiShell
Para salir del programa en ejecuci�n, pulsar Control+D
------------------------------------------------------------------------------*/

#include "ApoyoTareas.h" // Cabecera del m�dulo de apoyo ApoyoTareas.c
 
#define MAX_LINE 256 // 256 caracteres por l�nea para cada comando es suficiente
#include <string.h>  // Para comparar cadenas de cars. (a partir de la tarea 2)

job *tareas;

void manejador (int senal){
  job *item;
  int status;
  int info;
  int pid_wait=0;
  enum status status_res;

  for (int i = 1; i <= list_size(tareas); i++){
    item = get_item_bypos(tareas, i);
    pid_wait = waitpid(item->pgid, &status, WUNTRACED | WNOHANG);
    if(pid_wait == item->pgid){
      status_res = analyze_status(status, &info);
      if(status_res == SUSPENDIDO){
        printf("\nComando %s ejecutado en segundo plano con PID %d ha suspendido su ejecucion\n", item->command, item->pgid);
        item->ground = DETENIDO;
      }else if(status_res == FINALIZADO){
        printf("\nComando %s ejecutado en segundo plano con PID %d ha finalizado su ejecucion\n", item->command, item->pgid);
        delete_job(tareas,item);
      }
    }
  }
}
// --------------------------------------------
//                     MAIN          
// --------------------------------------------

int main(void)
{
      char inputBuffer[MAX_LINE]; // B�fer que alberga el comando introducido
      int background;         // Vale 1 si el comando introducido finaliza con '&'
      char *args[MAX_LINE/2]; // La l�nea de comandos (de 256 cars.) tiene 128 argumentos como m�x
                              // Variables de utilidad:
      int pid_fork, pid_wait; // pid para el proceso creado y esperado
      int status;             // Estado que devuelve la funci�n wait
      enum status status_res; // Estado procesado por analyze_status()
      int info;		      // Informaci�n procesada por analyze_status()

      job *item;

      ignore_terminal_signals();
      signal(SIGCHLD, manejador);
      tareas = new_list("Tareas");

      while (1) // El programa termina cuando se pulsa Control+D dentro de get_command()
      {  		
        printf("COMANDO->");
        fflush(stdout);
        get_command(inputBuffer, MAX_LINE, args, &background); // Obtener el pr�ximo comando
        if (args[0]==NULL) continue; // Si se introduce un comando vac�o, no hacemos nada

        if (!strcmp (args[0], "cd")){
          chdir(args[1]);
          continue;
        }

        if (!strcmp (args[0], "logout")){
          exit(0);
        }
        
        pid_fork = fork();
        if(pid_fork>0){
          if(background==0){
            waitpid(pid_fork, &status, WUNTRACED);
            set_terminal(getpid());
            status_res=analyze_status(status, &info);
            if(status_res == SUSPENDIDO){
              item = new_job(pid_fork, args[0], DETENIDO);
              add_job(tareas,item);
              printf("\nComando %s ejecutado en primer plano con pid %d. Estado %s. Info: %d\n",
               args[0], pid_fork, status_strings[status_res],info);
            }else if(status_res == FINALIZADO){
              if(info!=255){
                printf("\nComando %s ejecutado en primer plano con pid %d. Estado %s. Info: %d\n",
                 args[0], pid_fork,status_strings[status_res], info);
              }
            }
          }else{
            item = new_job(pid_fork, args[0], SEGUNDOPLANO);
            add_job(tareas,item);
            printf("\nComando %s ejecutado en segundo plano con pid %d\n", args[0], pid_fork);
          }
        }else{
          new_process_group(getpid());
          if(background==0){
            set_terminal(getpid());
          }
          restore_terminal_signals();
          execvp(args[0], args);
          printf("\nError. Comando %s no encontrado.\n", args[0]);
          exit(-1);
        }
      }// end while
}



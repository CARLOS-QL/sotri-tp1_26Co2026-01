## Paso 02 - Uso del parámetro de Tarea y prioridad de tareas

### ¿Cómo usar el parámetro de una Tarea?

El parámetro de una tarea en FreeRTOS se utiliza para enviar información específica a cada instancia creada mediante `xTaskCreate()`.

Este parámetro corresponde al cuarto argumento de la función:

```c
xTaskCreate( task,
             "Task",
             stack,
             parameters,
             priority,
             handle );
```

## ¿Cómo cambiar la prioridad de una Tarea ya creada?

En FreeRTOS, la prioridad de una tarea ya creada puede modificarse dinámicamente utilizando la función:

```c
vTaskPrioritySet();
```

## Paso 03 - Observación

Se modificó la tarea `task_btn` para recibir una estructura de datos mediante el parámetro `parameters` de `xTaskCreate()`. De esta forma, se crearon dos instancias de la misma tarea, cada una asociada a un botón diferente.

Para ello, se crearon dos instancias de la misma tarea:

- `Task BTN 1`
- `Task BTN 2`

La primera instancia gestiona el botón `B1` y la segunda instancia gestiona el botón externo configurado como `B2`.

Al presionar cualquiera de los botones, la tarea correspondiente detecta el evento mediante la máquina de estados y envía el evento hacia `task_led`.

La función `task_btn_statechart()` recibe un puntero a esta estructura y utiliza dicha información para gestionar de forma independiente cada botón.

Se comprobó que ambas instancias de `task_btn` funcionan de manera independiente usando la misma función de tarea, diferenciándose únicamente por el parámetro recibido al momento de su creación.

# Paso 04 - Modificación de prioridad y múltiples instancias de task_led

Para este paso se modificó la prioridad de la tarea `task_led` con el objetivo de garantizar que fuera la primera tarea en ejecutarse dentro del sistema.

Inicialmente, las tareas fueron creadas con prioridades similares:

```c
(tskIDLE_PRIORITY + 1ul)
```

Para asegurar que `task_led` sea la primera tarea en ejecutarse, se modificó temporalmente su prioridad al momento de crearla con `xTaskCreate()`.

La prioridad de `task_led` se configuró por encima de `task_btn`, usando un valor mayor que `(tskIDLE_PRIORITY + 1ul)`.

Luego de compilar y depurar el proyecto, se observó que `task_led` se ejecuta antes que las tareas de botones, ya que FreeRTOS selecciona primero la tarea lista con mayor prioridad.

Posteriormente, se recuperaron las prioridades originales, dejando nuevamente las tareas con prioridad similar.

También se crearon dos instancias de `task_led`:

- `Task LED 1`
- `Task LED 2`

Cada instancia recibe una estructura diferente mediante el parámetro de tarea.

La primera instancia utiliza `task_led_dta`, asociada al LED `LD2`, y la segunda instancia utiliza `task_led_dta_2`, asociada al LED `LD3`.

```c
ret = xTaskCreate(task_led,
                  "Task LED 1",
                  (2 * configMINIMAL_STACK_SIZE),
                  &task_led_dta,
                  (tskIDLE_PRIORITY + 1ul),
                  &h_task_led);

ret = xTaskCreate(task_led,
                  "Task LED 2",
                  (2 * configMINIMAL_STACK_SIZE),
                  &task_led_dta_2,
                  (tskIDLE_PRIORITY + 1ul),
                  &h_task_led_2);
```

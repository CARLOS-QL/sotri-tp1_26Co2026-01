
# Análisis Técnico del Firmware: STM32, HAL y FreeRTOS

Este documento detalla el análisis del funcionamiento y el flujo de ejecución del código fuente del proyecto basado en el microcontrolador **STM32F446xx** (núcleo ARM Cortex-M4), utilizando la capa de abstracción de hardware (HAL) de STMicroelectronics y el sistema operativo en tiempo real (RTOS) **FreeRTOS**.

---

## 1. Funcionamiento General del Código Fuente por Archivo

* **`startup_stm32f446retx.s`**: Es el archivo de arranque escrito en ensamblador. Define la tabla de vectores de interrupción (`g_pfnVectors`), asigna el puntero de pila inicial (`_estack`) y contiene el manejador de reinicio físico del chip (`Reset_Handler`). Su propósito principal es preparar el entorno de memoria física antes de ceder el control al código en C.
* **`main.c`**: Es el punto de entrada principal de la aplicación en C. Contiene la inicialización del hardware base (relojes, GPIO, UART, Timers), la creación de la tarea inicial (`defaultTask`) mediante la API CMSIS-RTOS y el arranque del planificador de FreeRTOS (`osKernelStart()`).
* **`stm32f4xx_it.c`**: Centraliza los manejadores de interrupción de hardware (ISRs) del sistema. Mapea las excepciones de la arquitectura Cortex-M (como fallos de hardware o fallos de memoria) y las interrupciones de periféricos como el `TIM1` y el `TIM2` hacia las rutinas de servicio de la HAL.
* **`FreeRTOSConfig.h`**: Contiene las directivas de preprocesador que adaptan el Kernel de FreeRTOS a las capacidades del hardware del microcontrolador. Regula el comportamiento del planificador (preemoción, uso de hooks, tamaño del Heap, prioridades máximas) y remapea los vectores de excepción del núcleo.
* **`freertos.c`**: Implementa la lógica de las tareas y los ganchos (*hooks*) de la aplicación. Define funciones de soporte requeridas por el Kernel, como la asignación de memoria estática para la tarea Idle (`vApplicationGetIdleTaskMemory`) y las respuestas ante desbordamientos de pila o ticks de reloj.

---

## 2. Análisis de la Evolución de Temporizadores y Relojes (SysTick & SystemCoreClock)

La gestión del tiempo y la frecuencia mutan dinámicamente conforme el microcontrolador avanza desde el reset físico hasta la ejecución multi-tarea.

Este apartado describe analíticamente cómo mutan las variables temporales del sistema desde el encendido del silicio hasta el control por el Kernel de tiempo real.

## Tabla de Estados Temporales

| Fase de Ejecución | Valor de `SystemCoreClock` (Software) | Estado y Registro `LOAD` de `SysTick` (Hardware) | Propósito y Control del Vector |
| :--- | :--- | :--- | :--- |
| **Reset_Handler** | 16,000,000 Hz (Reloj HSI Nativo) | Deshabilitado (Registros en `0`) | Hardware crudo, sin base de tiempo activa. |
| **HAL_Init()** | 16,000,000 Hz | Habilitado; `LOAD` = 15,999 (1 ms) | Base de tiempo provisional para las funciones HAL. |
| **SystemClock_Config()** | 180,000,000 Hz (Acelerado por PLL) | Habilitado; `LOAD` = 179,999 (1 ms) | Sincronización automática de la HAL a la velocidad máxima. |
| **osKernelStart()** | 180,000,000 Hz (Fijo y Constante) | Habilitado; Reconfigurado por FreeRTOS a 1 kHz | Control cedido a `xPortSysTickHandler` (RTOS Tick). |

### Puntos Clave del Comportamiento:
1. **Aislamiento Automático**: La variable `SystemCoreClock` le indica a FreeRTOS en `FreeRTOSConfig.h` la velocidad exacta del núcleo de forma dinámica mediante `configCPU_CLOCK_HZ`.
2. **Desviación del Flujo Principal**: El loop `while(1)` final de `main.c` no llega a ejecutarse porque el Planificador (*Scheduler*) de FreeRTOS detiene el avance lineal de la función `main()` para dar inicio a la multitarea en los hilos definidos.


## 3. Comportamiento del Programa: Flujo Secuencial desde `Reset_Handler` hasta antes de `while(1)`

El comportamiento del sistema durante la fase de arranque (boot) sigue una secuencia lineal y determinista que transita desde la inicialización física del silicio a bajo nivel hasta la preparación del entorno multitarea gobernado por el Kernel de FreeRTOS. Este flujo se divide claramente en dos fases secuenciales:

---

### A. Fase de Inicialización a Bajo Nivel (Ensamblador)
Al energizarse el microcontrolador o liberarse el pin físico de **RESET**, el hardware del procesador lee la dirección de memoria `0x00000000` para cargar el puntero de pila y la dirección `0x00000004` para saltar a la primera instrucción ejecutable: la subrutina **`Reset_Handler`** ubicada en el archivo `startup_stm32f446retx.s`.

A partir de este punto, el comportamiento secuencial es el siguiente:

1. **Configuración del Stack Pointer (SP):** Se establece el registro `sp` apuntando al final de la memoria RAM (`_estack`), delimitando la región segura donde se almacenarán las variables locales y las direcciones de retorno de las funciones en C.
2. **Llamada a `SystemInit`:** Se realiza un salto hacia esta función externa provista por el CMSIS de ST. Su comportamiento consiste en resetear el periférico RCC (Reset and Clock Control) a su estado seguro por defecto, apagando osciladores externos y asegurando que el microcontrolador empiece a operar de manera estable con su oscilador interno básico (HSI de 16 MHz).
3. **Ciclo de Copiado de Datos (`CopyDataInit`):** El código en ensamblador ejecuta un copiado en bloque de memoria. Toma los valores iniciales de todas las variables globales y estáticas no nulas (almacenadas en la memoria Flash no volátil, entre `_sidata` y `_edata`) y las escribe en sus direcciones operativas de la memoria SRAM (`_sdata`).
4. **Ciclo de Inicialización a Cero (`FillZerobss`):** Inmediatamente después, el procesador barre el segmento `.bss` de la memoria RAM (comprendido entre `_sbss` y `_ebss`). Su comportamiento es escribir un valor de cero (`0`) en cada una de estas posiciones, garantizando que todas las variables globales no inicializadas explícitamente en el código arranquen con un valor limpio y conocido.
5. **Constructores y Salto a C:** Se invoca a la subrutina `__libc_init_array` para procesar los constructores estáticos o inicializaciones de bibliotecas estándar de C, para finalmente realizar el salto definitivo al punto de entrada del software mediante la instrucción `bl main`.

---

### B. Fase de Configuración de la Aplicación y del Kernel (C)
Una vez transferido el control a la función `main(void)` en `main.c`, el programa sigue un comportamiento estrictamente lineal para levantar las capas de abstracción (HAL) y preparar las estructuras de FreeRTOS:

1. **Inicialización del Entorno de Hardware de ST (`HAL_Init`):** Configura la memoria de pre-búsqueda (Prefetch), la caché de instrucciones y datos, define el agrupamiento de prioridades de interrupción en el NVIC y enciende una base de tiempo de hardware provisional de 1 ms basada en el periférico `SysTick`.
2. **Aceleración Eléctrica del Sistema (`SystemClock_Config`):** El programa conmuta las fuentes de reloj. Enciende el bucle de enganche de fase (**PLL**), configurando el oscilador HSI como fuente de entrada, multiplica la frecuencia y distribuye los nuevos divisores para que el núcleo y los buses internos (`AHB`, `APB1`, `APB2`) comiencen a operar a su velocidad máxima de rendimiento (180 MHz).
3. **Inicialización de Periféricos Propios (`MX_...`):** Se mandan llamar secuencialmente las funciones encargadas de mapear los registros de hardware del proyecto:
   * `MX_GPIO_Init()`: Configura el modo, velocidad y de resistencias de pull-up/pull-down de los pines de entrada y salida digital.
   * `MX_USART2_UART_Init()`: Inicializa el periférico de comunicación serie UART2 (usado comúnmente para salida de telemetría y logs de depuración).
   * `MX_TIM2_Init()`: Configura las estructuras básicas, prescaladores y periodos de conteo del Temporizador 2 de hardware.
4. **Habilitación de Interrupciones de Hardware:** Se ejecuta explícitamente la instrucción `HAL_TIM_Base_Start_IT(&htim2)`. El comportamiento del microcontrolador aquí es activar los bits de interrupción por desborde (*Update Interrupt*) en el hardware del `TIM2`, permitiendo que el periférico comience a interrumpir autónomamente a la CPU para alimentar las estadísticas del sistema operativo.
5. **Inicialización de la Lógica de Usuario:** Se invoca a la función `app_init()` para realizar configuraciones previas a la inicialización del planificador, como la preparación de buffers o la calibración inicial de sensores.
6. **Creación de Tareas de FreeRTOS:** El programa interactúa con la API de abstracción de CMSIS-RTOS para reservar bloques de memoria y registrar hilos de ejecución. Específicamente ejecuta `osThreadCreate(osThread(defaultTask), NULL)` para instanciar el hilo principal de procesamiento (`defaultTask`).
7. **Lanzamiento del Kernel de Tiempo Real (`osKernelStart`):** Es la última función lineal ejecutada por el hilo principal de configuración. Su comportamiento interno consiste en deshabilitar temporalmente interrupciones configurables, inicializar las listas de tareas listas (*Ready Lists*), crear la tarea de fondo *Idle*, configurar los punteros de pila de proceso (PSP) para aislar las memorias de cada tarea, reconfigurar el `SysTick` como el latido maestro del RTOS (1 kHz) y volver a habilitar las interrupciones globales de la CPU.

---

### C. El Punto de Inflexión Crítico: ¿Por qué nunca se llega al `while(1)`?

> [!CRITICAL]
> Al ejecutarse las instrucciones de finalización dentro de la rutina `osKernelStart()`, el control del Contador de Programa (`PC`) del procesador es **cedido por completo y de forma definitiva al Planificador (*Scheduler*) de FreeRTOS**.

El planificador fuerza un cambio de contexto inicial por hardware que extrae de la memoria el contexto guardado de la tarea con mayor prioridad creada (`defaultTask`) y deposita su dirección en el registro del procesador. A partir de ese microsegundo, el microcontrolador pasa a un paradigma de ejecución concurrente basado en prioridades y cortes de tiempo. 

La secuencia lineal y estructurada que arrastraba la función `main()` **se interrumpe permanentemente**, haciendo que en condiciones de operación normales, **el bucle infinito `while(1)` ubicado al final de `main.c` sea un código inalcanzable (*dead code*)**.

---

## 4. Interacción del Timer 2 (`TIM2`) con la HAL del Proyecto STM32

El temporizador de propósito general **`TIM2`** asume un rol de aislamiento crítico dentro de la arquitectura de este firmware. Su configuración responde a una necesidad de diseño de STMicroelectronics para desacoplar de forma segura las dependencias temporales de la HAL frente al sistema operativo.

### A. ¿Cómo interactúa `TIM2` con la HAL?
La interacción se realiza por hardware mediante interrupciones periódicas por desborde (*Overflow / Update Event*) y se gestiona a través de la siguiente cadena de llamadas de software:

1. **Configuración Inicial:** En `main.c`, la función `MX_TIM2_Init()` parametriza el prescaler y el período del temporizador. Inmediatamente después, la instrucción `HAL_TIM_Base_Start_IT(&htim2)` activa los bits físicos de control de interrupciones en el periférico.
2. **Disparo de la ISR:** Cada vez que el contador interno de `TIM2` llega al límite de su período y se reinicia, el hardware del microcontrolador suspende momentáneamente la ejecución actual y salta al vector de interrupción global en `stm32f4xx_it.c`:
   ```c
   void TIM2_IRQHandler(void)
   {
     HAL_TIM_IRQHandler(&htim2);
   }

## 5. Análisis y Funcionamiento de la Capa de Aplicación (`app.c`, `task_btn.c`, `task_led.c`, `task_led_interface.c` y `freertos.c`)

Esta sección describe el comportamiento y la arquitectura del software del usuario en el espacio de tareas de FreeRTOS. El diseño modular implementa una arquitectura orientada a eventos basada en **Máquinas de Estados Finitos (MEF / Statecharts)** con una clara separación de responsabilidades para procesar la lectura de un botón físico (antirrebote) y controlar los efectos visuales de un LED.

---

### A. Estructura de Control y Estado Dinámico

El firmware utiliza variables globales estructuradas para empaquetar el estado, evento, flags y temporizaciones de cada módulo, garantizando la ocultación de datos en lo posible mediante abstracciones de interfaces:

* **`task_btn_dta` (`task_btn.c`)**: Controla la MEF del botón. Almacena el estado actual de la entrada digital (`ST_BTN_UP`, `ST_BTN_FALLING`, `ST_BTN_DOWN`, `ST_BTN_RISING`), el evento detectado y las marcas de tiempo (`tick`) calculadas mediante `xTaskGetTickCount()` para discriminar los rebotes mecánicos.
* **`task_led_dta` (`task_led.c` / `task_led_interface.c`)**: Controla el estado del LED (`ST_LED_OFF`, `ST_LED_BLINK`). Contiene la configuración de los descriptores físicos del hardware (puerto GPIO y pin), un flag booleano de sincronización y el registro del evento recibido desde otros módulos.

---

### B. Análisis Detallado del Funcionamiento por Archivo

#### 1. Archivo `app.c` (Inicializador de la Aplicación)
Actúa como el orquestador e instalador de la capa de lógica de usuario de alto nivel.
* **`app_init()`**: Es invocada desde el `main.c` antes del arranque del Kernel. Su función es doble:
  1. Llama a las subrutinas de inicialización de los atributos locales de los componentes: `task_btn_init()` y `task_led_init()`.
  2. Crea las tareas concurrentes nativas de FreeRTOS mediante la API `xTaskCreate()`. Instancia la tarea del botón (`task_btn`) y la tarea del LED (`task_led`), ambas con una prioridad inicial de `tskIDLE_PRIORITY + 1ul`. Cada creación es validada de forma estricta mediante la macro de aserción del sistema operativo `configASSERT(pdPASS == ret)`.

#### 2. Archivo `task_btn.c` (MEF del Botón con Antirrebote)
Implementa un hilo de ejecución cíclico infinito que lee periódicamente el pin de hardware asignado al botón.
* **Funcionamiento Cíclico**: La tarea se ejecuta dentro de un bucle que invoca a `task_btn_statechart()` y luego se suspende voluntariamente durante un tiempo determinado (`vTaskDelay(20 / portTICK_RATE_HZ)`), liberando la CPU para que otras tareas operen de forma eficiente.
* **Lógica del Statechart (`task_btn_statechart`)**: Implementa un algoritmo de filtrado de rebotes por software mediante una máquina de estados:
  * `ST_BTN_UP`: Estado de reposo (botón liberado). Al detectar el evento de presión física (`EV_BTN_DOWN`), guarda el tick de tiempo actual y cambia al estado de transición `ST_BTN_FALLING`.
  * `ST_BTN_FALLING`: Espera de estabilización. Evalúa si transcurrió el tiempo de guarda para el filtro de rebotes mecánico (`DEL_BTN_MAX`). Si el botón sigue presionado una vez cumplido el plazo, confirma la pulsación, **envía el evento de parpadeo a la interfaz del LED (`put_event_task_led(EV_LED_BLINK)`)**, e ingresa formalmente al estado `ST_BTN_DOWN`.
  * `ST_BTN_DOWN`: El botón permanece oprimido. Al ser liberado por el usuario, cambia el estado a `ST_BTN_RISING` registrando la marca de tiempo para validar el flanco de subida.

#### 3. Archivo `task_led_interface.c` (Capa de Abstracción de Eventos)
Este archivo proporciona una interfaz limpia para la comunicación entre hilos (Inter-Task Communication), evitando que el módulo del botón manipule directamente las estructuras o registros internos del LED.
* **`put_event_task_led(task_led_ev_t event)`**: Es la función pública que invoca la tarea del botón. Su comportamiento consiste en inyectar el evento solicitado (`EV_LED_BLINK` o `EV_LED_OFF`) dentro de la estructura interna del LED y activar un indicador lógico (`task_led_dta.flag = true`), actuando como un buzón de mensajería asíncrona ligero.

#### 4. Archivo `task_led.c` (MEF de Control del LED)
Implementa el hilo de ejecución encargado de modificar el estado físico del pin de salida digital conectado al LED de la placa de desarrollo.
* **Funcionamiento Cíclico**: Corre de forma infinita, evaluando constantemente el flujo de su propio Statechart (`task_led_statechart()`). Al igual que el botón, se bloquea de manera controlada utilizando `vTaskDelay()` para mantener el determinismo del sistema.
* **Lógica del Statechart (`task_led_statechart`)**:
  * `ST_LED_OFF`: Se encuentra con el pin del LED apagado. Al verificar que el flag de interfaz es verdadero y que el evento guardado corresponde a `EV_LED_BLINK`, consume el evento (limpia el flag), guarda el tick de inicio y cambia a `ST_LED_BLINK`, encendendo el LED en el hardware físico a través de `HAL_GPIO_WritePin()`.
  * `ST_LED_BLINK`: Ejecuta una temporización por software dentro de la tarea. Compara la diferencia entre el tiempo actual (`xTaskGetTickCount()`) y el tick guardado frente al retardo configurado (`DEL_LED_MAX`). Al expirar el plazo de milisegundos, invierte el estado físico del pin (conmutación / *Toggle*) y reinicia la marca de tiempo, generando un parpadeo constante y limpio controlado por el sistema operativo.

#### 5. Archivo `freertos.c` (Ganchos del Kernel y Métricas)
Contiene las funciones globales de enganche (*Hooks*) que FreeRTOS invoca de forma automática ante eventos críticos del sistema operativo:
* **`vApplicationIdleHook()`**: Se ejecuta de forma repetitiva cuando ninguna otra tarea del usuario está lista para procesar (el microcontrolador entra en reposo). El firmware incrementa la variable global `g_task_idle_cnt++`, lo cual sirve para auditar cuántos ciclos libres conserva el procesador.
* **`vApplicationTickHook()`**: Es invocada de manera síncrona en el contexto de la rutina de servicio de interrupción (ISR) de `SysTick` cada 1 milisegundo. Incrementa el contador global `g_app_tick_cnt++` para llevar un registro absoluto del tiempo de ejecución de la aplicación.

---

### C. Diagrama de Interacción y Flujo de Eventos

El siguiente diagrama detalla cómo los eventos de hardware fluyen a través de las diferentes capas de software y tareas independientes creadas en `app.c`:

## Diagrama de Interacción y Flujo de Eventos

El siguiente diagrama detalla cómo los eventos de hardware fluyen de forma asíncrona a través de las diferentes capas de software y tareas independientes concurrentes creadas por el firmware:

```mermaid
graph TD
    %% Definición de Estilos
    classDef hw fill:#f9f,stroke:#333,stroke-width:2px;
    classDef task fill:#bbf,stroke:#333,stroke-width:1px;
    classDef inter fill:#fbf,stroke:#333,stroke-width:1px;

    %% Nodos del Diagrama
    HW_BTN[HARDWARE: Botón Físico]:::hw
    TASK_BTN(Tarea: task_btn <br> Escaneo cada 20ms):::task
    INTERFACE[Capa de Interfaz: task_led_interface.c <br> put_event_task_led]:::inter
    TASK_LED(Tarea: task_led <br> Control de la MEF del LED):::task
    HW_LED[HARDWARE: Pin del LED]:::hw

    %% Conexiones y Eventos
    HW_BTN -->|Pulsación Mecánica / Flanco Caída| TASK_BTN
    TASK_BTN -->|Filtra Rebotes Mecánicos y confirma EV_BTN_DOWN| TASK_BTN
    TASK_BTN -->|Llamada no bloqueante: EV_LED_BLINK| INTERFACE
    INTERFACE -->|Modifica flag = true y estructura| TASK_LED
    TASK_LED -->|Calcula retardo DEL_LED_MAX| TASK_LED
    TASK_LED -->|Llamada a HAL_GPIO_WritePin / Conmutación| HW_LED
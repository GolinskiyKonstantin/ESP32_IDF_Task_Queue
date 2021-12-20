#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"
#include "time.h"

// указатель на функцию через который будем передавать функцию в задачу
// создал статическую чтоб не размещать в стеке ( рекомендации с документации на ESP32-IDF )
static void (*Pointer_PrintLogCallback)(void);

// прототип функции для первой задачи ( в которой инкрементируем счетчик и данные помещаем в очередь )
void vTaskCode_Count( void * pvParameters );

// прототип функции для второй задачи ( в которой вызываем функцию каллбек )
void vTaskCode_Log( void * pvParameters );

// прототип функции каллбек ( которая из очереди достает данные и печатает )
void PrintLogCallback(void);

// Хендлер для задачи инкремента счетчика
TaskHandle_t xHandle_TaskCount = NULL;

// Хендлер для задачи логирования
TaskHandle_t xHandle_TaskLog = NULL;

// Хендлер для очереди
QueueHandle_t xQueue = NULL;

// описываем структуру для отправки и приема данных ( можно просто массив или переменные но так удобнее )
typedef struct
{
	uint32_t count;				// для хранения счетчика
	uint32_t time_log;			// для хранения времени которое прошло между текущим и предыдущем отправки данных в очередь
}QueueMess;


void app_main(void)
{
	// привязываем к указателю адресс функции которую будем передавать в качестве аргумента при создании задачи
	Pointer_PrintLogCallback = PrintLogCallback;

	//создаем очередь на 10 эллементов типа QueueMess ( на прозапас 10 ) и присваевываем Дескриптор созданной очереди
	xQueue = xQueueCreate( 10, sizeof( QueueMess ) );

	// проверяем Дескриптор созданной очереди ( если он равен NULL тогда очередь не была создана )
	if( xQueue != NULL )
	{
		ESP_LOGW("WARNING", "Create Queue => OK");	// если "Успех" то пишем в логе
	}
	else
	{
		ESP_LOGE("ERROR", "Error Create Queue");	// если "Ошибка" то пишем в логе
	}
	// либо проверяем так:
	//configASSERT( xQueue );	// Проверяем, что дескриптор не равен NULL, очередь создана, иначе перехват этой ошибки

	// создаем задачу ( в которой инкрементируем счетчик и данные помещаем в очередь )
	BaseType_t xReturnedCount = xTaskCreate(
			vTaskCode_Count, 			// Указатель на функцию ввода задачи
			"Task_Count", 				// Описательное имя задачи
			1024*2, 					// Размер стека задач, заданный как количество байтов
			NULL, 						// Указатель, который будет использоваться в качестве параметра для создаваемой задачи.
			tskIDLE_PRIORITY, 			// Приоритет, с которым должна выполняться задача
			&xHandle_TaskCount			// Используется для возврата дескриптора, по которому можно ссылаться на созданную задачу
			);

	// проверяем была ли создана задача ( если он равен NULL тогда задача не была создана )
	if( xReturnedCount == pdPASS)
	{
		ESP_LOGW("WARNING", "Create Task (vTaskCode_Count) => OK"); // если "Успех" то пишем в логе
	}
	else
	{
		ESP_LOGE("ERROR", "Error Create Task => vTaskCode_Count"); // если "Ошибка" то пишем в логе
	}
	// либо проверяем так:
	//configASSERT( xHandle_TaskCount );	// Проверяем, что дескриптор не равен NULL, задача создана, иначе перехват этой ошибки

	// создаем задачу ( в которой вызываем функцию каллбек )
	BaseType_t xReturnedLog = xTaskCreate(
			vTaskCode_Log, 				// Указатель на функцию ввода задачи
			"Task_Log", 				// Описательное имя задачи
			1024*2, 					// Размер стека задач, заданный как количество байтов
			Pointer_PrintLogCallback, 	// Указатель, который будет использоваться в качестве параметра для создаваемой задачи.
			tskIDLE_PRIORITY, 			// Приоритет, с которым должна выполняться задача
			&xHandle_TaskLog			// Используется для возврата дескриптора, по которому можно ссылаться на созданную задачу
			);

	// проверяем была ли создана задача ( если он равен NULL тогда задача не была создана )
	if( xReturnedLog == pdPASS)
	{
		ESP_LOGW("WARNING", "Create Task (vTaskCode_Log) => OK"); // если "Успех" то пишем в логе
	}
	else
	{
		ESP_LOGE("ERROR", "Error Create Task => vTaskCode_Log"); // если "Ошибка" то пишем в логе
	}
	// либо проверяем так:
	//configASSERT( xHandle_TaskLog );	// Проверяем, что дескриптор не равен NULL, задача создана, иначе перехват этой ошибки

}


//===================================  Функции =====================================================================//

// функция калбек которая читает данные из очереди и печатает их в лог
void PrintLogCallback(void){

	  // создаем структуру для чтения данных из очереди
	  static QueueMess get_Mess;

	  // при каждой итерации обнуляем данные
	  get_Mess.count = 0;
	  get_Mess.time_log = 0;

	  // смотрим сколько сообщений хранится в очереди ( если больше 1 то считываем ),а также проверяем есть ли сама очередь
	  // функция uxQueueMessagesWaiting возвращает кол-во сообщений которое хранится на данный момент в очереди
	  if(  uxQueueMessagesWaiting(xQueue) && xQueue != NULL )
   	  {
		  // читаем данные из очереди ( указав в качестве параметров дескриптор очереди адрес структуры и кол-во тиков для попытки считать данные
		  // если функция xQueueReceive вернула pdTRUE значит данные успешно были считаны
		  if( xQueueReceive( xQueue, &get_Mess, ( TickType_t ) 10 ) != pdTRUE )
		  {
			  ESP_LOGE("ERROR", "Error Queue Receive");	// если чтение из очереди не удалась
		  }
		  else
		  {
			  // если считали то печатаем в лог сам счетчик и время между текущей и предыдущей отправки в очередь данных ( время в микросекундах )
			  ESP_LOGI("LOG", "Count => %u  time => %u us", get_Mess.count, get_Mess.time_log );
		  }
	  }
}


// функция задачи ( в которой инкрементируем счетчик и данные помещаем в очередь )
void vTaskCode_Count( void * pvParameters )
{
	  // создаем структуру для записи данных в очереди
	  static QueueMess set_Mess;

	  // создаем переменную в которой будем хранить предыдушее время записи данных в очередь
	  static uint32_t previos_Time = 0;

	  // обнуляем счетчик
	  set_Mess.count = 0;

	  for( ;; )
	  {
		  // смотрим сколько свободных мест в очереди ( если их больше 1 то записываем ),а также проверяем есть ли сама очередь
		  // функция uxQueueSpacesAvailable возвращает кол-во свободных мест в очереди
		  if( uxQueueSpacesAvailable(xQueue) && xQueue != NULL )
		  {
			  // расчитываем время между предыдушим и текущим отправки данных в очередь
			  set_Mess.time_log = (uint32_t)esp_timer_get_time () - previos_Time;

			  // отправляем данные в очередь ( для етого указываем дескриптор очереди, адрес структуры где хранятся наши данные,
			  // кол-во тиков для попытки отправить данные, а также в какую часть очереди записываем данные
			  // если данные успешно отправлены то функция uxQueueSpacesAvailable вернет pdTRUE
			  if( xQueueGenericSend( xQueue, ( void * ) &set_Mess, ( TickType_t ) 10, queueSEND_TO_BACK ) != pdTRUE )
			  {
				  ESP_LOGE("ERROR", "Error Queue Send");	// если отправка в очередь не удалась
			  }
			  else
			  {
				  // если удалась то записываем время для расчета в следующей итерации
				  previos_Time = (uint32_t)esp_timer_get_time ();
			  }
		  }

		  // увеличиваем счетчик на 1
		  set_Mess.count++;

		  // Останавливает задачу на указанное время
		  vTaskDelay(5000 / portTICK_PERIOD_MS);
	  }
	  //vTaskDelete( NULL );
}


// функция задачи ( в которой вызываем функцию каллбек )
void vTaskCode_Log( void * pvParameters )
{
	  for( ;; )
	  {
		  // вызываем фукцию для печати данных в лог
		  Pointer_PrintLogCallback();

		  // Останавливает задачу на указанное время
		  vTaskDelay(10 / portTICK_PERIOD_MS);
	  }
	  //vTaskDelete( NULL );
}



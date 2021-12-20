#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"
#include "time.h"

// ��������� �� ������� ����� ������� ����� ���������� ������� � ������
// ������ ����������� ���� �� ��������� � ����� ( ������������ � ������������ �� ESP32-IDF )
static void (*Pointer_PrintLogCallback)(void);

// �������� ������� ��� ������ ������ ( � ������� �������������� ������� � ������ �������� � ������� )
void vTaskCode_Count( void * pvParameters );

// �������� ������� ��� ������ ������ ( � ������� �������� ������� ������� )
void vTaskCode_Log( void * pvParameters );

// �������� ������� ������� ( ������� �� ������� ������� ������ � �������� )
void PrintLogCallback(void);

// ������� ��� ������ ���������� ��������
TaskHandle_t xHandle_TaskCount = NULL;

// ������� ��� ������ �����������
TaskHandle_t xHandle_TaskLog = NULL;

// ������� ��� �������
QueueHandle_t xQueue = NULL;

// ��������� ��������� ��� �������� � ������ ������ ( ����� ������ ������ ��� ���������� �� ��� ������� )
typedef struct
{
	uint32_t count;				// ��� �������� ��������
	uint32_t time_log;			// ��� �������� ������� ������� ������ ����� ������� � ���������� �������� ������ � �������
}QueueMess;


void app_main(void)
{
	// ����������� � ��������� ������ ������� ������� ����� ���������� � �������� ��������� ��� �������� ������
	Pointer_PrintLogCallback = PrintLogCallback;

	//������� ������� �� 10 ���������� ���� QueueMess ( �� �������� 10 ) � ������������� ���������� ��������� �������
	xQueue = xQueueCreate( 10, sizeof( QueueMess ) );

	// ��������� ���������� ��������� ������� ( ���� �� ����� NULL ����� ������� �� ���� ������� )
	if( xQueue != NULL )
	{
		ESP_LOGW("WARNING", "Create Queue => OK");	// ���� "�����" �� ����� � ����
	}
	else
	{
		ESP_LOGE("ERROR", "Error Create Queue");	// ���� "������" �� ����� � ����
	}
	// ���� ��������� ���:
	//configASSERT( xQueue );	// ���������, ��� ���������� �� ����� NULL, ������� �������, ����� �������� ���� ������

	// ������� ������ ( � ������� �������������� ������� � ������ �������� � ������� )
	BaseType_t xReturnedCount = xTaskCreate(
			vTaskCode_Count, 			// ��������� �� ������� ����� ������
			"Task_Count", 				// ������������ ��� ������
			1024*2, 					// ������ ����� �����, �������� ��� ���������� ������
			NULL, 						// ���������, ������� ����� �������������� � �������� ��������� ��� ����������� ������.
			tskIDLE_PRIORITY, 			// ���������, � ������� ������ ����������� ������
			&xHandle_TaskCount			// ������������ ��� �������� �����������, �� �������� ����� ��������� �� ��������� ������
			);

	// ��������� ���� �� ������� ������ ( ���� �� ����� NULL ����� ������ �� ���� ������� )
	if( xReturnedCount == pdPASS)
	{
		ESP_LOGW("WARNING", "Create Task (vTaskCode_Count) => OK"); // ���� "�����" �� ����� � ����
	}
	else
	{
		ESP_LOGE("ERROR", "Error Create Task => vTaskCode_Count"); // ���� "������" �� ����� � ����
	}
	// ���� ��������� ���:
	//configASSERT( xHandle_TaskCount );	// ���������, ��� ���������� �� ����� NULL, ������ �������, ����� �������� ���� ������

	// ������� ������ ( � ������� �������� ������� ������� )
	BaseType_t xReturnedLog = xTaskCreate(
			vTaskCode_Log, 				// ��������� �� ������� ����� ������
			"Task_Log", 				// ������������ ��� ������
			1024*2, 					// ������ ����� �����, �������� ��� ���������� ������
			Pointer_PrintLogCallback, 	// ���������, ������� ����� �������������� � �������� ��������� ��� ����������� ������.
			tskIDLE_PRIORITY, 			// ���������, � ������� ������ ����������� ������
			&xHandle_TaskLog			// ������������ ��� �������� �����������, �� �������� ����� ��������� �� ��������� ������
			);

	// ��������� ���� �� ������� ������ ( ���� �� ����� NULL ����� ������ �� ���� ������� )
	if( xReturnedLog == pdPASS)
	{
		ESP_LOGW("WARNING", "Create Task (vTaskCode_Log) => OK"); // ���� "�����" �� ����� � ����
	}
	else
	{
		ESP_LOGE("ERROR", "Error Create Task => vTaskCode_Log"); // ���� "������" �� ����� � ����
	}
	// ���� ��������� ���:
	//configASSERT( xHandle_TaskLog );	// ���������, ��� ���������� �� ����� NULL, ������ �������, ����� �������� ���� ������

}


//===================================  ������� =====================================================================//

// ������� ������ ������� ������ ������ �� ������� � �������� �� � ���
void PrintLogCallback(void){

	  // ������� ��������� ��� ������ ������ �� �������
	  static QueueMess get_Mess;

	  // ��� ������ �������� �������� ������
	  get_Mess.count = 0;
	  get_Mess.time_log = 0;

	  // ������� ������� ��������� �������� � ������� ( ���� ������ 1 �� ��������� ),� ����� ��������� ���� �� ���� �������
	  // ������� uxQueueMessagesWaiting ���������� ���-�� ��������� ������� �������� �� ������ ������ � �������
	  if(  uxQueueMessagesWaiting(xQueue) && xQueue != NULL )
   	  {
		  // ������ ������ �� ������� ( ������ � �������� ���������� ���������� ������� ����� ��������� � ���-�� ����� ��� ������� ������� ������
		  // ���� ������� xQueueReceive ������� pdTRUE ������ ������ ������� ���� �������
		  if( xQueueReceive( xQueue, &get_Mess, ( TickType_t ) 10 ) != pdTRUE )
		  {
			  ESP_LOGE("ERROR", "Error Queue Receive");	// ���� ������ �� ������� �� �������
		  }
		  else
		  {
			  // ���� ������� �� �������� � ��� ��� ������� � ����� ����� ������� � ���������� �������� � ������� ������ ( ����� � ������������� )
			  ESP_LOGI("LOG", "Count => %u  time => %u us", get_Mess.count, get_Mess.time_log );
		  }
	  }
}


// ������� ������ ( � ������� �������������� ������� � ������ �������� � ������� )
void vTaskCode_Count( void * pvParameters )
{
	  // ������� ��������� ��� ������ ������ � �������
	  static QueueMess set_Mess;

	  // ������� ���������� � ������� ����� ������� ���������� ����� ������ ������ � �������
	  static uint32_t previos_Time = 0;

	  // �������� �������
	  set_Mess.count = 0;

	  for( ;; )
	  {
		  // ������� ������� ��������� ���� � ������� ( ���� �� ������ 1 �� ���������� ),� ����� ��������� ���� �� ���� �������
		  // ������� uxQueueSpacesAvailable ���������� ���-�� ��������� ���� � �������
		  if( uxQueueSpacesAvailable(xQueue) && xQueue != NULL )
		  {
			  // ����������� ����� ����� ���������� � ������� �������� ������ � �������
			  set_Mess.time_log = (uint32_t)esp_timer_get_time () - previos_Time;

			  // ���������� ������ � ������� ( ��� ����� ��������� ���������� �������, ����� ��������� ��� �������� ���� ������,
			  // ���-�� ����� ��� ������� ��������� ������, � ����� � ����� ����� ������� ���������� ������
			  // ���� ������ ������� ���������� �� ������� uxQueueSpacesAvailable ������ pdTRUE
			  if( xQueueGenericSend( xQueue, ( void * ) &set_Mess, ( TickType_t ) 10, queueSEND_TO_BACK ) != pdTRUE )
			  {
				  ESP_LOGE("ERROR", "Error Queue Send");	// ���� �������� � ������� �� �������
			  }
			  else
			  {
				  // ���� ������� �� ���������� ����� ��� ������� � ��������� ��������
				  previos_Time = (uint32_t)esp_timer_get_time ();
			  }
		  }

		  // ����������� ������� �� 1
		  set_Mess.count++;

		  // ������������� ������ �� ��������� �����
		  vTaskDelay(5000 / portTICK_PERIOD_MS);
	  }
	  //vTaskDelete( NULL );
}


// ������� ������ ( � ������� �������� ������� ������� )
void vTaskCode_Log( void * pvParameters )
{
	  for( ;; )
	  {
		  // �������� ������ ��� ������ ������ � ���
		  Pointer_PrintLogCallback();

		  // ������������� ������ �� ��������� �����
		  vTaskDelay(10 / portTICK_PERIOD_MS);
	  }
	  //vTaskDelete( NULL );
}



#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"

#include <inttypes.h>
#include <stdio.h>

#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "bootloader_random.h"
#include "esp_random.h"

// struct data for queue
typedef struct
{
    uint8_t ID;
    uint32_t val;
} Data;
// this variable hold queue handle
xQueueHandle xQueue;

// variables and constants for request

#define N_REQUEST       3
#define SINGLE_REQUEST  1
uint8_t request[N_REQUEST] = {0, 0, 0};
// check if there is any request
bool flagRequest = false;

// create request task
void vRequest();
// reception task
void vReceptionTask(void);
// functional tasks
void vResponse0(void *parameter);
void vResponse1(void *parameter);
void vResponse2(void *parameter);

void app_main()
{
    /* create the queue which size can contains 5 elements of Data */
    xQueue = xQueueCreate(5, sizeof(Data));

    xTaskCreate(vRequest, "vRequest", 2048, NULL, 3, NULL);

    xTaskCreate(vReceptionTask, "vReceptionTask1", 2048, NULL, 2, NULL);

    xTaskCreate(vResponse0, "vResponse0", 2048, (uint8_t *)0u, 1, NULL);    //  id = 0
    xTaskCreate(vResponse1, "vResponse1", 2048, (uint8_t *)1u, 1, NULL);    //  id = 1
    xTaskCreate(vResponse2, "vResponse2", 2048, (uint8_t *)2u, 1, NULL);    //  id = 2
}

void vRequest()
{
    uint8_t i = 0;

    while (1)
    {
        // create 3 requests every 5 seconds

        // create request randomly from 0 to 3
        // if request == 3 then error
        bootloader_random_enable();
        request[0] = esp_random() % 3;
        request[1] = esp_random() % 3;
        request[2] = esp_random() % 3;
        bootloader_random_disable();

        // set flag to notify there is request
        flagRequest = true;

        vTaskDelay(5000 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void vReceptionTask(void)
{
    /* keep the status of sending data */
    BaseType_t xStatus;
    /* time to block the task until the queue has free space */
    const TickType_t xTicksToWait = pdMS_TO_TICKS(100);
    /* Data is added to queue*/
    Data requestedData;

    uint8_t i = 0;

    while (1)
    {
        // check if there is any request
        if (flagRequest)
        {
            // classify 3 tasks,
            // then add data to queue
            for (i = 0; i < SINGLE_REQUEST; i++)
            {
                if (request[i] == 0)
                {
                    requestedData.ID = 0;
                    requestedData.val = 0;
                    printf("Request 0: 0\n");
                }
                else if (request[i] == 1)
                {
                    requestedData.ID = 1;
                    requestedData.val = 10;
                    printf("Request 1: 10\n");
                }
                else if (request[i] == 2)
                {
                    requestedData.ID = 2;
                    requestedData.val = 20;
                    printf("Request 2: 20\n");
                }
                else
                {
                    // If no functional task receives the request,
                    // raise an error
                    requestedData.ID = request[i];
                    requestedData.val = 0;
                    printf("Request %d: Error\n", request[i]);

                    // unset flag, and ignore that request
                    flagRequest = false;
                    continue;
                }

                // send data to queue
                xStatus = xQueueSendToFront(xQueue, &requestedData, xTicksToWait);

                // check if sending is ok or not
                if (xStatus == pdPASS)
                {
                    ;
                }
                else
                {
                    printf("Request %hhn: Could not send data\n", request);
                }
            }

            // unset flag
            flagRequest = false;
        }

        vTaskDelay(50 / portTICK_RATE_MS);
    }
    
    vTaskDelete(NULL);
}

void vResponse0(void *parameter)
{
    // keep the status of receiving data
    BaseType_t xStatus;
    // time to block the task until data is available
    const TickType_t xTicksToWait = pdMS_TO_TICKS(100);

    Data data;
    uint8_t id = (uint8_t)parameter;

    while (1)
    {
        // Peek data from the queue
        // to check if the next request is for it
        xStatus = xQueuePeek(xQueue, &data, xTicksToWait);

        if (xStatus == pdPASS)
        {
            // check if request
            if (data.ID == id)
            {
                // pop data from the queue
                xStatus = xQueueReceive(xQueue, &data, xTicksToWait);

                if (xStatus == pdPASS)
                {
                    printf("Response 0: %d\n", data.val);
                }
                else
                {
                    printf("Response: Cound not recieve data\n");
                }
            }
        }

        vTaskDelay(50 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void vResponse2(void *parameter)
{
    // keep the status of receiving data
    BaseType_t xStatus;
    // time to block the task until data is available
    const TickType_t xTicksToWait = pdMS_TO_TICKS(100);

    Data data;
    uint8_t id = (uint8_t)parameter;

    while (1)
    {
        // Peek data from the queue
        // to check if the next request is for it
        xStatus = xQueuePeek(xQueue, &data, xTicksToWait);

        if (xStatus == pdPASS)
        {
            // check if request
            if (data.ID == id)
            {
                xStatus = xQueueReceive(xQueue, &data, xTicksToWait);

                // pop data from the queue
                if (xStatus == pdPASS)
                {
                    printf("Response 2: %d\n", data.val);
                }
                else
                {
                    printf("Response: Cound not recieve data\n");
                }
            }
        }

        vTaskDelay(50 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void vResponse1(void *parameter)
{
    // keep the status of receiving data
    BaseType_t xStatus;
    // time to block the task until data is available
    const TickType_t xTicksToWait = pdMS_TO_TICKS(100);

    Data data;
    uint8_t id = (uint8_t)parameter;

    while (1)
    {
        // Peek data from the queue
        // to check if the next request is for it
        xStatus = xQueuePeek(xQueue, &data, xTicksToWait);

        if (xStatus == pdPASS)
        {
            // check if request
            if (data.ID == id)
            {
                xStatus = xQueueReceive(xQueue, &data, xTicksToWait);

                // pop data from the queue
                if (xStatus == pdPASS)
                {
                    printf("Response 1: %d\n", data.val);
                }
                else
                {
                    printf("Response: Cound not recieve data\n");
                }
            }
        }

        vTaskDelay(50 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}
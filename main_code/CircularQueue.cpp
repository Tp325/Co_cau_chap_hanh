#include "CircularQueue.h"
#include <stdlib.h>
#include <string.h>
//#include <Arduino.h>

// Hàm tạo hàng đợi mới
CircularQueue *createQueue(unsigned capacity)
{
    CircularQueue *queue = (CircularQueue *)malloc(sizeof(CircularQueue));
    if (!queue)
        return NULL;

    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;
    queue->array = (char **)malloc(queue->capacity * sizeof(char *));
    for (int i = 0; i < queue->capacity; i++)
    {
        queue->array[i] = (char *)malloc(STRING_SIZE * sizeof(char));
        if (!queue->array[i])
        { 
            for (int j = 0; j < i; j++)
                free(queue->array[j]);
            free(queue->array);
            free(queue);
            return NULL;
        }
    }
    return queue;
}

// Hàm kiểm tra xem hàng đợi có đầy không
bool isFull(CircularQueue *queue)
{
    return (queue->size == queue->capacity);
}

// Hàm kiểm tra xem hàng đợi có rỗng không
bool isEmpty(CircularQueue *queue)
{
    return (queue->size == 0);
}

// Hàm thêm một phần tử vào hàng đợi
bool enqueueData(CircularQueue *queue, const char *str)
{
    if (isFull(queue))
    {
        return false;
    }
    queue->rear = (queue->rear + 1) % queue->capacity;
    strncpy(queue->array[queue->rear], str, STRING_SIZE);
    queue->array[queue->rear][STRING_SIZE - 1] = '\0'; // Đảm bảo chuỗi có kết thúc
    queue->size = queue->size + 1;
    return true;
}

// Hàm xóa một phần tử từ hàng đợi
char* dequeue(CircularQueue *queue) {
    if (isEmpty(queue)) {
        return NULL; // Trả về NULL nếu hàng đợi rỗng
    }
    char* item = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    return item; // Chú ý: item này nên được xử lý hoặc giải phóng sau khi sử dụng để tránh rò rỉ bộ nhớ
}

char* front(CircularQueue *queue) {
    if (isEmpty(queue)) {
        return NULL; // Trả về NULL nếu hàng đợi rỗng
    }
    return queue->array[queue->front];
}

// Hàm lấy (nhưng không xóa) phần tử ở cuối hàng đợi
char* rear(CircularQueue *queue) {
    if (isEmpty(queue)) {
        return NULL; // Trả về NULL nếu hàng đợi rỗng
    }
    return queue->array[queue->rear];
}

// Hàm giải phóng hàng đợi
void freeQueue(CircularQueue *queue)
{
    if (queue)
    {
        if (queue->array)
        {
            for (int i = 0; i < queue->capacity; i++)
            {
                free(queue->array[i]);
            }
            free(queue->array);
        }
        free(queue);
    }
}


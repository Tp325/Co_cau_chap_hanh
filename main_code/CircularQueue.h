// Đảm bảo rằng tệp header này chỉ được include một lần
#ifndef CIRCULARQUEUE_H
#define CIRCULARQUEUE_H

#define TIMESTAMP_SIZE 20
#define DATA_SENSOR_SIZE 100
#define STRING_SIZE 256

// Định nghĩa cấu trúc hàng đợi vòng
typedef struct {
    int front, rear, size;
    unsigned capacity;
    char** array;
} CircularQueue;

// Khai báo các hàm
CircularQueue* createQueue(unsigned capacity);
bool isFull(CircularQueue* queue);
bool isEmpty(CircularQueue* queue);
bool enqueueData(CircularQueue *queue, const char *str);
char* dequeue(CircularQueue *queue);
char* front(CircularQueue *queue);
char* rear(CircularQueue *queue);
void freeQueue(CircularQueue* queue);

#endif // CIRCULARQUEUE_H

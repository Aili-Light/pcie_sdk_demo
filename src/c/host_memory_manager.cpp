#include "host_memory_manager.h"
#include "jetson-utils/cudaMappedMemory.h"

host_memory_ring_buffer::host_memory_ring_buffer()
{
    head = 0;
    tail = 0;
}

host_memory_ring_buffer::~host_memory_ring_buffer()
{
    deinit_ring_buffer();
}


int host_memory_ring_buffer::init_ring_buffer(size_t data_size)
{
    sem_init(&sem, 0, 0);

    for(int i = 0; i < RING_BUFFER_SIZE; i++)
    {
        if( !cudaAllocMapped((void**)&(buffer[i].img_yuv2u), data_size * 2) )
        {
            printf(" failed to allocate %zu bytes for image\n", data_size);
            return 1;
        }
//        if( !cudaAllocMapped((void**)&(buffer[i].img_rgba32f), data_size * 16) )
//        {
//            printf(" failed to allocate %zu bytes for image\n", data_size);
//            return 1;
//        }
    }

    return 0;
}

int host_memory_ring_buffer::deinit_ring_buffer()
{
    sem_destroy(&sem);

    for(int i = 0; i < RING_BUFFER_SIZE; i++)
    {
        CUDA(cudaFreeHost(buffer[i].img_yuv2u));
//        CUDA(cudaFreeHost(buffer[i].img_rgba32f));
    }

    return 0;
}

int host_memory_ring_buffer::is_empty()
{
    if(this->head == this->tail)
        return 1;
    else
        return 0;
}

void host_memory_ring_buffer::move_on(int *v)
{
    *v = (*v+1) % RING_BUFFER_SIZE;
}

int host_memory_ring_buffer::push(host_memory_manager_t* t)
{
    int head;
    head = this->head;

    // printf("header %d\n", head);
    this->buffer[head] = *t;
    move_on(&(this->head));

    if(this->head == this->tail)
        move_on(&(this->tail));

    return 0;
}

host_memory_manager_t* host_memory_ring_buffer::get_head()
{
    int head;
    head = this->head;
    move_on(&(this->head));

    // printf("header %d\n", head);
    return &(this->buffer[head]);
}

host_memory_manager_t* host_memory_ring_buffer::get_tail()
{
    int tail;
    tail = this->tail;
    move_on(&(this->tail));

    // printf("header %d\n", head);
    return &(this->buffer[tail]);
}

int host_memory_ring_buffer::read_head()
{
    return this->head;
}

int host_memory_ring_buffer::read_tail()
{
    return this->tail;
}

int host_memory_ring_buffer::pop(host_memory_manager_t* t)
{
    if(!is_empty())
    {
        int tail;
        tail = this->tail;

        // printf("t %p\n", (void*)p->buffer[tail].header);
        *t = this->buffer[tail];
        move_on(&(this->tail));
        return 0;
    }
    else
    {
        return 1;
    }
}

int host_memory_ring_buffer::send_signal_frm_refresh()
{
    sem_post(&(this->sem));

    return 0;
}

int host_memory_ring_buffer::receive_signal_frm_refresh()
{
    int ret = sem_wait(&(this->sem));

    return ret;
}

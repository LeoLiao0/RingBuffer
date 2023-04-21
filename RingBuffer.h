#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
	ringbuffer_error_ok = 0,
	ringbuffer_error_parameter = 1,
	ringbuffer_error_no_memory = 2,
	ringbuffer_error_no_data = 3,
}e_ringbuffer_error_t;

typedef struct {
	
	/* Enable the priority */
	bool priority_enable;
	
	/* Setting priority level, the max is 255, and highest level is 0 */
	uint8_t priority_level;
	
	/* Write pointer index */
	uint16_t *w;
	
	/* Read pointer index */
	uint16_t *r;
	
	/* Ringbuffer data buffer pointer */
	uint8_t *pbuffer;
	
	/* Setting data size */
	uint8_t buffer_data_size;
	
	/* Setting buffer fifo size */
	uint8_t buffer_fifo_size;
	
}s_ringbuffer_t;

e_ringbuffer_error_t RingBuffer_Register ( s_ringbuffer_t *m_ringbuffer, bool priority_enable, uint8_t priority_level, uint8_t buffer_fifo_size, uint8_t buffer_data_size );
e_ringbuffer_error_t RingBuffer_UnRegister ( s_ringbuffer_t *m_ringbuffer );
e_ringbuffer_error_t RingBuffer_Push ( s_ringbuffer_t *m_ringbuffer, uint8_t priority, uint8_t *data, uint8_t len );
e_ringbuffer_error_t RingBuffer_Pop ( s_ringbuffer_t *m_ringbuffer, uint8_t *data );


#ifdef __cplusplus
}
#endif

#endif // RING_BUFFER_H

/** @} */

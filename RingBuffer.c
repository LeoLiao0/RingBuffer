#include "RingBuffer.h"

/*********************************/
/*       Internal Function       */
/*********************************/

/*
 * @ Description:
 *   - Alloc memory of ring buffer.
 * @ Input:
 *	 - ptr: The memory size want to alloc.
 * @ Output:
 *	 - The result pointer of alloc memory. If return is 0(NULL), it means alloc memory is failed.
 */
static void* ringbuffer_malloc ( size_t size )
{
	return malloc( size );
}

/*
 * @ Description:
 *   - Free the alloc memory of ring buffer.
 * @ Input:
 *	 - ptr: The pointer which is wanted to free the memory.
 */
static void ringbuffer_free ( void* ptr )
{
	if( ptr != NULL )
	{
		free( ptr );
	}
}



/*********************************/
/*       External Function       */
/*********************************/

/*
 * @ Description:
 *   - Register a ring buffer. To alloc ring buffer area in memory.
 * @ Input:
 *	 - m_ringbuffer: The pointer of ring buffer handler to stored the register ring buffer.
 *	 - priority_enable: Enable/Disable the priority using in ring buffer.
 *	 - priority_level: Configurate the ring buffer priority level. If priority do not enable, this value will be ignore. 
 * 					   The smaller the priority number, the higher the priority level.
 *	 - buffer_fifo_size: Configurate the buffer fifo size.
 *	 - buffer_data_size: Configurate the data buffer size.
 * @ Output:
 *	 - The result register ruing buffer.
 *     ringbuffer_error_parameter, The ring buffer handler or priority_level may have worring.
 *     ringbuffer_error_no_memory, Alloc memory failed.
 *     ringbuffer_error_ok, Register the ring buffer success.
 */
e_ringbuffer_error_t RingBuffer_Register ( s_ringbuffer_t *m_ringbuffer, bool priority_enable, uint8_t priority_level, uint8_t buffer_fifo_size, uint8_t buffer_data_size )
{
	m_ringbuffer->priority_enable = priority_enable;
	m_ringbuffer->priority_level = priority_level;
	m_ringbuffer->w = NULL;
	m_ringbuffer->r = NULL;
	m_ringbuffer->pbuffer = NULL;
	m_ringbuffer->buffer_data_size = buffer_data_size;
	m_ringbuffer->buffer_fifo_size = buffer_fifo_size;
		
	/* The handler pointer can not be NULL */
	if( m_ringbuffer == NULL )
	{
		return ringbuffer_error_parameter;
	}
	
	if( priority_enable == false )
	{
		/* w */
		m_ringbuffer->w = ringbuffer_malloc( sizeof(*m_ringbuffer->w) );
		
		/* Malloc write pointer fail */
		if( m_ringbuffer->w == NULL )
		{
			return ringbuffer_error_no_memory;
		}
		
		/* r */
		m_ringbuffer->r = (uint16_t*)ringbuffer_malloc( sizeof(*m_ringbuffer->r) );
		
		/* Malloc read pointer fail */
		if( m_ringbuffer->r == NULL )
		{
			ringbuffer_free( (void*)m_ringbuffer->w );
			return ringbuffer_error_no_memory;
		}
		
		/* buffer[buffer_fifo_size][buffer_data_size] */
		m_ringbuffer->pbuffer = (uint8_t*)ringbuffer_malloc( buffer_fifo_size * buffer_data_size );
		
		/* Malloc buffer fail */
		if( m_ringbuffer->pbuffer == NULL )
		{
			ringbuffer_free( (void*)m_ringbuffer->w );
			ringbuffer_free( (void*)m_ringbuffer->r );
			return ringbuffer_error_no_memory;
		}
		
		memset( (void*)m_ringbuffer->w, 0x00, sizeof(*m_ringbuffer->w) );
		memset( (void*)m_ringbuffer->r, 0x00, sizeof(*m_ringbuffer->r) );
		memset( (void*)m_ringbuffer->pbuffer, 0x00, (buffer_fifo_size * buffer_data_size) );
	}
	else
	{
		/* If priority is enable, the level must more than 0 */
		if( priority_level == 0 )
		{
			return ringbuffer_error_parameter;
		}
		
		/* w[priority_level] */
		m_ringbuffer->w = ringbuffer_malloc( sizeof(*m_ringbuffer->w) * priority_level );
		
		/* Malloc write pointer fail */
		if( m_ringbuffer->w == NULL )
		{
			return ringbuffer_error_no_memory;
		}
		
		/* r[priority_level] */
		m_ringbuffer->r = (uint16_t*)ringbuffer_malloc( sizeof(*m_ringbuffer->r) * priority_level );
		
		/* Malloc read pointer fail */
		if( m_ringbuffer->r == NULL )
		{
			ringbuffer_free( (void*)m_ringbuffer->w );
			return ringbuffer_error_no_memory;
		}
		
		/* buffer[priority_level][buffer_fifo_size][buffer_data_size] */
		m_ringbuffer->pbuffer = (uint8_t*)ringbuffer_malloc( priority_level * buffer_fifo_size * buffer_data_size );
		
		/* Malloc buffer fail */
		if( m_ringbuffer->pbuffer == NULL )
		{
			ringbuffer_free( (void*)m_ringbuffer->w );
			ringbuffer_free( (void*)m_ringbuffer->r );
			return ringbuffer_error_no_memory;
		}
		
		memset( (void*)m_ringbuffer->w, 0x00, sizeof(*m_ringbuffer->w) * priority_level );
		memset( (void*)m_ringbuffer->r, 0x00, sizeof(*m_ringbuffer->r) * priority_level );
		memset( (void*)m_ringbuffer->pbuffer, 0x00, (priority_level * buffer_fifo_size * buffer_data_size) );
	}
	return ringbuffer_error_ok;
}


/*
 * @ Description:
 *   - Un-Register a ring buffer. To free ring buffer area in memory.
 * @ Input:
 *	 - m_ringbuffer: The pointer of ring buffer handler to stored the register ring buffer.
 * @ Output:
 *	 - The result free ruing buffer.
 *     ringbuffer_error_parameter, The ring buffer handler was not correct.
 *     ringbuffer_error_ok, The specific ring buffer is free.
 */
e_ringbuffer_error_t RingBuffer_UnRegister ( s_ringbuffer_t *m_ringbuffer )
{
	/* The handler pointer can not be NULL */
	if( m_ringbuffer == NULL )
	{
		return ringbuffer_error_parameter;
	}
	
	ringbuffer_free( (void*)m_ringbuffer->w );
	ringbuffer_free( (void*)m_ringbuffer->r );
	ringbuffer_free( (void*)m_ringbuffer->pbuffer );
	
	memset( (void*)m_ringbuffer, 0x00, sizeof(s_ringbuffer_t) );
	
	return ringbuffer_error_ok;
}

e_ringbuffer_error_t RingBuffer_Push ( s_ringbuffer_t *m_ringbuffer, uint8_t priority, uint8_t *data, uint8_t len )
{
	/* The handler pointer can not be NULL */
	if( m_ringbuffer == NULL )
	{
		return ringbuffer_error_parameter;
	}
	
	/* Check the read, write index pointer and buffer pointer is exist */
	if( (m_ringbuffer->w == NULL) || (m_ringbuffer->r == NULL) || (m_ringbuffer->pbuffer == NULL) )
	{
		return ringbuffer_error_parameter;
	}
		
	/* Check whether the data length is in data buffer size. */
	if( len > m_ringbuffer->buffer_data_size )
	{
		return ringbuffer_error_parameter;
	}
	
	if( m_ringbuffer->priority_enable == false )
	{
		uint16_t temp_index = *m_ringbuffer->w + 1;
		temp_index = ( temp_index >= m_ringbuffer->buffer_fifo_size ) ? 0 : temp_index;
		
		/* Check whether there is free memory to write new data. */
		if( temp_index == *m_ringbuffer->r )
		{
			return ringbuffer_error_no_memory;
		}
		
		/* ringbuffer[fifo][data] */
		uint32_t buffer_offset = *m_ringbuffer->w * m_ringbuffer->buffer_data_size;
		memcpy( (void*)(m_ringbuffer->pbuffer + buffer_offset), (const void*)data, len );
		*m_ringbuffer->w = temp_index;
	}
	else
	{
		/* Check the priority is in range */
		if( priority >= m_ringbuffer->priority_level )
		{
			return ringbuffer_error_parameter;
		}
		
		/* w[priority] */
		/* r[priority] */
		uint8_t w_offset = priority;
		uint8_t r_offset = priority;
		uint16_t temp_index = *(m_ringbuffer->w + w_offset) + 1;
		temp_index = ( temp_index >= m_ringbuffer->buffer_fifo_size ) ? 0 : temp_index;
		
		/* Check whether there is free memory to write new data. */
		if( temp_index == *(m_ringbuffer->r + r_offset) )
		{
			return ringbuffer_error_no_memory;
		}
		
		/* ringbuffer[priority][fifo][data] */
		uint32_t buffer_offset = ( *(m_ringbuffer->w + w_offset) + priority * m_ringbuffer->buffer_fifo_size ) * m_ringbuffer->buffer_data_size;
		memcpy( (void*)(m_ringbuffer->pbuffer + buffer_offset), (const void*)data, len );
		*(m_ringbuffer->w + w_offset) = temp_index;
	}
	
	return ringbuffer_error_ok;
}

/*
 * @ Description:
 *   - Read out data from ring buffer.
 * @ Input:
 *	 - m_ringbuffer: The ring buffer pointer which is wanted to read out the data from.
 *   - data: The pointer which is stored the ring buffer read out data.
 * @ Return:
 *   - The result of reading data from ring buffer.
 *     ringbuffer_error_parameter, Read failed, the ring buffer handler or data pointer may have worring.
 *     ringbuffer_error_no_data, Read failed, the ring buffer is empty.
 *     ringbuffer_error_ok, Read success.
 */
e_ringbuffer_error_t RingBuffer_Pop ( s_ringbuffer_t *m_ringbuffer, uint8_t *data )
{
	/* The handler pointer can not be NULL */
	if( m_ringbuffer == NULL )
	{
		return ringbuffer_error_parameter;
	}
	
	/* The data pointer can not be NULL */
	if( data == NULL )
	{
		return ringbuffer_error_parameter;
	}
	
	if( m_ringbuffer->priority_enable == false )
	{
		uint16_t temp_index = ( (*m_ringbuffer->r + 1) >= m_ringbuffer->buffer_fifo_size ) ? 0 : (*m_ringbuffer->r + 1);
		
		/* Check whether there is free memory to write new data. */
		if( temp_index == *m_ringbuffer->w )
		{
			return ringbuffer_error_no_data;
		}
		
		/* ringbuffer[fifo][data] */
		uint32_t buffer_offset = *m_ringbuffer->r * m_ringbuffer->buffer_data_size;
		memcpy( (void*)data, (const void*)(m_ringbuffer->pbuffer + buffer_offset), m_ringbuffer->buffer_data_size );
		*m_ringbuffer->r = temp_index;
	}
	else
	{
		for( uint8_t priority = 0 ; priority < m_ringbuffer->priority_level ; priority++ )
		{
			if( *(m_ringbuffer->r + priority) != *(m_ringbuffer->w + priority) )
			{
				/* ringbuffer[priority][fifo][data] */
				uint32_t buffer_offset = ( *(m_ringbuffer->r + priority) + priority * m_ringbuffer->buffer_fifo_size ) * m_ringbuffer->buffer_data_size;
				memcpy( (void*)data, (const void*)(m_ringbuffer->pbuffer + buffer_offset), m_ringbuffer->buffer_data_size );
				*(m_ringbuffer->r + priority) = ( (*(m_ringbuffer->r + priority) + 1) >= m_ringbuffer->buffer_fifo_size ) ? 0 : (*(m_ringbuffer->r + priority) + 1);
				break;
			}
		}
	}
	return ringbuffer_error_ok;
}

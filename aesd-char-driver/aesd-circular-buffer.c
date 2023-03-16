/**
 * @file aesd-circular-buffer.c
 * @brief Functions and data related to a circular buffer imlementation
 *
 * @author Dan Walkes
 * @date 2020-03-01
 * @copyright Copyright (c) 2020
 *
 */

#ifdef __KERNEL__
#include <linux/string.h>
#else
#include <string.h>
#endif
 
#include "aesd-circular-buffer.h"

#define RESET 0
/**
 * @param buffer the buffer to search for corresponding offset.  Any necessary locking must be performed by caller.
 * @param char_offset the position to search for in the buffer list, describing the zero referenced
 *      character index if all buffer strings were concatenated end to end
 * @param entry_offset_byte_rtn is a pointer specifying a location to store the byte of the returned aesd_buffer_entry
 *      buffptr member corresponding to char_offset.  This value is only set when a matching char_offset is found
 *      in aesd_buffer. 
 * @return the struct aesd_buffer_entry structure representing the position described by char_offset, or
 * NULL if this position is not available in the buffer (not enough data is written).
 */
struct aesd_buffer_entry *aesd_circular_buffer_find_entry_offset_for_fpos(struct aesd_circular_buffer *buffer,
			size_t char_offset, size_t *entry_offset_byte_rtn )
{

    size_t buff_sum = RESET;
    size_t size     = RESET;
    uint8_t buff_idx;

    buff_idx = buffer->out_offs;
    
    //iterate till in_idx
    do{
        
        buff_sum += buffer->entry[buff_idx].size;
        
        size = buffer->entry[buff_idx].size;;
       
        //Check for first block
        if(char_offset <= (buff_sum-1)) {
            *entry_offset_byte_rtn = char_offset - (buff_sum - size);
            return &buffer->entry[buff_idx];
        }

        buff_idx++;

        if(buff_idx == AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED) {
            buff_idx = 0;
            }
            
    }while(buff_idx != buffer->in_offs);



    
    return NULL;
}

/**
* Adds entry @param add_entry to @param buffer in the location specified in buffer->in_offs.
* If the buffer was already full, overwrites the oldest entry and advances buffer->out_offs to the
* new start location.
* Any necessary locking must be handled by the caller
* Any memory referenced in @param add_entry must be allocated by and/or must have a lifetime managed by the caller.
*/
const char* aesd_circular_buffer_add_entry(struct aesd_circular_buffer *buffer, const struct aesd_buffer_entry *add_entry)
{
    /**
    * TODO: implement per description 
    */
    const char* entry_ptr = NULL;
     
    
     //Check if buffer is full
     if ( (buffer->in_offs == buffer->out_offs) && buffer->full ){
        entry_ptr = buffer->entry[buffer->in_offs].buffptr;
 
        buffer->entry[buffer->in_offs] = *(add_entry);
        
        buffer->in_offs++;   // Increment head
        
         //Check rollover
        if(buffer->in_offs == AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED) {
        buffer->in_offs = 0;
        }
        
        buffer->out_offs = buffer->in_offs; 
       }
      else
    {
        buffer->entry[buffer->in_offs] = *add_entry;
        
        buffer->in_offs++;

        //Check rollover
        if(buffer->in_offs == AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED)
        
        buffer->in_offs = 0;

        //Check for Buffer full
        if(buffer->in_offs == buffer->out_offs) {
            buffer->full = true;
            }
        else {
            buffer->full = false;
            }
        
    }
        return entry_ptr;
}


/**
* Initializes the circular buffer described by @param buffer to an empty struct
*/
void aesd_circular_buffer_init(struct aesd_circular_buffer *buffer)
{
    memset(buffer,0,sizeof(struct aesd_circular_buffer));
}

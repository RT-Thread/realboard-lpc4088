#ifndef _RTI_DATA_FRAME_H_
#define _RTI_DATA_FRAME_H_

#include "rti_data_object.h"
#include "rti_object_container.h"

#include <vector>
using std::vector;

/*
 * RTI Frame Format
 * Frame Magic
 * Frame Length
 */
class rti_data_frame
{
public:
	rti_data_frame(void);
	~rti_data_frame(void);

	vector<rti_object*> get_objects() { return objects; }

	rt_uint16_t parse_frame(rt_uint8_t *data_ptr);
	rt_uint16_t frame_size() { return length; }

protected:
	rt_uint16_t magic;
	rt_uint16_t length;

	vector<rti_object*> objects;
};

#endif

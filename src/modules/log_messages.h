#pragma once







#define LOG_DEF(_name, _rate, _format, _labels) { \
	.format = { \
		.length = sizeof(struct log_##_name##_s), \
		.name = #_name, \
		.format = _format, \
		.labels = _labels \
	}, \
	.rate = _rate, \
	.pack =  log_##_name##_write \
}

#define LOG_FORMAT_MSG	  0x80










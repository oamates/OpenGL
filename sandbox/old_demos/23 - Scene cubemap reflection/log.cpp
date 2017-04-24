#include "log.hpp"

void log_write(const char *format, ...)
{
	va_list ap;
	FILE *output = fopen("debug.log", "a+");
	if (!output) return;
	va_start(ap, format);
	vfprintf(output, format, ap);
	va_end(ap);
	fclose(output);
}

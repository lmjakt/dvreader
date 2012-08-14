#ifndef CLERROR_H
#define CLERROR_H

#include <CL/cl.h>

const char* error_string(cl_int err);
void report_error(cl_int err);
void report_error_pf(const char* pf, cl_int err);

#endif

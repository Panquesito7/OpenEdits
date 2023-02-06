#include "unittest_internal.h"

void unittest_packet();
void unittest_connection();
void unittest_world();

void unittest()
{
	puts("==> Start unittest");
	unittest_world();
	unittest_packet();
	unittest_connection();
	puts("<== Unittest completed");
}


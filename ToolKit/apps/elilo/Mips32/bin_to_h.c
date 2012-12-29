#include <stdio.h>
#include <stdlib.h>

int
main(void)
{
	unsigned n = 0;
	int c;

	printf("UINT8 rmswitch_image[] = {\n");

	while ((c = getchar()) != EOF) {
		printf("0x%02x,%s",
			c & 0xFF,
			(++n & 0x07) ? " " : "\n");
	}

	if (n & 0x07) {
		printf("\n");
	}

	printf(
		"};\n"
		"UINTN rmswitch_size = sizeof rmswitch_image;\n");

	return 0;
}

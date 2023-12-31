#include <stdio.h>
#include <math.h>

int main()
{
	printf( "#ifndef _SINTABLE_H\n#define _SINTABLE_H\n#include <stdint.h>\n" );
	printf( "char sintable127[512] = { " );
	int i;
	for( i = 0; i < 512; i++ )
	{
		printf( "%d, ", (int)(sin(i*3.14159/256) * 62 + 64) );
	}
	printf( "};\n#endif\n" );
}
